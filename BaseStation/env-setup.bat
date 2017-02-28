:: Dependency script for LAME
@ECHO OFF

:: MSBUILD directories
@if exist "%ProgramFiles%\MSBuild\14.0\bin" set msbuild="%ProgramFiles%\MSBuild\14.0\bin"
@if exist "%ProgramFiles(x86)%\MSBuild\14.0\bin" set msbuild="%ProgramFiles(x86)%\MSBuild\14.0\bin"

:: /f or clean as cmd args?
IF /I [%1]==[] (
    goto MAYBE_CLONE_DEP
) ELSE ( 
    goto ARG_PARSE
)

:ARG_PARSE
    IF /i "%1"=="clean" goto CLEAN
    IF /i "%1"=="/f" goto CLONE_DEP
    goto :eof

:CLEAN
    IF EXIST ".\JoystickLibrary" ( rd /s /q JoystickLibrary )
    del *.dll *.lib
    goto :eof

:: If no switches specified, go down this path
:MAYBE_CLONE_DEP
    IF EXIST ".\JoystickLibrary" (
        echo JoystickLibrary already exists, will not override. Use /f switch to override.
        cd JoystickLibrary 
        git pull
        cd ..
    ) ELSE (
         git clone https://github.com/WisconsinRobotics/JoystickLibrary
    )

    goto BUILD_DEP

:: Force clean issued
:CLONE_DEP
    IF EXIST ".\JoystickLibrary" ( rd /s /q JoystickLibrary )
    git clone https://github.com/WisconsinRobotics/JoystickLibrary
    goto BUILD_DEP

:: Actually build the dependencies
:BUILD_DEP
    :: Ensure repo is up to date
    git pull
    echo ==== Building Dependencies ====
    del *.dll *.lib
    pushd .
    
    :: JoystickLibrary, C#
    pushd
    cd JoystickLibrary\csharp
    call %msbuild%\msbuild.exe JoystickLibrary.sln /t:Rebuild /p:Configuration=Debug /p:Platform=x64 /m
    copy x64\Debug\JoystickLibrary.* ..\..
    popd
    
    echo ==== Complete! ====
    popd
