if not exist %USERPROFILE%\vcpkg-binary-cache mkdir %USERPROFILE%\vcpkg-binary-cache
set VCPKG_BINARY_SOURCES=clear;files,%USERPROFILE%\vcpkg-binary-cache,readwrite