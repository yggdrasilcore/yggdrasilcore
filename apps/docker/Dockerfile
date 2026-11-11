ARG UBUNTU_VERSION=22.04 # lts

# Build type.
ARG CTYPE="RelWithDebInfo"
ARG CWITH_WARNINGS="ON"
ARG CMAKE_EXTRA_OPTIONS=""

# This target lays out the general directory skeleton for YggdrasilCore,
# This target isn't intended to be directly used
FROM ubuntu:$UBUNTU_VERSION AS skeleton

# Note: ARG instructions defined after FROM are available in this build stage.
# Placing ARG TZ here (after FROM) ensures it is accessible for configuring the timezone below.
ARG TZ=Etc/UTC
ARG DOCKER=1
ARG DEBIAN_FRONTEND=noninteractive

ENV YC_FORCE_CREATE_DB=1

RUN mkdir -pv \
        /yggdrasilcore/bin                   \
        /yggdrasilcore/data                  \
        /yggdrasilcore/deps                  \
        /yggdrasilcore/env/dist/bin          \
        /yggdrasilcore/env/dist/data/Cameras \
        /yggdrasilcore/env/dist/data/dbc     \
        /yggdrasilcore/env/dist/data/maps    \
        /yggdrasilcore/env/dist/data/mmaps   \
        /yggdrasilcore/env/dist/data/vmaps   \
        /yggdrasilcore/env/dist/logs         \
        /yggdrasilcore/env/dist/temp         \
        /yggdrasilcore/env/dist/etc          \
        /yggdrasilcore/modules               \
        /yggdrasilcore/src                   \
        /yggdrasilcore/build

# Configure Timezone
RUN apt-get update                                                       \
    && apt-get install -y --no-install-recommends tzdata ca-certificates \
    && ln -snf "/usr/share/zoneinfo/$TZ" /etc/localtime                  \
    && echo "$TZ" > /etc/timezone                                          \
    && dpkg-reconfigure --frontend noninteractive tzdata                 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /yggdrasilcore

###################################
# Compiler toolchain              #
###################################
# boost: only the five components the core links against (filesystem,
# program_options, iostreams, regex, thread). libboost-all-dev additionally
# pulls in mpi, python, serialization and wave, none of which are referenced.
#
# No libmysql++-dev: the core talks to MySQL through libmysqlclient directly.
# No mysql client CLI, openssl CLI, lsb-base, curl, unzip or libtool: none of
# them are needed to build the core.
FROM skeleton AS build-toolchain

# The development packages of every build variant are installed here, in front
# of the source copy. A layer behind the source copy is invalidated by every
# source change, so installing the packages per variant would run
# "apt-get update" and "apt-get install" again on every rebuild.
#
#   - readline and ncurses: the worldserver links them directly (see the
#     LINK_FLAGS in src/server/apps/CMakeLists.txt)
#   - zlib and bzip2: the map libraries pull them in
#   - lld: links the big binaries considerably faster than GNU ld
RUN apt-get update                                                     \
    && apt-get install -y --no-install-recommends                         \
        build-essential ccache cmake ninja-build clang lld git            \
        default-libmysqlclient-dev libssl-dev libdw-dev                   \
        libboost-dev                                                      \
        libboost-filesystem-dev libboost-program-options-dev              \
        libboost-iostreams-dev libboost-regex-dev libboost-thread-dev      \
        libreadline-dev libncurses5-dev zlib1g-dev libbz2-dev             \
    && dpkg -s libmysqlclient-dev | grep '^Version' | cut -d' ' -f2 > /mysql-client-version \
    && rm -rf /var/lib/apt/lists/*

# The compiler cache.
#
# CCACHE_DIR has to point at the cache mount, otherwise ccache stores the
# objects in /root/.cache/ccache, outside of the mount, and both the mount and
# the ccache-export stage stay empty.
#
# The other settings are what make the cache usable in a container: the
# checkout gives every source file a new mtime on every build, so without
# include_file_mtime every entry would be invalidated, and the precompiled
# headers are regenerated with every build, so without pch_defines every
# translation unit that uses them would be invalidated.
ARG CCACHE_DIR="/ccache"
ARG CCACHE_MAXSIZE="1000MB"
ARG CCACHE_COMPILERCHECK="content"
ARG CCACHE_SLOPPINESS="pch_defines,time_macros,include_file_mtime"
ARG CCACHE_COMPRESS="1"

ENV CCACHE_DIR="${CCACHE_DIR}"                     \
    CCACHE_MAXSIZE="${CCACHE_MAXSIZE}"             \
    CCACHE_COMPILERCHECK="${CCACHE_COMPILERCHECK}" \
    CCACHE_SLOPPINESS="${CCACHE_SLOPPINESS}"       \
    CCACHE_COMPRESS="${CCACHE_COMPRESS}"

###################################
# Source tree                     #
###################################
FROM build-toolchain AS build-source

COPY CMakeLists.txt /yggdrasilcore/CMakeLists.txt
COPY conf /yggdrasilcore/conf
COPY deps /yggdrasilcore/deps
COPY src /yggdrasilcore/src
COPY modules /yggdrasilcore/modules

WORKDIR /yggdrasilcore/build

###################################
# Revision + compiler cache seed  #
###################################
# BuildKit keys a RUN on the contents of the contexts it binds, and the checkout
# rewrites `.git` on every run. Stage the checkout metadata into a cache mount
# first (this writes nothing to the image), then compile with only cache mounts
# attached. Cache mounts are excluded from the cache key and persist between
# builds, so an unchanged revision reuses the compile layer instead of rebuilding.
#
# Both cache mounts are shared by the build variants below, so a source file is
# only ever compiled once, no matter which image is being built.
FROM build-source AS build-seed

ARG CACHEBUST=1

RUN --mount=type=bind,source=.git,target=/git-seed \
    --mount=type=bind,source=var/docker/ccache,target=/ccache-seed \
    --mount=type=cache,target=/yggdrasilcore/.git,id=yggdrasilcore-git,sharing=locked \
    --mount=type=cache,target=/ccache,id=yggdrasilcore-ccache,sharing=shared \
    echo "CACHEBUST=$CACHEBUST"; \
    find /yggdrasilcore/.git -mindepth 1 -delete 2>/dev/null || true; \
    cp -a /git-seed/. /yggdrasilcore/.git/; \
    cp -an /ccache-seed/. /ccache/ 2>/dev/null || true

###################################
# Build variants                  #
###################################
# Every image compiles only the code it ships:
#
#   authserver  -> the shared/common/database libraries and the authserver
#                  executable. It uses neither the game, the script nor the
#                  module code.
#   worldserver -> the game/script/module code and the worldserver executable,
#                  plus dbimport, which links those very same libraries.
#   tools       -> only the map extraction tools.
#
# The scopes are fixed per variant: dbimport links the game, script and module
# libraries, so it cannot be built without them, and one build argument
# changing the scope of a variant would break the others.

# -- Authserver ------------------------------------------------------------
FROM build-seed AS build-auth

ARG CTYPE
ARG CWITH_WARNINGS
ARG CMAKE_EXTRA_OPTIONS

RUN --mount=type=cache,target=/ccache,id=yggdrasilcore-ccache,sharing=shared \
    --mount=type=cache,target=/yggdrasilcore/.git,id=yggdrasilcore-git,sharing=shared \
    git config --global --add safe.directory /yggdrasilcore \
    && cmake /yggdrasilcore \
       -G Ninja \
       -DCMAKE_INSTALL_PREFIX="/yggdrasilcore/env/dist"  \
       -DAPPS_BUILD="auth-only"                          \
       -DTOOLS_BUILD="none"                              \
       -DSCRIPTS="none"                                  \
       -DMODULES="none"                                  \
       -DWITH_WARNINGS="$CWITH_WARNINGS"                 \
       -DCMAKE_BUILD_TYPE="$CTYPE"                       \
       -DCMAKE_CXX_COMPILER="clang++"                    \
       -DCMAKE_C_COMPILER="clang"                        \
       -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld"           \
       -DCMAKE_CXX_COMPILER_LAUNCHER="ccache"            \
       -DCMAKE_C_COMPILER_LAUNCHER="ccache"              \
       -DBoost_USE_STATIC_LIBS="ON"                      \
       -DLUA_VERSION="luajit"                            \
       -DCMAKE_C_FLAGS="-gdwarf-aranges"                 \
       -DCMAKE_CXX_FLAGS="-gdwarf-aranges"               \
       $CMAKE_EXTRA_OPTIONS                                \
    && cmake --build . --config "$CTYPE" -j $(($(nproc) + 1)) \
    && cmake --install . --config "$CTYPE"

# -- Worldserver + dbimport -----------------------------------------------
FROM build-seed AS build-world

ARG CTYPE
ARG CWITH_WARNINGS
ARG CMAKE_EXTRA_OPTIONS

RUN --mount=type=cache,target=/ccache,id=yggdrasilcore-ccache,sharing=shared \
    --mount=type=cache,target=/yggdrasilcore/.git,id=yggdrasilcore-git,sharing=shared \
    git config --global --add safe.directory /yggdrasilcore \
    && cmake /yggdrasilcore \
       -G Ninja \
       -DCMAKE_INSTALL_PREFIX="/yggdrasilcore/env/dist"  \
       -DAPPS_BUILD="world-only"                         \
       -DTOOLS_BUILD="db-only"                           \
       -DSCRIPTS="static"                                \
       -DMODULES="static"                                \
       -DWITH_WARNINGS="$CWITH_WARNINGS"                 \
       -DCMAKE_BUILD_TYPE="$CTYPE"                       \
       -DCMAKE_CXX_COMPILER="clang++"                    \
       -DCMAKE_C_COMPILER="clang"                        \
       -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld"           \
       -DCMAKE_CXX_COMPILER_LAUNCHER="ccache"            \
       -DCMAKE_C_COMPILER_LAUNCHER="ccache"              \
       -DBoost_USE_STATIC_LIBS="ON"                      \
       -DLUA_VERSION="luajit"                            \
       -DCMAKE_C_FLAGS="-gdwarf-aranges"                 \
       -DCMAKE_CXX_FLAGS="-gdwarf-aranges"               \
       $CMAKE_EXTRA_OPTIONS                                \
    && cmake --build . --config "$CTYPE" -j $(($(nproc) + 1)) \
    && cmake --install . --config "$CTYPE"

# -- Map extraction tools -------------------------------------------------
FROM build-seed AS build-tools

ARG CTYPE
ARG CWITH_WARNINGS
ARG CMAKE_EXTRA_OPTIONS

RUN --mount=type=cache,target=/ccache,id=yggdrasilcore-ccache,sharing=shared \
    --mount=type=cache,target=/yggdrasilcore/.git,id=yggdrasilcore-git,sharing=shared \
    git config --global --add safe.directory /yggdrasilcore \
    && cmake /yggdrasilcore \
       -G Ninja                                          \
       -DCMAKE_INSTALL_PREFIX="/yggdrasilcore/env/dist"  \
       -DAPPS_BUILD="none"                               \
       -DTOOLS_BUILD="maps-only"                         \
       -DSCRIPTS="none"                                  \
       -DMODULES="none"                                  \
       -DWITH_WARNINGS="$CWITH_WARNINGS"                 \
       -DCMAKE_BUILD_TYPE="$CTYPE"                       \
       -DCMAKE_CXX_COMPILER="clang++"                    \
       -DCMAKE_C_COMPILER="clang"                        \
       -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld"           \
       -DCMAKE_CXX_COMPILER_LAUNCHER="ccache"            \
       -DCMAKE_C_COMPILER_LAUNCHER="ccache"              \
       -DBoost_USE_STATIC_LIBS="ON"                      \
       -DLUA_VERSION="luajit"                            \
       -DCMAKE_C_FLAGS="-gdwarf-aranges"                 \
       -DCMAKE_CXX_FLAGS="-gdwarf-aranges"               \
       $CMAKE_EXTRA_OPTIONS                               \
    && cmake --build . --config "$CTYPE" -j $(($(nproc) + 1)) \
    && cmake --install . --config "$CTYPE"

###################################
# Compiler cache export (CI only) #
###################################
# Republishes the ccache cache mount into the stage filesystem so the workflow
# can extract it with `--output type=local` and cache it between jobs. Never
# part of a normal image build.
FROM alpine:3.20 AS ccache-export

ARG CACHEBUST=1

RUN --mount=type=bind,source=var/docker/ccache,target=/ccache-seed \
    --mount=type=cache,target=/ccache,id=yggdrasilcore-ccache,sharing=locked \
    cp -an /ccache-seed/. /ccache/ 2>/dev/null || true; \
    echo "$CACHEBUST"; \
    rm -rf /out && mkdir -p /out && cp -a /ccache/. /out/

#############################
# Base runtime for services #
#############################
# Only the C++ runtime is shared by every service binary. The rest is added by
# the image that needs it:
#   - libssl3  : the core links OpenSSL.
#   - libdw1   : the DWARF reader of the crash handler and the deadlock
#                detector. It is linked into the common library, so every
#                binary that links it needs it at runtime.
#   - libmysqlclient21 : the database layer.
FROM skeleton AS runtime

ARG USER_ID=1000
ARG GROUP_ID=1000
ARG DOCKER_USER=ycore

ENV YCORE_COMPONENT=undefined

# The version of the MySQL client library the binaries were linked against, so
# the runtime library matches the one from the build stage.
COPY --from=build-toolchain /mysql-client-version /mysql-client-version

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      libstdc++6 libgcc-s1 && \
    rm -rf /var/lib/apt/lists/*

ENV PATH="/yggdrasilcore/env/dist/bin:$PATH"

RUN groupadd --gid "$GROUP_ID"  "$DOCKER_USER" && \
    useradd -d /yggdrasilcore --uid "$USER_ID"  --gid "$GROUP_ID"  "$DOCKER_USER" && \
    passwd -d "$DOCKER_USER"

COPY --chown=$DOCKER_USER:$DOCKER_USER \
     --chmod=755 \
     apps/docker/entrypoint.sh /yggdrasilcore/entrypoint.sh

RUN mkdir -p /yggdrasilcore/env/dist/logs && \
    chown -R "$DOCKER_USER:$DOCKER_USER" /yggdrasilcore && \
    chmod -R 755 /yggdrasilcore/env/dist/logs

VOLUME /yggdrasilcore/env/dist/etc

ENTRYPOINT ["/usr/bin/env", "bash", "/yggdrasilcore/entrypoint.sh"]

###############
# Auth Server #
###############
# Links the shared/common/database libraries, so it needs the MySQL client
# library on top of the shared runtime.
FROM runtime AS authserver
LABEL description="YggdrasilCore Auth Server"

ENV YCORE_COMPONENT=authserver
# Don't run database migrations. We can leave that up to the db-import container
ENV YC_UPDATES_ENABLE_DATABASES=0
# This disables user prompts. The console is still active, however
ENV YC_DISABLE_INTERACTIVE=1
ENV YC_CLOSE_IDLE_CONNECTIONS=0

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      libssl3 libdw1 \
      libmysqlclient21=$(cat /mysql-client-version) && \
    rm -rf /var/lib/apt/lists/*

COPY --chown=$DOCKER_USER:$DOCKER_USER \
     --from=build-auth \
     /yggdrasilcore/env/dist/bin/authserver /yggdrasilcore/env/dist/bin/authserver

# The crash handler prints the source lines around the crash address. It reads
# them from the paths recorded in the DWARF information, which are the paths
# the sources were compiled under, so the sources have to be present at the
# very same location at runtime. Only the trees the authserver is built from
# are needed.
COPY --chown=$DOCKER_USER:$DOCKER_USER \
     --from=build-auth \
     /yggdrasilcore/src/common /yggdrasilcore/src/common

COPY --chown=$DOCKER_USER:$DOCKER_USER \
     --from=build-auth \
     /yggdrasilcore/src/server/database /yggdrasilcore/src/server/database

COPY --chown=$DOCKER_USER:$DOCKER_USER \
     --from=build-auth \
     /yggdrasilcore/src/server/shared /yggdrasilcore/src/server/shared

COPY --chown=$DOCKER_USER:$DOCKER_USER \
     --from=build-auth \
     /yggdrasilcore/src/server/apps/authserver /yggdrasilcore/src/server/apps/authserver

# The reference configuration of the component this image runs.
COPY --chown=$DOCKER_USER:$DOCKER_USER \
     --from=build-auth \
     /yggdrasilcore/env/dist/etc/ /yggdrasilcore/env/ref/etc/

USER $DOCKER_USER

CMD ["authserver"]

################
# World Server #
################
# The worldserver links readline and ncurses for its console, and the map
# libraries it pulls in need zlib.
FROM runtime AS worldserver
LABEL description="YggdrasilCore World Server"

ENV YCORE_COMPONENT=worldserver
# Don't run database migrations. We can leave that up to the db-import container
ENV YC_UPDATES_ENABLE_DATABASES=0
# This disables user prompts. The console is still active, however
ENV YC_DISABLE_INTERACTIVE=1
ENV YC_CLOSE_IDLE_CONNECTIONS=0

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      libssl3 libdw1 zlib1g \
      libmysqlclient21=$(cat /mysql-client-version) \
      libreadline8 libncurses6 && \
    rm -rf /var/lib/apt/lists/*

COPY --chown=$DOCKER_USER:$DOCKER_USER \
     --from=build-world \
     /yggdrasilcore/env/dist/bin/worldserver /yggdrasilcore/env/dist/bin/worldserver

# The crash handler prints the source lines around the crash address. It reads
# them from the paths recorded in the DWARF information, which are the paths
# the sources were compiled under, so the sources have to be present at the
# very same location at runtime.
COPY --chown=$DOCKER_USER:$DOCKER_USER \
     --from=build-world \
     /yggdrasilcore/src /yggdrasilcore/src

COPY --chown=$DOCKER_USER:$DOCKER_USER \
     --from=build-world \
     /yggdrasilcore/modules /yggdrasilcore/modules

# The reference configuration of the components this image runs.
COPY --chown=$DOCKER_USER:$DOCKER_USER \
     --from=build-world \
     /yggdrasilcore/env/dist/etc/ /yggdrasilcore/env/ref/etc/

USER $DOCKER_USER

CMD ["worldserver"]

#############
# DB Import #
#############
# Shares the game/script/module libraries with the worldserver, so it is built
# from the same variant and needs the same libraries. It does not install the
# crash handler, so it needs none of the crash report sources.
FROM runtime AS db-import
LABEL description="YggdrasilCore Database Import tool"

ENV YCORE_COMPONENT=dbimport

# The database updater applies the SQL files with the mysql client, it spawns
# the executable (see the StartProcess call in DBUpdater.h), so the client has
# to be in the image.
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      libssl3 libdw1 zlib1g \
      libmysqlclient21=$(cat /mysql-client-version) \
      default-mysql-client && \
    rm -rf /var/lib/apt/lists/*

COPY --chown=$DOCKER_USER:$DOCKER_USER \
     --from=build-world \
     /yggdrasilcore/env/dist/bin/dbimport /yggdrasilcore/env/dist/bin/dbimport

# The reference configuration of the components this image runs.
COPY --chown=$DOCKER_USER:$DOCKER_USER \
     --from=build-world \
     /yggdrasilcore/env/dist/etc/ /yggdrasilcore/env/ref/etc/

USER $DOCKER_USER

CMD [ "/yggdrasilcore/env/dist/bin/dbimport" ]

###############
# Client Data #
###############
# This image only downloads the client data, it runs no part of the core, so
# it carries neither the C++ runtime nor any of the core libraries.
FROM skeleton AS client-data

ARG USER_ID=1000
ARG GROUP_ID=1000
ARG DOCKER_USER=ycore

LABEL description="YggdrasilCore client-data"

ENV DATAPATH=/yggdrasilcore/env/dist/data

RUN apt-get update && \
    apt-get install -y --no-install-recommends curl unzip && \
    rm -rf /var/lib/apt/lists/*

RUN groupadd --gid "$GROUP_ID"  "$DOCKER_USER" && \
    useradd -d /yggdrasilcore --uid "$USER_ID"  --gid "$GROUP_ID"  "$DOCKER_USER" && \
    passwd -d "$DOCKER_USER" && \
    chown -R "$DOCKER_USER:$DOCKER_USER" /yggdrasilcore

COPY --chown=$DOCKER_USER:$DOCKER_USER apps apps

VOLUME /yggdrasilcore/env/dist/data

USER $DOCKER_USER

CMD ["bash", "/yggdrasilcore/apps/docker/download-client-data.sh" ]

##################
# Map Extractors #
##################
# The map tools do not talk to the database, so they need neither the MySQL
# client library nor the server side libraries. They do link the common
# library, which pulls in OpenSSL and the DWARF reader, and the mpq library,
# which pulls in zlib and bzip2.
FROM runtime AS tools
LABEL description="YggdrasilCore Tools"

WORKDIR /yggdrasilcore/env/dist/

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      libssl3 libdw1 zlib1g libbz2-1.0 && \
    rm -rf /var/lib/apt/lists/*

# The map tools have no configuration of their own, so the reference
# configuration directory stays empty.
RUN mkdir -pv /yggdrasilcore/env/dist/Cameras \
              /yggdrasilcore/env/dist/dbc     \
              /yggdrasilcore/env/dist/maps    \
              /yggdrasilcore/env/dist/mmaps   \
              /yggdrasilcore/env/dist/vmaps \
              /yggdrasilcore/env/ref/etc

COPY --chown=$DOCKER_USER:$DOCKER_USER --from=build-tools \
  /yggdrasilcore/env/dist/bin/map_extractor /yggdrasilcore/env/dist/bin/map_extractor

COPY --chown=$DOCKER_USER:$DOCKER_USER --from=build-tools \
  /yggdrasilcore/env/dist/bin/mmaps_generator /yggdrasilcore/env/dist/bin/mmaps_generator

COPY --chown=$DOCKER_USER:$DOCKER_USER --from=build-tools \
  /yggdrasilcore/env/dist/bin/vmap4_assembler /yggdrasilcore/env/dist/bin/vmap4_assembler

COPY --chown=$DOCKER_USER:$DOCKER_USER --from=build-tools \
  /yggdrasilcore/env/dist/bin/vmap4_extractor /yggdrasilcore/env/dist/bin/vmap4_extractor

USER $DOCKER_USER

CMD ["bash"]
