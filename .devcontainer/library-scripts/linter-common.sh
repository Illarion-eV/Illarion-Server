#!/usr/bin/env bash

REUSE_CONTAINER=${ILLARION_REUSE_LINTER_CONTAINER:-false}
IMAGE_ID=$1
shift
EXECUTABLE=$1
shift
LINTER_NAME=$1
shift

CONTAINER_ID=$(basename "$(cat /proc/1/cpuset)")

if [[ "${REUSE_CONTAINER}" -eq "true" ]]
then
    LINTER_CONTAINER_ID=$(docker container ls --quiet --filter "label=illarion.linter=${LINTER_NAME}" --filter "label=illarion.container.parent=${CONTAINER_ID}" --latest)

    if [[ -z "${LINTER_CONTAINER_ID}" ]]
    then
        docker pull ${IMAGE_ID} > /dev/null
        LINTER_CONTAINER_ID=$(docker run --detach --rm --volumes-from ${CONTAINER_ID} --label illarion.linter=${LINTER_NAME} --label illarion.container.parent=${CONTAINER_ID} ${IMAGE_ID} sleep infinity)
    fi

    docker exec ${LINTER_CONTAINER_ID} ${EXECUTABLE} "$@"
else
    docker pull ${IMAGE_ID} > /dev/null
    docker run --rm --volumes-from ${CONTAINER_ID} ${IMAGE_ID} ${EXECUTABLE} "$@"
fi