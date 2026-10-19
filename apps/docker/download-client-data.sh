#!/usr/bin/env bash

# Downloads the world client data (maps, vmaps, mmaps, etc.) used by the
# server. Invoked by the `client-data` Docker target.

# change the following version when needed
VERSION=v1

echo "#######################"
echo "Client data downloader"
echo "#######################"

# first check if it's defined in env, otherwise use the default
path="${DATAPATH:-/yggdrasilcore/env/dist/data}"
zipPath="${DATAPATH_ZIP:-"$path/data.zip"}"

dataVersionFile="$path/data-version"

[ -f "$dataVersionFile" ] && source "$dataVersionFile"

# create the path if doesn't exists
mkdir -p "$path"

if [ "$VERSION" == "$INSTALLED_VERSION" ]; then
    echo "Data $VERSION already installed. If you want to force the download remove the following file: $dataVersionFile"
    exit 0
fi

echo "Downloading client data in: $zipPath ..."
curl -L https://github.com/yggdrasilcore/client-data/releases/download/$VERSION/Data.zip > "$zipPath" \
    && echo "unzip downloaded file in $path..." && unzip -q -o "$zipPath" -d "$path/" \
    && echo "Remove downloaded file" && rm "$zipPath" \
    && echo "INSTALLED_VERSION=$VERSION" > "$dataVersionFile"
