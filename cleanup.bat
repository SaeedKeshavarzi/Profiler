IF EXIST ".\.vs" (
    rmdir ".\.vs" /s /q
)

IF EXIST ".\bin" (
    rmdir ".\bin" /s /q
)

IF EXIST ".\obj" (
    rmdir ".\obj" /s /q
)

IF EXIST ".\ipch" (
    rmdir ".\ipch" /s /q
)

IF EXIST ".\x64" (
    rmdir ".\x64" /s /q
)

IF EXIST ".\release" (
    rmdir ".\release" /s /q
)

IF EXIST ".\debug" (
    rmdir ".\debug" /s /q
)

IF EXIST ".\Makefile" (
    del ".\Makefile" /s /q
)

del *.db

del *.Debug

del *.Release

del *.stash

del *.user