#!/bin/bash

if ! type -p astyle >/dev/null; then
	echo "astyle not found" >&2
	exit 1	
fi

if ! type -p flip >/dev/null; then
	flip() {
		:
	}
fi

set -e

export ARTISTIC_STYLE_OPTIONS="\
--preserve-date \
--indent-preprocessor \
--brackets=break \
--convert-tabs \
--indent=spaces=2 \
--indent-classes \
--indent-labels \
--indent-namespaces \
--indent-switches \
--one-line=keep-blocks \
--one-line=keep-statements \
--max-instatement-indent=40 \
--min-conditional-indent=-1 \
--suffix=none"

export ARTISTIC_STYLE_OPTIONS="\
$ARTISTIC_STYLE_OPTIONS \
--pad=oper \
--pad=paren-in \
--unpad=paren"

for f in "$@"; do
	if ! [ -f "$f" ]; then
		echo "$f not found" >&2
		continue
        fi

	flip -ub "$f" 
	#qgsloggermig.pl "$f"
	astyle $ARTISTIC_STYLE_OPTIONS "$f"
done
