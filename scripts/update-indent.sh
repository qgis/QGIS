#!/bin/bash

if ! [ -x astyle.sh ]; then
	PATH=$PATH:$(dirname $0)
fi

set -e

# determine last commit
REV0=$(svn info | sed -ne "s/Revision: //p")

# update
MODIFIED=$(svn update | sed -ne "s/^[^ ]* *//p")
REV1=$(svn info | sed -ne "s/Revision: //p")

if [ "$REV0" -eq "$REV1" ]; then
	echo "No activity since last run."
	exit 0
fi

echo "Checking changes between $REV0 and $REV1"

ASTYLEDIFF=astyle.r$REV0-r$REV1.diff
>$ASTYLEDIFF


# reformat
for f in $MODIFIED; do
	case "$f" in
        *.cpp|*.h|*.c|*.h|*.cxx|*.hxx|*.c++|*.h++|*.cc|*.hh|*.C|*.H)
                ;;

        *)
                continue
                ;;
        esac

        m=$f.r$REV1.prepare

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

	# just echo for now
	echo svn commit -m "automatic indentation update (r$REV0-r$REV1)"
else
	echo "No indentation updates."
	rm $ASTYLEDIFF
fi
