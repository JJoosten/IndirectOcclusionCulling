
cd Android
@echo Make sure these paths are all correct!
mklink /J Ndk "C:\Program Files (x86)\Microsoft Visual Studio 14.0\Apps\android-ndk-r10"
mklink /J Sdk "C:\Program Files (x86)\Android\android-sdk"
mklink /J Ant "C:\Program Files (x86)\Microsoft Visual Studio 14.0\Apps\apache-ant-1.9.3"
mklink /J Jdk "C:\Program Files (x86)\Java\jdk1.7.0_55"
@echo off
call config.bat
timeout 5