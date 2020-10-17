FROM debian:buster

# We need this for our own luabind, since Debian 10 has a breaking bug in luabind.
# See https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=959501 for details.
# This might get fixed in Debian 11, at which point this repo should be dropped.
WORKDIR /root/
COPY illarion.gpg.key .
RUN apt-get update && \
    apt-get -y install gnupg2 ca-certificates && \
    cat illarion.gpg.key | apt-key add -
WORKDIR /etc/apt/sources.list.d/
COPY illarion.list .

# Copy and install the newest Illarion server package, built with CPack.
WORKDIR /root/
COPY .deb/illarion.deb .
RUN apt-get update && apt-get -y install ./illarion.deb && \
    mkdir -p /usr/share/illarion/map && \
    mkdir -p /usr/share/illarion/scripts && \
    mkdir /scripts && \
    mkdir /maps && \
    ln -s /maps /usr/share/illarion/map/import

EXPOSE 3012
VOLUME /scripts
VOLUME /maps

# Modifications of the base image (to be plugged in from github) for local use
RUN sed -i 's/^postgres_host .*/postgres_host db/' /etc/illarion.conf && \
    apt-get update && apt-get -y install default-jre-headless rsync
ADD http://illarion.org/media/localserver/compiler.jar /opt/easyCompiler/compiler.jar
COPY pre-reload /usr/share/illarion/

CMD [ "illarion", "/etc/illarion.conf" ]
