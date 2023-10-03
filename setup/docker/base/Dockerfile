FROM debian:bullseye AS base

COPY *.deb /tmp
COPY pre-reload /usr/share/illarion/

RUN \
    apt-get -qq update && \
    export DEBIAN_FRONTEND=noninteractive && \
    apt-get -y -qq install default-jre-headless rsync wget && \
    find /tmp -name *.deb | xargs apt-get -y -qq install && \
    apt-get -qq clean && \
    rm /tmp/*.deb && \
    # Download the easyCompiler
    mkdir -p /opt/easyCompiler && \
    wget https://illarion.org/media/localserver/compiler.jar -O /opt/easyCompiler/compiler.jar -q && \    
    # Setup directories for the server.
    mkdir -p /usr/share/illarion/map && \
    mkdir -p /usr/share/illarion/scripts && \
    mkdir /scripts && \
    mkdir /maps && \
    ln -s /maps /usr/share/illarion/map/import && \
    # Setup access to script and maps directory
    chmod a+rw /usr/share/illarion/scripts /usr/share/illarion/map

EXPOSE 3012
VOLUME /scripts
VOLUME /maps
VOLUME /usr/share/illarion/map

CMD [ "illarion", "/etc/illarion.conf" ]
