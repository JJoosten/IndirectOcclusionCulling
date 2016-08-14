rem Android environment setup file
rem ################################
rem CONFIGURE THIS:

REM Default setting - expects everything to be relative to the current directory
set ANDROID_ROOT=%~dp0
set ANDROID_NDK=%ANDROID_ROOT%Ndk
set ANDROID_SDK=%ANDROID_ROOT%Sdk
set ANDROID_ANT=%ANDROID_ROOT%Ant
set JAVA_HOME=%ANDROID_ROOT%Jdk

rem ################################

IF NOT EXIST "%JAVA_HOME%\bin\java.exe" GOTO ERROR_NO_JAVA
IF NOT EXIST "%ANDROID_NDK%\prebuilt\android-arm" GOTO ERROR_NO_NDK
IF NOT EXIST "%ANDROID_SDK%\platform-tools" GOTO ERROR_NO_SDK
IF NOT EXIST "%ANDROID_SDK%\build-tools" GOTO ERROR_NO_SDK
IF NOT EXIST "%ANDROID_ANT%\bin\ant" GOTO ERROR_NO_ANT

set ANDROID_HOME=%ANDROID_SDK%
set ANDROID_SDK_HOME=%ANDROID_HOME%

set PATH=%PATH%;%JAVA_HOME%\bin;%ANDROID_HOME%\tools;%ANDROID_HOME%\platform-tools;%ANDROID_NDK%\prebuilt\windows-x86_64\bin;%ANDROID_NDK%\prebuilt\windows\bin;%ANDROID_NDK%;%ANDROID_ANT%\bin

GOTO ENDING

:ERROR_NO_JAVA
@echo ####################################
@echo # NO JAVA DETECTED (JAVA_HOME env variable)
@echo # (modify this config file or install in the following folder):
@echo # %JAVA_HOME%
@echo ####################################
timeout 30
EXIT /B 100
GOTO ENDING

:ERROR_NO_NDK
@echo ####################################
@echo # NO ANDROID NDK DETECTED
@echo # (modify this config file or install in the following folder):
@echo # %ANDROID_NDK%
@echo ####################################
timeout 30
EXIT /B 101
GOTO ENDING

:ERROR_NO_SDK
@echo ####################################
@echo # NO ANDROID SDK DETECTED
@echo # (modify this config file or install in the following folder):
@echo # %ANDROID_SDK%
@echo ####################################
timeout 30
EXIT /B 102
GOTO ENDING

:ERROR_NO_ANT
@echo ####################################
@echo # NO ANDROID APACHE ANT DETECTED:
@echo # (modify this config file or install in the following folder):
@echo # %ANDROID_ANT%
@echo ####################################
timeout 30
EXIT /B 103
GOTO ENDING

:ENDING
rem End.