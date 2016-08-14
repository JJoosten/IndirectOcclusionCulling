cmake --help >nul 2>&1
IF NOT %errorlevel%==9009 GOTO SUCCESS_HAS_CMAKE
IF EXIST "C:\Program Files (x86)\CMake\bin\cmake.exe" GOTO FOUND_CMAKE

:ERROR_NO_CMAKE
@echo #############################################
@echo.
@echo CMake is not installed (correctly)!
@echo.
@echo IMPORTANT: DURING INSTALLATION, CHANGE:
@echo    "Do not add CMake to the system PATH"
@echo TO:
@echo    "Add CMake to the system PATH for all users"
@echo.
@echo #############################################
@timeout 30
EXIT /b -1

:FOUND_CMAKE
SET PATH="C:\Program Files (x86)\CMake\bin";%PATH%

:SUCCESS_HAS_CMAKE