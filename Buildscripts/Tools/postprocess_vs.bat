@echo off
echo - Postprocessing VS solution (removing ALL_BUILD)
set BASE_PATH=%~dp0
"%BASE_PATH%\lua53.exe" "%BASE_PATH%\postprocess_vs_solution.lua" Crossframework.sln > Crossframework_.sln
del Crossframework.sln
move Crossframework_.sln Crossframework.sln
echo - Removing ALL_BUILD project files
del ALL_BUILD*
echo - Inserting CrossFrameworkLauncher.vcxproj.user (debug folder configuration)
copy "%BASE_PATH%\CrossFrameworkLauncher.vcxproj.user.in" Source\Shared\CrossFrameworkLauncher.vcxproj.user