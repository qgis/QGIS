#!/usr/bin/env bash
###########################################################################
#    prepare_commit_code_fixup.sh
#    ---------------
#    Date                 : October 2020
#    Copyright            : (C) 2020 by Even Rouault
#    Email                : even.rouault@spatialys.com
###########################################################################
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
###########################################################################

set -e

TOPLEVEL=$(git rev-parse --show-toplevel)

# capture files passed by pre-commit
MODIFIED="$@"

FILES_TO_CHECK=""
for f in $MODIFIED; do
  case "$f" in
    *.h|*.cpp)
      FILES_TO_CHECK="$FILES_TO_CHECK $f"
      ;;
  esac
done

if [ -z "$FILES_TO_CHECK" ]; then
  exit 0
fi

for f in $FILES_TO_CHECK; do
  m=$f.code_fixup
  python "${TOPLEVEL}/scripts/pre_commit/prepare_commit_code_fixup.py" "$f" > "$m"
  if diff -u "$m" "$f" >/dev/null; then
    # no difference found
    rm "$m"
  else
    mv "$m" "$f"
  fi
done

exit 0
