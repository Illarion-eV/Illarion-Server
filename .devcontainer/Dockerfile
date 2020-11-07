# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.145.1/containers/cpp/.devcontainer/base.Dockerfile

ARG VARIANT="buster"
FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT}

RUN \
    apt-get update && \
    export DEBIAN_FRONTEND=noninteractive && \
    apt-get -y --no-install-recommends install file ninja-build psmisc && \
    apt-get -y --no-install-recommends install default-jre-headless rsync wget && \
    apt-get -y --no-install-recommends install libboost-graph-dev libboost-system-dev libpqxx-dev lua5.2-dev librange-v3-dev && \
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
    mkdir /scripts && \
    mkdir /maps && \
    ln -s /maps /usr/share/illarion/map/import && \
    # Configure git
    git config --global http.sslBackend gnutls

COPY library-scripts/pre-reload /usr/share/illarion/

EXPOSE 3012
VOLUME /scripts
VOLUME /maps