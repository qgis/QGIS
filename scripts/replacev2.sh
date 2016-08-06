#!/bin/bash

set -e

codepaths=$(echo python/{analysis,console,core,custom_widgets,gui,networkanalysis,plugins,pyplugin_installer,server,sip_helpers,testing} src/ tests/)
repl=

s=$(mktemp -t skipped.XXXX.log)
r=$(mktemp -t repl.XXXX.pl)
d=$(mktemp -t apibreak.XXXX.txt)

# Rename classes
for i in $(git --no-pager grep "Qgs[a-zA-Z0-9_]*V2" $codepaths | perl -pe 's#^.*(Qgs[a-zA-Z0-9_]*)V2([a-zA-Z0-9_]*).*$#$1V2$2#' | sort -u)
do
	src=$i
	dst=${src/V2/}
	if git --no-pager grep -l "\<$dst\>" $codepaths; then
		echo "$src vs $dst" >>$s
	else
		echo "  REPLACE $src => $dst"
		echo "s/\b$src\b/$dst/g;" >>$r
		echo "s/\bsipType_$src\b/sipType_$dst/;" >>$r
		echo "<tr><td>$src<td>$dst" >>$d
	fi
done

for i in $(git --no-pager grep "::[a-zA-Z0-9_]*V2" $codepaths | perl -pe 's#^.*::([a-zA-Z0-9_]*)V2([a-zA-Z0-9_]*).*$#$1V2$2#' | egrep -v "^Qgs|SslV2" | sort -u)
do
	src=$i
	dst=${src/V2/}

	if git --no-pager grep -l "\<$dst\>" $codepaths; then
		echo "$src vs $dst" >>$s
	else
		echo "  REPLACE $src => $dst"
		echo "s/\b$src\b/$dst/g;" >>$r
		echo "<tr><td>$src<td>$dst" >>$d
	fi
done

echo "API breaks logged to: $d"
echo "Skipped V2 symbols: $s"
echo "Replacing from $r"
find $codepaths -type f | xargs perl -i -p $r
