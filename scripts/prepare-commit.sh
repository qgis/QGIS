#!/bin/bash

PATH=$PATH:$(dirname $0)

if ! type -p astyle.sh >/dev/null; then
	echo astyle.sh not found
	exit 1
fi

if ! type -p colordiff >/dev/null; then
	colordiff()
	{
		cat "$@"
	}
fi

set -e

# determine changed files
MODIFIED=$(svn status | sed -ne "s/^[MA] *//p")

if [ -z "$MODIFIED" ]; then
	echo nothing was modified
	exit 1
fi

# save original changes
REV=$(svn info | sed -ne "s/Revision: //p")
svn diff >r$REV.diff

ASTYLEDIFF=astyle.r$REV.diff
>$ASTYLEDIFF

# reformat
for f in $MODIFIED; do
	case "$f" in
	src/core/spatialite/*)
                echo $f skipped
		continue
		;;

        *.cpp|*.c|*.h|*.cxx|*.hxx|*.c++|*.h++|*.cc|*.hh|*.C|*.H)
                ;;

        *)
                continue
                ;;
        esac

        m=$f.r$REV.prepare

	cp $f $m
	astyle.sh $f
	if diff -u $m $f >>$ASTYLEDIFF; then
		# no difference found
		rm $m
	fi
done

if [ -s "$ASTYLEDIFF" ]; then
	if tty -s; then
		# review astyle changes
		colordiff <$ASTYLEDIFF | less -r
	else
		echo "Files changed (see $ASTYLEDIFF)"
	fi
else
	rm $ASTYLEDIFF
fi
