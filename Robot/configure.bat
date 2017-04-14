@ECHO OFF

git pull
pushd .
IF EXIST ".\build" rd /s /q build
mkdir build && cd build
cmake -DCMAKE_GENERATOR_PLATFORM=x64 ..
popd
IF /i "%1"=="/q" (
    exit
) 
build\LAME.sln