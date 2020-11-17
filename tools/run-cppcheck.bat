@echo off
pushd %~dp0
docker-compose -f cppcheck.yml run --rm cppcheck sh -c "cppcheck --std=c++17 --enable=warning,style,performance,portability --inconclusive -j $(nproc) --error-exitcode=1 -q --suppress=functionStatic:src/db/InsertQuery.hpp -Isrc/ src/"
popd
