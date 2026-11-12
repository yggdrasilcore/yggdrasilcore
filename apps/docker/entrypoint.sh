#!/usr/bin/env bash
set -euo pipefail

CONF_DIR="${CONF_DIR:-/yggdrasilcore/env/dist/etc}"
LOGS_DIR="${LOGS_DIR:-/yggdrasilcore/env/dist/logs}"

# Let the server locate the crash report directory (see CrashHandler.cpp).
export LOGS_DIR

if ! touch "$CONF_DIR/.write-test" || ! touch "$LOGS_DIR/.write-test"; then
    cat <<EOF
===== WARNING =====
The current user doesn't have write permissions for
the configuration dir ($CONF_DIR) or logs dir ($LOGS_DIR).
It's likely that services will fail due to this.

This is usually caused by cloning the repository as root,
so the files are owned by root (uid 0).

To resolve this, you can set the ownership of the
configuration directory with the command on the host machine.
Note that if the files are owned as root, the ownership must
be changed as root (hence sudo).

$ sudo chown -R $(id -u):$(id -g) /path/to$CONF_DIR /path/to$LOGS_DIR

Alternatively, you can set the DOCKER_USER environment
variable (on the host machine) to "root", though this
isn't recommended.

$ DOCKER_USER=root docker-compose up -d
====================
EOF
fi

[[ -f "$CONF_DIR/.write-test" ]] && rm -f "$CONF_DIR/.write-test"
[[ -f "$LOGS_DIR/.write-test" ]] && rm -f "$LOGS_DIR/.write-test"

# Copy all default config files to env/dist/etc if they don't already exist
# -r == recursive
# -n == no clobber (don't overwrite)
# -v == be verbose
# Not every image ships reference configs (the map tools do not), so an empty
# directory must not make the glob fail the script.
shopt -s nullglob
ref_configs=(/yggdrasilcore/env/ref/etc/*)
if [ ${#ref_configs[@]} -gt 0 ]; then
    cp -rnv "${ref_configs[@]}" "$CONF_DIR"
fi
shopt -u nullglob

CONF="$CONF_DIR/$YCORE_COMPONENT.conf"
CONF_DIST="$CONF_DIR/$YCORE_COMPONENT.conf.dist"

# Copy the "dist" file to the "conf" if the conf doesn't already exist
if [[ -f "$CONF_DIST" ]]; then
    cp -vn "$CONF_DIST" "$CONF"
else
    touch "$CONF"
fi

echo "Starting $YCORE_COMPONENT..."

exec "$@"
