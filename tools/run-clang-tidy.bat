@echo off
pushd %~dp0
docker-compose -f clang-tidy.yml run --rm clang-tidy
popd
