#!/usr/bin/env bash

set -e

codepaths=$(echo python/{analysis,console,core,custom_widgets,gui,plugins,pyplugin_installer,server,sip_helpers,testing} src/ tests/)

s=$(mktemp -t skipped.XXXX.log)
r=$(mktemp -t repl.XXXX.pl)
d=$(mktemp -t apibreak.XXXX.txt)

# Rename classes
for i in $(git --no-pager grep "Qgs[a-zA-Z0-9_]*V2" "$codepaths" | perl -pe 's#^.*(Qgs[a-zA-Z0-9_]*)V2([a-zA-Z0-9_]*).*$#$1V2$2#' | sort -u)
do
	src=$i
	dst=${src/V2/}
	if git --no-pager grep -l "\<$dst\>" "$codepaths"; then
		echo "$src vs $dst" >>"$s"
	else
		echo "  REPLACE $src => $dst"
		echo "s/\b$src\b/$dst/g;" >>"$r"
		echo "s/\bsipType_$src\b/sipType_$dst/;" >>"$r"
		echo "<tr><td>$src<td>$dst" >>"$d"
	fi
done

for i in $(git --no-pager grep "::[a-zA-Z0-9_]*V2" "$codepaths" | perl -pe 's#^.*::([a-zA-Z0-9_]*)V2([a-zA-Z0-9_]*).*$#$1V2$2#' | grep -v -E "^Qgs|SslV2" | sort -u)
do
	src=$i
	dst=${src/V2/}

	if git --no-pager grep -l "\<$dst\>" "$codepaths"; then
		echo "$src vs $dst" >>"$s"
	else
		echo "  REPLACE $src => $dst"
		echo "s/\b$src\b/$dst/g;" >>"$r"
		echo "<tr><td>$src<td>$dst" >>"$d"
	fi
done

find "$codepaths" \( -name "*v2*.h" -o -name "*v2*.cpp" -o -name "*v2*.sip" \) -type f | while read f; do
	s=${f##*/}
	d=${s/v2/}
	echo "FIND $d"
	if [ $(find "$codepaths" -name "$d" -print | wc -l) -gt 0 ]; then
		echo "$f vs $b" >>"$s"
		continue
	fi

	git mv "$f" "${f/v2/}"

	case "$s" in
	*.sip)
		echo "s#\b$s\b#$d#g;" >>"$r"
		;;

	*)
		echo "s#\b$s\b#$d#g;" >>"$r"
		;;
	esac
done

echo "API breaks logged to: $d"
echo "Skipped V2 symbols: $s"
echo "Replacing from $r"
find "$codepaths" -type f | xargs perl -i -p "$r"
