#!/bin/bash

set -e

find . \( -name "*.astyle" -o -name "*.iostream" \) -exec rm {} \;
svn revert -R .
svn update
patch -p0 --dry-run <qgslogger-before.diff
patch -p0 <qgslogger-before.diff
mv qgslogger-before.diff qgslogger-before.diff.orig
svn diff >qgslogger-before.diff
diff -u qgslogger-before.diff.orig qgslogger-before.diff
