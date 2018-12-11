#!/bin/bash

################################################################################
# Copyright (c) 2018 Intel Corporation 
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

# To stich trusty image, LKBIN_DIR should be defined
#export LKBIN_DIR=
export COMPILE_TOOLCHAIN=
export BUILD_DIR=$PWD/out/

${LKBIN_DIR:? Error: LKBIN_DIR should be defined!}

make

cp ${LKBIN_DIR}lk.elf ${BUILD_DIR}

# File sizes in 512-byte blocks

s=$(stat -c%s "out/trusty_loader.bin")
LoaderStart=0
LoaderCount=$(((s + 511) / 512))

echo s = $s
echo LoaderCount = $LoaderCount

s=$(stat -c%s "out/lk.elf")
TrustyStart=$((LoaderStart + LoaderCount))
TrustyCount=$(((s + 511) / 512))

echo TrustyStart = $TrustyStart 

# Build the loader binary

dd if=${BUILD_DIR}trusty_loader.bin of=${BUILD_DIR}trusty_pkg.bin seek=$LoaderStart
dd if=${BUILD_DIR}lk.elf of=${BUILD_DIR}trusty_pkg.bin seek=$TrustyStart
