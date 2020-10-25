FROM debian:buster AS base

COPY illarion.deb /tmp/illarion.deb
COPY pre-reload /usr/share/illarion/

RUN \
    apt-get -qq update && \
    export DEBIAN_FRONTEND=noninteractive && \
    apt-get -y -qq install default-jre-headless rsync wget && \
    apt-get -y -qq install /tmp/illarion.deb && \
    apt-get -qq clean && \
    rm /tmp/illarion.deb && \
    # Download the easyCompiler
    mkdir -p /opt/easyCompiler && \
    wget http://illarion.org/media/localserver/compiler.jar -O /opt/easyCompiler/compiler.jar -q && \    
    # Setup directories for the server.
    mkdir -p /usr/share/illarion/map && \
    mkdir -p /usr/share/illarion/scripts && \
    mkdir /scripts && \
    mkdir /maps && \
    ln -s /maps /usr/share/illarion/map/import

EXPOSE 3012
VOLUME /scripts
VOLUME /maps

CMD [ "illarion", "/etc/illarion.conf" ]
