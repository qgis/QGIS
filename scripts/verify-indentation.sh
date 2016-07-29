#!/bin/bash

cd $(git rev-parse --show-toplevel)

export PATH=$PATH:$PWD/scripts

if [ -z "$TRAVIS_COMMIT_RANGE" ]; then
	echo "No commit range given"
	exit 0
fi

if ! type -p astyle.sh >/dev/null; then
	echo astyle.sh not found
	exit 1
fi

set -e

ASTYLEDIFF=/tmp/astyle.diff
>$ASTYLEDIFF

case "${TRAVIS_COMMIT_RANGE}" in
*...*)
	curl https://api.github.com/repos/$TRAVIS_REPO_SLUG/compare/$TRAVIS_COMMIT_RANGE | jq -r .files[].filename >/tmp/changed-files
	;;

*)
	git diff --name-only $TRAVIS_COMMIT_RANGE >/tmp/changed-files
	;;
esac

while read f
do
	if ! [ -f "$f" ]; then
		echo "$f was removed." >>/tmp/ctest-important.log
		continue
	fi

	echo "Checking $f" >>/tmp/ctest-important.log
	case "$f" in
	src/core/gps/qextserialport/*|src/plugins/dxf2shp_converter/dxflib/src/*|src/plugins/globe/osgEarthQt/*|src/plugins/globe/osgEarthUtil/*|scripts/customwidget_template*)
		echo "$f skipped"
		continue
		;;

	*.cpp|*.c|*.h|*.cxx|*.hxx|*.c++|*.h++|*.cc|*.hh|*.C|*.H|*.sip|*.py)
		;;

	*)
		continue
		;;
	esac

	m="$f.prepare"
	cp "$f" "$m"
	astyle.sh "$f"
	if diff -u "$m" "$f" >>$ASTYLEDIFF; then
		rm "$m"
	else
		echo "File $f needs indentation"
	fi
done </tmp/changed-files

if [ -s "$ASTYLEDIFF" ]; then
	echo
	echo "Required indentation updates:"
	cat "$ASTYLEDIFF"

	cat <<EOF

Tips to prevent and resolve:
* Enable WITH_ASTYLE in your cmake configuration to format C++ code
* Install autopep8 (>= 1.2.1) to format python code
* Use "scripts/astyle.sh file" to fix the now badly indented files
* Consider using scripts/prepare-commit.sh as pre-commit hook to avoid this
  in the future (ln -s ../../scripts/prepare-commit.sh .git/hooks/pre-commit) or
  run it manually before each commit.
EOF

	exit 1
fi
