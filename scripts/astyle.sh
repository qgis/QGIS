#!/bin/bash

#if ! qgsloggermig.pl >/dev/null 2>&1; then
#	echo qgsloggermig.pl not found in path >&2
#	exit 1
#fi

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

#--break-blocks \

export ARTISTIC_STYLE_OPTIONS="\
$ARTISTIC_STYLE_OPTIONS \
--pad=oper \
--pad=paren-in \
--unpad=paren"

for f in "$@"; do
	flip -ub "$f" 
    	#qgsloggermig.pl "$f"
	astyle $ARTISTIC_STYLE_OPTIONS "$f"
done
