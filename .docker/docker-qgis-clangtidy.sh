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

# This is needed for `git status` to work, see
# https://github.com/qgis/QGIS/runs/6733585841?check_suite_focus=true#step:13:89
git config --global --add safe.directory ${SRCDIR}

echo "::group::Install clang tidy"
apt install -y \
    clang-tidy
echo "::endgroup::"

cd ${SRCDIR}

echo "::group::Download clang-tidy-diff"
curl -XGET https://raw.githubusercontent.com/llvm/llvm-project/llvmorg-14.0.6/clang-tools-extra/clang-tidy/tool/clang-tidy-diff.py -o clang-tidy-diff.py
echo "::endgroup::"

echo "${bold}Run clang-tidy on modifications...${endbold}"
CLANG_TIDY_CHECKS=$(cat tests/code_layout/clangtidy_checks.txt | grep -ve "^#" | grep -ve "^$" | tr -d '\n')
git diff -U0 HEAD^ | python3 clang-tidy-diff.py -p1 -path=${CTEST_BUILD_DIR} -use-color -checks="$CLANG_TIDY_CHECKS" | tee clang-tidy.log

echo -e "\e[1;34mTo reproduce locally:"
echo -e "\e[1;34m - launch cmake with option -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
echo -e "\e[1;34m - update build by calling Ninja"
echo -e "\e[1;34m - launch command ./scripts/clang-tidy.sh -p <your_build_dir> <source_file>"

exit $(grep -c "warning:" clang-tidy.log)
