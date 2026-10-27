#!/usr/bin/env bash
# Consolidated SQL validation for YggdrasilCore CI.
#
# Performs both halves of the SQL operation in a single script:
#   1. Check changed SQL update files against the data-change rules.
#   2. Import the base schema and every update into a fresh MySQL instance,
#      mirroring what dbimport does at boot.
#
# Usage: sql-validate.sh <changed-sql-files...>
set -euo pipefail

MYSQL_HOST="${MYSQL_HOST:-127.0.0.1}"
MYSQL_PORT="${MYSQL_PORT:-3306}"
MYSQL_USER="${MYSQL_USER:-root}"
MYSQL_PASSWORD="${MYSQL_PASSWORD:-password}"
export MYSQL_PWD="$MYSQL_PASSWORD"

MYSQL=(
    mysql --protocol=TCP --host="$MYSQL_HOST" --port="$MYSQL_PORT" --user="$MYSQL_USER"
    --default-character-set=utf8mb4 --max-allowed-packet=1G
)

ROOT_DIR="${GITHUB_WORKSPACE:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)}"

# ============================================================================
# Step 1: SQL update rule checks
# ============================================================================
python3 - "$@" <<'PYEOF'
import argparse
import re
import sys
from pathlib import Path

DELETE_RE = re.compile(
    r"^DELETE\s+(?:(?:LOW_PRIORITY|QUICK|IGNORE)\s+)*FROM\s+"
    r"(?:(?:`[^`]+`|[A-Za-z0-9_]+)\.)?(`[^`]+`|[A-Za-z0-9_]+)",
    re.IGNORECASE,
)
INSERT_RE = re.compile(
    r"^INSERT\s+(IGNORE\s+)?(?:INTO\s+)?"
    r"(?:(?:`[^`]+`|[A-Za-z0-9_]+)\.)?(`[^`]+`|[A-Za-z0-9_]+)",
    re.IGNORECASE,
)
REPLACE_RE = re.compile(
    r"^REPLACE\s+(?:(?:LOW_PRIORITY|DELAYED)\s+)*(?:INTO\s+)?"
    r"(?:(?:`[^`]+`|[A-Za-z0-9_]+)\.)?(`[^`]+`|[A-Za-z0-9_]+)",
    re.IGNORECASE,
)
UPDATE_RE = re.compile(
    r"^UPDATE\s+(?:(?:LOW_PRIORITY|IGNORE)\s+)*"
    r"(?:(?:`[^`]+`|[A-Za-z0-9_]+)\.)?(`[^`]+`|[A-Za-z0-9_]+)",
    re.IGNORECASE,
)
ON_DUPLICATE_RE = re.compile(r"\bON\s+DUPLICATE\s+KEY\s+UPDATE\b", re.IGNORECASE)


def normalize_table(name: str) -> str:
    parts = name.replace("`", "").split(".")
    return parts[-1].lower()


def iter_statements(text: str):
    """Yield (line_number, statement) for every semicolon-terminated statement."""
    statements = []
    buffer = []
    start_line = 1
    line = 1
    index = 0
    length = len(text)

    def first_char_seen() -> None:
        nonlocal start_line
        if not buffer:
            start_line = line

    while index < length:
        char = text[index]

        if char == "\n":
            line += 1
            if buffer:
                buffer.append(char)
            index += 1
            continue

        if char == "-" and index + 1 < length and text[index + 1] == "-":
            while index < length and text[index] != "\n":
                index += 1
            continue

        if char == "#":
            while index < length and text[index] != "\n":
                index += 1
            continue

        if char == "/" and index + 1 < length and text[index + 1] == "*":
            index += 2
            while index + 1 < length and not (text[index] == "*" and text[index + 1] == "/"):
                if text[index] == "\n":
                    line += 1
                index += 1
            index += 2
            continue

        if char in ("'", '"'):
            quote = char
            first_char_seen()
            buffer.append(char)
            index += 1
            while index < length:
                current = text[index]
                if current == "\\" and index + 1 < length:
                    buffer.append(text[index:index + 2])
                    index += 2
                    continue
                if current == quote:
                    if index + 1 < length and text[index + 1] == quote:
                        buffer.append(quote * 2)
                        index += 2
                        continue
                    buffer.append(quote)
                    index += 1
                    break
                if current == "\n":
                    line += 1
                buffer.append(current)
                index += 1
            continue

        if char == "`":
            first_char_seen()
            buffer.append(char)
            index += 1
            while index < length:
                current = text[index]
                if current == "`":
                    if index + 1 < length and text[index + 1] == "`":
                        buffer.append("``")
                        index += 2
                        continue
                    buffer.append("`")
                    index += 1
                    break
                if current == "\n":
                    line += 1
                buffer.append(current)
                index += 1
            continue

        if char == ";":
            statement = "".join(buffer).strip()
            if statement:
                statements.append((start_line, statement))
            buffer = []
            index += 1
            continue

        if not char.isspace():
            first_char_seen()
        if buffer or not char.isspace():
            buffer.append(char)
        index += 1

    statement = "".join(buffer).strip()
    if statement:
        statements.append((start_line, statement))
    return statements


def classify(statement: str):
    for kind, pattern in (
        ("DELETE", DELETE_RE),
        ("INSERT", INSERT_RE),
        ("REPLACE", REPLACE_RE),
        ("UPDATE", UPDATE_RE),
    ):
        match = pattern.match(statement)
        if match:
            table = normalize_table(match.group(match.lastindex))
            return kind, table, match
    return None, None, None


def check_file(path: Path):
    errors = []
    warnings = []
    text = path.read_text(encoding="utf-8")
    statements = iter_statements(text)

    parsed = []
    for line, statement in statements:
        kind, table, match = classify(statement)
        if kind:
            parsed.append((line, statement, kind, table, match))

    for offset, (line, statement, kind, table, match) in enumerate(parsed):
        if kind == "DELETE":
            next_kind = None
            for _, _, later_kind, later_table, _ in parsed[offset + 1:]:
                if later_table != table:
                    continue
                if later_kind == "DELETE":
                    continue
                next_kind = later_kind
                break

            if next_kind == "INSERT":
                continue
            if next_kind == "REPLACE":
                warnings.append(
                    f"{path}:{line}: DELETE from `{table}` is followed by REPLACE; "
                    "prefer DELETE followed by INSERT to recreate the rows"
                )
                continue
            if next_kind == "UPDATE":
                errors.append(
                    f"{path}:{line}: DELETE from `{table}` is followed by UPDATE; "
                    "use UPDATE/REPLACE to update existing rows or DELETE followed by INSERT to recreate them"
                )
                continue
            errors.append(
                f"{path}:{line}: DELETE from `{table}` is not followed by an INSERT; "
                "use DELETE followed by INSERT to recreate rows"
            )

        elif kind == "INSERT" and ON_DUPLICATE_RE.search(statement):
            if not match.group(1):
                errors.append(
                    f"{path}:{line}: INSERT ... ON DUPLICATE KEY UPDATE on `{table}` "
                    "must use INSERT IGNORE"
                )

    return errors, warnings


def main() -> int:
    files = [Path(name) for name in sys.argv[1:] if Path(name).is_file()]
    if not files:
        print("No SQL files to check.")
        return 0

    all_errors = []
    all_warnings = []
    for path in files:
        errors, warnings = check_file(path)
        all_errors.extend(errors)
        all_warnings.extend(warnings)

    for message in all_warnings:
        print(f"::warning::{message}")
    for message in all_errors:
        print(f"::error::{message}")

    print(f"Checked {len(files)} SQL file(s): {len(all_errors)} error(s), {len(all_warnings)} warning(s).")
    return 1 if all_errors else 0


sys.exit(main())
PYEOF

# ============================================================================
# Step 2: Import base schema and updates into a fresh MySQL instance.
# Mirrors what dbimport does at boot: load data/sql/base/db_<name>, then
# apply every file in data/sql/updates/db_<name> in filename order.
# ============================================================================
databases=(auth world characters)

for database in "${databases[@]}"; do
    echo "Creating database ycore_${database}"
    "${MYSQL[@]}" -e "CREATE DATABASE IF NOT EXISTS \`ycore_${database}\` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;"
done

for database in "${databases[@]}"; do
    echo "Importing base db_${database}"
    mapfile -t base_files < <(find "$ROOT_DIR/data/sql/base/db_${database}" -maxdepth 1 -type f -name '*.sql' | sort)
    for file in "${base_files[@]}"; do
        echo "  base: $(basename "$file")"
        "${MYSQL[@]}" "ycore_${database}" < "$file"
    done
done

for database in "${databases[@]}"; do
    echo "Applying updates db_${database}"
    mapfile -t update_files < <(find "$ROOT_DIR/data/sql/updates/db_${database}" -maxdepth 1 -type f -name '*.sql' | sort)
    for file in "${update_files[@]}"; do
        echo "  update: $(basename "$file")"
        "${MYSQL[@]}" "ycore_${database}" < "$file"
    done
done

echo "All base files and updates imported successfully."
