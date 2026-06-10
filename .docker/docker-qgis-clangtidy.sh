#!/usr/bin/env bash
###########################################################################
#    docker-qgis-clangtidy.sh
#    ---------------------
#    Date                 : September 2022
#    Copyright            : (C) 2022 by Julien Cabieces
#    Email                : julien dot cabieces at oslandia dot com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

set -e

SRCDIR=${CTEST_SOURCE_DIR-/root/QGIS}
cd ${SRCDIR}

# Variables for output styling
bold=$(tput bold)
endbold=$(tput sgr0)

# This is needed for `git status` to work, see
# https://github.com/qgis/QGIS/runs/6733585841?check_suite_focus=true#step:13:89
git config --global --add safe.directory ${SRCDIR}

# Note: The clang-tidy version installed needs to match the one used to compile QGIS.
# Otherwise, it will not be able to inspect the modified files.
echo "::group::Install clang tidy"
apt install -y clang-tidy-22
echo "::endgroup::"

cd ${SRCDIR}

# Unity build doesn't write in compile_commands.json the targets used to build intermediate object
# *.o files which are required by clang-tidy. For instance, It doesn't know all include sub
# paths (core/project/, core/processing/, core/proj/ ...)
#
# Precompiled headers generate errors because they come from a different CI step, so the
# timestamps don't match the just-downloaded source files
#
# So we disable both
echo "::group::Disable unity build and precompiled headers..."
cmake . -B build -DENABLE_UNITY_BUILDS=OFF -DUSE_PRECOMPILED_HEADERS=OFF
echo "::endgroup::"

# Make the clang-tidy report warnings as errors
# TODO We are currently using clang 20. When switching to clang 21, we could
# directly add this option to clang-tidy-diff call
echo "WarningsAsErrors: '*'" >> .clang-tidy

echo "${bold}Run clang-tidy on modifications...${endbold}"

# We need to add build/src/test dir as extra include directories because when clang-tidy tries to process qgstest.h
# it has no compile_commands.json instructions to know what are include directories
# It manages to figure out for other headers though, I don't get how...
if ! git diff $DIFF_RANGE | clang-tidy-diff-22.py -p1 -path=${CTEST_BUILD_DIR} -use-color -extra-arg=-I${CTEST_BUILD_DIR}/src/test/; then
  echo -e "\e[1;34mTo reproduce locally:"
  echo -e "\e[1;34m - launch cmake with option -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
  echo -e "\e[1;34m - update build by calling Ninja"
  echo -e "\e[1;34m - launch command: clang-tidy -p <your_build_dir> <source_file>"
  exit 1
fi
