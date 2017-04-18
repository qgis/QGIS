#!/bin/bash
DIR=`dirname $0`
git status -s | sed -ne 's/^ *[AM]  *//p' | xargs "$DIR/astyle.sh"
