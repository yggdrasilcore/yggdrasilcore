ARG UBUNTU_VERSION=22.04 # lts

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

# This target builds the docker image
# This target can be useful to inspect the explicit outputs from the build,
FROM skeleton AS build

ARG CTOOLS_BUILD="all"
ARG CTYPE="RelWithDebInfo"
ARG CCACHE_CPP2="true"
ARG CSCRIPTPCH="OFF"
ARG CSCRIPTS="static"
ARG CMODULES="static"
ARG CSCRIPTS_DEFAULT_LINKAGE="static"
ARG CWITH_WARNINGS="ON"
ARG CMAKE_EXTRA_OPTIONS=""
ARG GIT_DISCOVERY_ACROSS_FILESYSTEM=1

ARG CCACHE_DIR="/ccache"
ARG CCACHE_MAXSIZE="1000MB"
ARG CCACHE_SLOPPINESS="pch_defines,time_macros,include_file_mtime"
ARG CCACHE_COMPRESS=""
ARG CCACHE_COMPRESSLEVEL="9"
ARG CCACHE_COMPILERCHECK="content"
ARG CCACHE_LOGFILE=""

RUN apt-get update                                                     \
    && apt-get install -y --no-install-recommends                         \
        build-essential ccache libtool cmake-data ninja-build cmake clang    \
        git lsb-base curl unzip default-mysql-client openssl                  \
        default-libmysqlclient-dev libboost-all-dev libssl-dev libmysql++-dev \
        libreadline-dev zlib1g-dev libbz2-dev libncurses5-dev         \
    && dpkg -s libmysqlclient-dev | grep '^Version' | cut -d' ' -f2 > /mysql-client-version \
    && rm -rf /var/lib/apt/lists/*

COPY CMakeLists.txt /yggdrasilcore/CMakeLists.txt
COPY conf /yggdrasilcore/conf
COPY deps /yggdrasilcore/deps
COPY src /yggdrasilcore/src
COPY modules /yggdrasilcore/modules

ARG CACHEBUST=1

WORKDIR /yggdrasilcore/build

# BuildKit keys a RUN on the contents of the contexts it binds, and the checkout
# rewrites `.git` on every run. Stage the checkout metadata into a cache mount
# first (this writes nothing to the image), then compile with only cache mounts
# attached. Cache mounts are excluded from the cache key and persist between
# builds, so an unchanged revision reuses the compile layer instead of rebuilding.
RUN --mount=type=bind,source=.git,target=/git-seed \
    --mount=type=cache,target=/yggdrasilcore/.git,id=yggdrasilcore-git,sharing=locked \
    find /yggdrasilcore/.git -mindepth 1 -delete 2>/dev/null || true; \
    cp -a /git-seed/. /yggdrasilcore/.git/

RUN --mount=type=cache,target=/ccache,sharing=locked \
                                                                            
                                                                           
                                                                                
                                                                               
                                     
    --mount=type=cache,target=/yggdrasilcore/.git,id=yggdrasilcore-git,sharing=locked \
    git config --global --add safe.directory /yggdrasilcore \
    && cmake /yggdrasilcore \
       -G Ninja \
       -DCMAKE_INSTALL_PREFIX="/yggdrasilcore/env/dist"  \
       -DAPPS_BUILD="all"                              \
       -DTOOLS_BUILD="$CTOOLS_BUILD"                   \
       -DSCRIPTS="$CSCRIPTS"                           \
       -DMODULES="$CMODULES"                           \
       -DWITH_WARNINGS="$CWITH_WARNINGS"               \
       -DCMAKE_BUILD_TYPE="$CTYPE"                     \
       -DCMAKE_CXX_COMPILER="clang++"                  \
       -DCMAKE_C_COMPILER="clang"                      \
       -DCMAKE_CXX_COMPILER_LAUNCHER="ccache"          \
       -DCMAKE_C_COMPILER_LAUNCHER="ccache"            \
       -DBoost_USE_STATIC_LIBS="ON"                    \
       -DLUA_VERSION="luajit"                          \
    && cmake --build . --config "$CTYPE" -j $(($(nproc) + 1)) \
    && cmake --install . --config "$CTYPE"

#############################
# Base runtime for services #
#############################

FROM skeleton AS runtime

ARG USER_ID=1000
ARG GROUP_ID=1000
ARG DOCKER_USER=ycore

ENV YCORE_COMPONENT=undefined

# Install base dependencies for yggdrasilcore
COPY --from=build /mysql-client-version /mysql-client-version

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      libmysqlclient21=$(cat /mysql-client-version) libreadline8 \
      gettext-base default-mysql-client \
      gdb gdbserver && \
    rm -rf /var/lib/apt/lists/*

COPY apps/startup-scripts/gdb.conf /yggdrasilcore/gdb.conf

COPY --from=build /yggdrasilcore/env/dist/etc/ /yggdrasilcore/env/ref/etc

VOLUME /yggdrasilcore/env/dist/etc

ENV PATH="/yggdrasilcore/env/dist/bin:$PATH"

RUN groupadd --gid "$GROUP_ID"  "$DOCKER_USER" && \
    useradd -d /yggdrasilcore --uid "$USER_ID"  --gid "$GROUP_ID"  "$DOCKER_USER" && \
    passwd -d "$DOCKER_USER"
                                                     

COPY --chown=$DOCKER_USER:$DOCKER_USER \
     --chmod=755 \
     apps/docker/entrypoint.sh /yggdrasilcore/entrypoint.sh

RUN mkdir -p /yggdrasilcore/env/dist/logs && \
    mkdir -p /yggdrasilcore/monitor && \
    chown -R "$DOCKER_USER:$DOCKER_USER" /yggdrasilcore && \
    chmod -R 755 /yggdrasilcore/env/dist/logs && \
    chmod -R 755 /yggdrasilcore/monitor

USER $DOCKER_USER

ENTRYPOINT ["/usr/bin/env", "bash", "/yggdrasilcore/entrypoint.sh"]

###############
# Auth Server #
###############

FROM runtime AS authserver
LABEL description="YggdrasilCore Auth Server"

ENV YCORE_COMPONENT=authserver
# Don't run database migrations. We can leave that up to the db-import container
ENV YC_UPDATES_ENABLE_DATABASES=0
# This disables user prompts. The console is still active, however
ENV YC_DISABLE_INTERACTIVE=1
ENV YC_CLOSE_IDLE_CONNECTIONS=0

COPY --chown=$DOCKER_USER:$DOCKER_USER \
     --from=build \
     /yggdrasilcore/env/dist/bin/authserver /yggdrasilcore/env/dist/bin/authserver


CMD ["authserver"]

################
# World Server #
################

FROM runtime AS worldserver

LABEL description="YggdrasilCore World Server"

ENV YCORE_COMPONENT=worldserver
# Don't run database migrations. We can leave that up to the db-import container
ENV YC_UPDATES_ENABLE_DATABASES=0
# This disables user prompts. The console is still active, however
ENV YC_DISABLE_INTERACTIVE=1
ENV YC_CLOSE_IDLE_CONNECTIONS=0

COPY --chown=$DOCKER_USER:$DOCKER_USER \
     --from=build \
     /yggdrasilcore/env/dist/bin/worldserver /yggdrasilcore/env/dist/bin/worldserver

VOLUME /yggdrasilcore/env/dist/etc

CMD ["worldserver"]

#############
# DB Import #
#############

FROM runtime AS db-import

LABEL description="YggdrasilCore Database Import tool"

USER $DOCKER_USER

ENV YCORE_COMPONENT=dbimport

COPY --chown=$DOCKER_USER:$DOCKER_USER\
     --from=build \
     /yggdrasilcore/env/dist/bin/dbimport /yggdrasilcore/env/dist/bin/dbimport

CMD [ "/yggdrasilcore/env/dist/bin/dbimport" ]

###############
# Client Data #
###############

FROM skeleton AS client-data

LABEL description="YggdrasilCore client-data"

ENV DATAPATH=/yggdrasilcore/env/dist/data

RUN apt-get update && \
    apt-get install -y curl unzip && \
    rm -rf /var/lib/apt/lists/*

COPY --chown=$DOCKER_USER:$DOCKER_USER apps apps

VOLUME /yggdrasilcore/env/dist/data

USER $DOCKER_USER

CMD ["bash", "/yggdrasilcore/apps/docker/download-client-data.sh" ]

##################
# Map Extractors #
##################

FROM runtime AS tools

LABEL description="YggdrasilCore Tools"

WORKDIR /yggdrasilcore/env/dist/

RUN mkdir -pv /yggdrasilcore/env/dist/Cameras \
              /yggdrasilcore/env/dist/dbc     \
              /yggdrasilcore/env/dist/maps    \
              /yggdrasilcore/env/dist/mmaps   \
              /yggdrasilcore/env/dist/vmaps

COPY --chown=$DOCKER_USER:$DOCKER_USER --from=build \
  /yggdrasilcore/env/dist/bin/map_extractor /yggdrasilcore/env/dist/bin/map_extractor

COPY --chown=$DOCKER_USER:$DOCKER_USER --from=build \
  /yggdrasilcore/env/dist/bin/mmaps_generator /yggdrasilcore/env/dist/bin/mmaps_generator

COPY --chown=$DOCKER_USER:$DOCKER_USER --from=build \
  /yggdrasilcore/env/dist/bin/vmap4_assembler /yggdrasilcore/env/dist/bin/vmap4_assembler

COPY --chown=$DOCKER_USER:$DOCKER_USER --from=build \
  /yggdrasilcore/env/dist/bin/vmap4_extractor /yggdrasilcore/env/dist/bin/vmap4_extractor
