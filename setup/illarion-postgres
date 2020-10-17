FROM postgres:11.9
WORKDIR /docker-entrypoint-initdb.d
COPY init-db.sh 1-init-db.sh
COPY test-chars.sql 2-test-chars.sql
RUN chmod 777 . && apt-get update && apt-get -y install wget
