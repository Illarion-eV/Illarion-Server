@echo off
pushd %~dp0
docker-compose -f clang-format.yml run --rm clang-format
popd