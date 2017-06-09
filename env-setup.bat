:: Windows build script for LAME
:: Builds only the base station code
@ECHO OFF

:: Find MsBuild
for /f "usebackq tokens=*" %%i in (`.\tools\vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
  set msbuild=%%i
)

if exist "%msbuild%\MSBuild\15.0\Bin\MSBuild.exe" (
  set msbuild="%msbuild%\MSBuild\15.0\Bin"
)

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
    del BaseStation\*.dll BaseStation\*.lib BaseStation\*.pdb Robot\*.pdb
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
    del BaseStation\*.dll BaseStation\*.lib BaseStation\*.pdb
    pushd .
    
    :: JoystickLibrary, C#
    pushd
    cd JoystickLibrary\csharp
    call %msbuild%\msbuild.exe JoystickLibrary.sln /t:Rebuild /p:Configuration=Debug /p:Platform=x64 /m
    copy x64\Debug\* ..\..\BaseStation\*
    popd
    
    echo ==== Complete! ====
    popd
