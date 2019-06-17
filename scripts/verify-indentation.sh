#!/usr/bin/env bash
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
true > $ASTYLEDIFF


if [[ -n  $TRAVIS_PULL_REQUEST_BRANCH  ]]; then
  # if on a PR, just analyze the changed files
  echo "TRAVIS PR BRANCH: $TRAVIS_PULL_REQUEST_BRANCH"
  FILES=$(git diff --diff-filter=AM --name-only $(git merge-base HEAD ${TRAVIS_BRANCH}) | tr '\n' ' ' )
elif [[ -n  $TRAVIS_COMMIT_RANGE  ]]; then
  echo "TRAVIS COMMIT RANGE: $TRAVIS_COMMIT_RANGE"
  FILES=$(git diff --diff-filter=AM --name-only ${TRAVIS_COMMIT_RANGE/.../..} | tr '\n' ' ' )
fi

for f in $FILES; do
	if ! [ -f "$f" ]; then
		echo "$f was removed." >>/tmp/ctest-important.log
		continue
	fi

	echo "Checking $f" >>/tmp/ctest-important.log
	case "$f" in
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
done

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
