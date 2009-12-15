#!/bin/bash

PATH=$PATH:$(dirname $0)

set -e

export elcr="$(tput el)$(tput cr)"

find src -type f -print | while read f; do
	case "$f" in
	src/core/spatialite/*)
		continue
		;;


        *.cpp|*.h|*.c|*.h|*.cxx|*.hxx|*.c++|*.h++|*.cc|*.hh|*.C|*.H)
                cmd=astyle.sh
                ;;

	*.ui|*.qgm|*.txt|*.t2t|*.py|*.sip|resources/context_help/*)
		cmd="flip -ub"
		;;


        *)
                continue
                ;;
        esac

        if [ -f "$f.astyle" ]; then
		# reformat backup
                cp "$f.astyle" "$f"
                touch -r "$f.astyle" "$f"
        else
		# make backup
                cp "$f" "$f.astyle"
                touch -r "$f" "$f.astyle"
        fi

  	echo -ne "Reformating $f $elcr"
	$cmd "$f"
done

echo
