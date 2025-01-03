#!/usr/bin/env bash
cd $(git rev-parse --show-toplevel)

export PATH=$PATH:$PWD/scripts

if [ -z "$1" ]; then
	echo "No commit range given. "
	echo "  Usage: ./scripts/verify_indentation [HEAD_REF]..[BASE_REF]"
	exit 0
fi

if ! type -p astyle.sh >/dev/null; then
	echo astyle.sh not found
	exit 1
fi

set -e

ASTYLEDIFF=/tmp/astyle.diff
true > $ASTYLEDIFF


# echo "Commit range: $1"
FILES=$(git diff --diff-filter=AM --name-only $1 | tr '\n' ' ' )

for f in $FILES; do
	if ! [ -f "$f" ]; then
		echo "$f was removed." >>/tmp/ctest-important.log
		continue
	fi

	# echo "Checking $f"
	case "$f" in
	*.cpp|*.c|*.h|*.cxx|*.hxx|*.c++|*.h++|*.cc|*.hh|*.C|*.H|*.sip)
		;;

	*)
		continue
		;;
	esac

	# only run astyle on sipified directories, others are handled by clang-format (see .pre-commit-config.yaml)
	if [[ $f =~ ^src/(core) ]]; then
		m="$f.prepare"
		cp "$f" "$m"
		astyle.sh "$f"
		if diff -u "$m" "$f" >>$ASTYLEDIFF; then
			rm "$m"
		else
			echo "File $f is not styled properly."
		fi
	fi
done

if [ -s "$ASTYLEDIFF" ]; then
	echo
	echo "Required indentation updates:"
	cat "$ASTYLEDIFF"

	cat <<EOF

Tips to prevent and resolve:
* Install astyle to format C++ code
* Install autopep8 (>= 1.2.1) to format python code
* Use "scripts/astyle.sh file" to fix the now incorrectly formatted files
* Consider using scripts/prepare_commit.sh as pre-commit hook to avoid this
  in the future (ln -s ../../scripts/prepare_commit.sh .git/hooks/pre-commit) or
  run it manually before each commit.
EOF

	exit 1
fi
