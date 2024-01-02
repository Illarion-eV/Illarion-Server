# [Choice] Debian version: buster, stretch
ARG VARIANT="bullseye"
FROM buildpack-deps:${VARIANT}-curl

# [Option] Install zsh
ARG INSTALL_ZSH="true"
# [Option] Upgrade OS packages to their latest versions
ARG UPGRADE_PACKAGES="true"
# [Option] Enable non-root Docker access in container
ARG ENABLE_NONROOT_DOCKER="true"
# Install needed packages and setup non-root user. Use a separate RUN statement to add your own dependencies.
ARG SOURCE_SOCKET=/var/run/docker-host.sock
ARG TARGET_SOCKET=/var/run/docker.sock
ARG USERNAME=vscode
ARG USER_UID=automatic
ARG USER_GID=$USER_UID
COPY library-scripts/*.sh library-scripts/*.env /tmp/library-scripts/
RUN apt-get update && \
    /bin/bash /tmp/library-scripts/common-debian.sh "${INSTALL_ZSH}" "${USERNAME}" "${USER_UID}" "${USER_GID}" "${UPGRADE_PACKAGES}" "true" "true" && \
    # Use Docker script from script library to set things up
    /bin/bash /tmp/library-scripts/docker-debian.sh "${ENABLE_NONROOT_DOCKER}" "${SOURCE_SOCKET}" "${TARGET_SOCKET}" "${USERNAME}" "false" && \
    # Clean up
    apt-get autoremove -y && apt-get clean -y && rm -rf /var/lib/apt/lists/* /tmp/library-scripts/

RUN \
    export DEBIAN_FRONTEND=noninteractive && \
    apt-get update && \
    apt-get -y --no-install-recommends install \
        file ninja-build psmisc \
        default-jre-headless rsync wget \
        g++ gdb \
        libboost-graph-dev libboost-system-dev libpq-dev lua5.2-dev librange-v3-dev && \
    apt-get autoremove -y && \
    apt-get clean -y && \
    rm -rf /var/lib/apt/lists/* && \
    mkdir -p /tmp/cmake && cd /tmp/cmake && \
    wget https://github.com/Kitware/CMake/releases/download/v3.21.1/cmake-3.21.1-linux-x86_64.sh -O /tmp/cmake/cmake.sh -q && \
    sh cmake.sh --skip-license --prefix=/usr/local && cd && rm -rf /tmp/cmake

RUN \
    # Download the easyCompiler
    mkdir -p /opt/easyCompiler && \
    wget https://illarion.org/media/localserver/compiler.jar -O /opt/easyCompiler/compiler.jar -q && \    
    # Setup directories for the server.
    mkdir -p /usr/share/illarion/map && \
    mkdir -p /usr/share/illarion/scripts && \
    chown -R ${USERNAME}:root /usr/share/illarion/map && \
    chown -R ${USERNAME}:root /usr/share/illarion/scripts && \
    mkdir /scripts && \
    mkdir /maps && \
    ln -s /maps /usr/share/illarion/map/import && \
    # Setup build directory
    mkdir -p /tmp/illarion/build && \
    chown -R ${USERNAME}:root /tmp/illarion/build && \
    # Setup VSCode directory
    mkdir -p /tmp/vscode && \
    chown -R ${USERNAME}:root /tmp/vscode

COPY library-scripts/pre-reload /usr/share/illarion/
COPY library-scripts/linter-*.sh /opt/linter/

RUN \
    chown ${USERNAME}:root /usr/share/illarion/pre-reload && \
    chmod u+x /usr/share/illarion/pre-reload && \
    find /opt/linter -name linter-*.sh | xargs chown ${USERNAME}:root && \
    find /opt/linter -name linter-*.sh | xargs chmod u+x

USER ${USERNAME}
RUN \
    # Configure git
    git config --global http.sslBackend gnutls

EXPOSE 3012
VOLUME /scripts
VOLUME /maps
