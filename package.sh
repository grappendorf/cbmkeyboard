#!/bin/bash
PROJECT_DIR=cbmkeyboard
VERSION=$(cat version.txt)
mkdir -p target
rm -f target/*.zip
cp target/cbmkeyboard.hex .
cd ..
zip ${PROJECT_DIR}/target/cbmkeyboard-$VERSION.zip ${PROJECT_DIR} -r \
    --exclude ${PROJECT_DIR}/.git/\* \
    --exclude ${PROJECT_DIR}/build/\* \
    --exclude ${PROJECT_DIR}/target/\* \
    --exclude ${PROJECT_DIR}/\*/eagle.epf \
    --exclude ${PROJECT_DIR}/eagle/\*.b\#? \
    --exclude ${PROJECT_DIR}/eagle/\*.s\#?
cd ${PROJECT_DIR}
rm cbmkeyboard.hex

