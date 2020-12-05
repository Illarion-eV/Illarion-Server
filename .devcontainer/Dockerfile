# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.145.1/containers/cpp/.devcontainer/base.Dockerfile

ARG VARIANT="buster"
FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT}

# [Option] Install zsh
ARG INSTALL_ZSH="true"
# [Option] Upgrade OS packages to their latest versions
ARG UPGRADE_PACKAGES="false"
# [Option] Enable non-root Docker access in container
ARG ENABLE_NONROOT_DOCKER="true"

# Install needed packages and setup non-root user. Use a separate RUN statement to add your own dependencies.
ARG SOURCE_SOCKET=/var/run/docker-host.sock
ARG TARGET_SOCKET=/var/run/docker.sock
ARG USERNAME=vscode
ARG USER_UID=automatic
ARG USER_GID=$USER_UID
COPY library-scripts/*-debian.sh /tmp/library-scripts/
RUN apt-get update && \
    /bin/bash /tmp/library-scripts/common-debian.sh "${INSTALL_ZSH}" "${USERNAME}" "${USER_UID}" "${USER_GID}" "${UPGRADE_PACKAGES}" && \
    # Use Docker script from script library to set things up
    /bin/bash /tmp/library-scripts/docker-debian.sh "${ENABLE_NONROOT_DOCKER}" "${SOURCE_SOCKET}" "${TARGET_SOCKET}" "${USERNAME}" && \
    # Clean up
    apt-get autoremove -y && apt-get clean -y && rm -rf /var/lib/apt/lists/* /tmp/library-scripts/

RUN \
    export DEBIAN_FRONTEND=noninteractive && \
    apt-get update && \
    apt-get -y --no-install-recommends install \
        file ninja-build psmisc \
        default-jre-headless rsync wget \
        libboost-graph-dev libboost-system-dev libpqxx-dev lua5.2-dev librange-v3-dev && \
    apt-get autoremove -y && \
    apt-get clean -y && \
    rm -rf /var/lib/apt/lists/*

RUN \
    # Download the easyCompiler
    mkdir -p /opt/easyCompiler && \
    wget http://illarion.org/media/localserver/compiler.jar -O /opt/easyCompiler/compiler.jar -q && \    
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
    chown -R ${USERNAME}:root /tmp/illarion/build

COPY library-scripts/pre-reload /usr/share/illarion/

USER ${USERNAME}
RUN \
    # Configure git
    git config --global http.sslBackend gnutls

EXPOSE 3012
VOLUME /scripts
VOLUME /maps