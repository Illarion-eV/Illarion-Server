@echo off
pushd %~dp0
docker-compose -f cppcheck.yml run --rm cppcheck
popd