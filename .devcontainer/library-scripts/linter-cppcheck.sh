#!/usr/bin/env bash

LINTER_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
"${LINTER_DIR}/linter-common.sh" "ghcr.io/illarion-ev/illarion-server-tools-cppcheck:sha-94c41a15" "cppcheck" "cppcheck" "$@"
