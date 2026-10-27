#!/usr/bin/env bash
# Import the base schema and every SQL update into a fresh MySQL instance.
#
# This mirrors what dbimport does at boot: load data/sql/base/db_<name>, then
# apply every file in data/sql/updates/db_<name> in filename order. Any error
# stops the import and is reported with the file that caused it.

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SQL_DIR="${SQL_DIR:-$ROOT_DIR/data/sql}"

MYSQL_HOST="${MYSQL_HOST:-127.0.0.1}"
MYSQL_PORT="${MYSQL_PORT:-3306}"
MYSQL_USER="${MYSQL_USER:-root}"
MYSQL_PASSWORD="${MYSQL_PASSWORD:-password}"
MYSQL_BIN="${MYSQL_BIN:-mysql}"

export MYSQL_PWD="$MYSQL_PASSWORD"

MYSQL=(
    "$MYSQL_BIN"
    --protocol=TCP
    --host="$MYSQL_HOST"
    --port="$MYSQL_PORT"
    --user="$MYSQL_USER"
    --default-character-set=utf8mb4
    --max-allowed-packet=1G
)

databases=(auth world characters)

for database in "${databases[@]}"; do
    echo "Creating database ycore_${database}"
    "${MYSQL[@]}" -e "CREATE DATABASE IF NOT EXISTS \`ycore_${database}\` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;"
done

for database in "${databases[@]}"; do
    echo "Importing base db_${database}"
    mapfile -t files < <(find "$SQL_DIR/base/db_${database}" -maxdepth 1 -type f -name '*.sql' | sort)
    for file in "${files[@]}"; do
        echo "  base: $(basename "$file")"
        "${MYSQL[@]}" "ycore_${database}" < "$file"
    done
done

for database in "${databases[@]}"; do
    echo "Applying updates db_${database}"
    mapfile -t files < <(find "$SQL_DIR/updates/db_${database}" -maxdepth 1 -type f -name '*.sql' | sort)
    for file in "${files[@]}"; do
        echo "  update: $(basename "$file")"
        "${MYSQL[@]}" "ycore_${database}" < "$file"
    done
done

echo "All base files and updates imported successfully."
