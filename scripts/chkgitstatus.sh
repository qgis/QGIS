#!/bin/sh

srcdir=$(dirname $0)/..

command -v git > /dev/null || {
  echo "git is not found in PATH" >&2
  exit 2
}

if test "$1" = "log"; then
  git -C ${srcdir} status -uno | grep 'modified: ' | sort -u > .gitstatus
  exit 0
elif test "$1" = "check"; then
  if git -C ${srcdir} status -uno | grep 'modified: ' | sort -u | diff - .gitstatus; then
    exit 0
  else
    echo "Source files (printed above) were modified. Diff follows:"
    git -C ${srcdir} diff
    exit 1
  fi
else
  echo "Usage: $0 [log|check]" >&2
  exit 1
fi
