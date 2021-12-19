#!/usr/bin/env bash

find -E . -type f \
    -regex '.*\.(cpp|c|hpp|h|js)' \
    -not -path "./build/*" \
    -not -path "./Thirdparty/*" \
    -not -name './Viewer/libOpenDrive.js' \
    -not -path './Viewer/lib/*' \
    -not -path './Viewer/glad/*' \
    -not -path './Viewer/KHR/*' \
    -exec clang-format --style=file -i '{}' \;
