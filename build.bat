mkdir windows

del windows\mulc.exe

cmake -S . -B build
cmake --build build --config Release

copy build\Release\mulc.exe windows\