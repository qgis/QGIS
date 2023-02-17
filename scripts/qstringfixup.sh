#!/usr/bin/env bash
###########################################################################
#    qstringfixup.sh
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

TOPLEVEL=$(git rev-parse --show-toplevel)

cd "$TOPLEVEL" || exit

# GNU prefix command for bsd/mac os support (gsed, gsplit)
GP=
if [[ "$OSTYPE" == *bsd* ]] || [[ "$OSTYPE" =~ darwin* ]]; then
  GP=g
fi

if test "$1" == "--all"; then
    MODIFIED=$(find src tests -name "*.h" -o -name "*.cpp")
else
    # determine changed files
    MODIFIED=$(git status --porcelain| ${GP}sed -ne "s/^ *[MA]  *//p" | sort -u)
fi

if [ -z "$MODIFIED" ]; then
  echo nothing was modified
  exit 0
fi

for f in $MODIFIED; do

  case "$f" in
  *.cpp|*.h)
    ;;

  *)
    continue
    ;;
  esac

  m=$f.qstringfixup
  python "${TOPLEVEL}/scripts/qstringfixup.py" "$f" > "$m"
  if diff -u "$m" "$f" >/dev/null; then
    # no difference found
    rm "$m"
  else
    echo "Patching $f"
    mv "$m" "$f"
  fi
done
