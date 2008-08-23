#!/bin/bash

if ! astyle.sh >/dev/null 2>&1; then
	echo astyle.sh not found in path >&2
	exit 1
fi

set -e

export elcr="$(tput el)$(tput cr)"

find src -type f -print | while read f; do
	case "$f" in
        *.cpp|*.h|*.c|*.h|*.cxx|*.hxx|*.c++|*.h++|*.cc|*.hh|*.C|*.H)
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

  	echo -ne "Reformating $f$elcr"
	astyle.sh "$f"
done

echo

# convert CRLF to LF
find .. -type f \
  ! -path "*/.svn/*" \
  ! -path "*/win_build/*" \
  ! -name "*.def" \
  ! -name "*.rc" \
  ! -name "*.png" \
  -exec file {} \; |
  grep CRLF |
  cut -d: -f1 |
  while read f; do
  	echo -ne "Flipping $f$elcr"
	flip -ub "$f"
  done
