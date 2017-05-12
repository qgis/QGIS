#!/usr/bin/env bash

#set -e

# GNU prefix command for mac os support (gsed, gsplit)
GP=
if [[ "$OSTYPE" =~ darwin* ]]; then
  GP=g
fi

annot_sip=("SIP_FACTORY" "SIP_OUT" "SIP_IN" "SIP_INOUT" "SIP_TRANSFER" "SIP_KEEPREFERENCE" "SIP_TRANSFERTHIS" "SIP_TRANSFERBACK" "SIP_RELEASEGIL" "SIP_ARRAY" "SIP_ARRAYSIZE" "SIP_PYNAME\( \1 \)")
annot_head=("Factory" "Out" "In" "InOut" "Transfer" "KeepReference" "TransferThis" "TransferBack" "ReleaseGIL" "Array" "ArraySize" "PyName=(.*)")

DIR=$(git rev-parse --show-toplevel)

pushd ${DIR} > /dev/null

DIFF=sip.diff
>$DIFF

while read -r sipfile; do
    #echo "$sipfile"
    sipfile=$(echo $sipfile | ${GP}sed -r 's/\.\/python\///g')

    if grep -Fxq "$sipfile" python/auto_sip.blacklist; then
        echo "$sipfile   blacklisted"

        # align pointers (?!(\s*(\/\/|\* )))
        # ${GP}sed -i -r 's/^(.*[^\/])([*&]) (\w+.*;)\s*(\/\/.*)?$/\1 \2\3/g' python/$sipfile
        # ${GP}sed -i -r 's/^(.*[^\/])([*&]) (\w+.*;)\s*(\/\/.*)?$/\1 \2\3/g' python/$sipfile
        # ${GP}sed -i -r 's/^(.*[^\/])([*&]) (\w+.*;)\s*(\/\/.*)?$/\1 \2\3/g' python/$sipfile
        # ${GP}sed -i -r 's/^(.*[^\/])([*&]) (\w+.*;)\s*(\/\/.*)?$/\1 \2\3/g' python/$sipfile


        # replace nullptr by 0
        ${GP}sed -i -r 's/ = nullptr/ = 0/g' python/$sipfile

        # replace = "" by QString = ""
        #${GP}sed -i -r 's/ = ""/ = QString()/g' python/$sipfile


        header=$(${GP}sed -E 's/(.*)\.sip/src\/\1.h/' <<< $sipfile)
        echo "$header"

        # continue

        if [ ! -f $header ]; then
            continue
        fi
        if [[ $sipfile =~ core/qgsexception.sip ]]; then
            continue
        fi
        if [[ $sipfile =~ core/qgsdataitem.sip ]]; then
            continue
        fi

        m=$header.copy
        cp $header $m

        for ((i=0;i<${#annot_head[@]};i++)); do
            #echo ${annot_head[$i]}
            while read -r line; do
                echo $line
                line=$(${GP}sed -r 's/^\s+//; s/\s+/ /g; s/\s+$//g;' <<< $line)
                orig_line=$(${GP}sed -r "s@ /${annot_head[$i]}/@@g" <<< $line)
                dest_line=$(${GP}sed -r "s@/${annot_head[$i]}/@${annot_sip[$i]}@g" <<< $line)
                esc_orig_line=$(${GP}sed -r 's/([(){}*+?$^&])/\\\1/g' <<< $orig_line)
                esc_dest_line=$(${GP}sed -r 's/([&])/\\\1/g' <<< $dest_line)
                esc_orig_line=$(${GP}sed -r 's/0/(0|nullptr)/g' <<< $esc_orig_line)
                esc_orig_line=$(${GP}sed -r 's/""/(""|QString\(\))/g' <<< $esc_orig_line)
                esc_dest_line=$(${GP}sed -r 's/(\*\w+) = 0/\1 = nullptr/g' <<< $esc_dest_line)
                echo $esc_orig_line
                echo $esc_dest_line
                #echo "${GP}sed -i -r \"s/$esc_orig_line/$esc_dest_line/\" $header"
                ${GP}sed -i -r "s/$esc_orig_line/$esc_dest_line/" $header
            done < <(egrep "\/${annot_head[$i]}\/" python/$sipfile)
        done

        if ! cmp $header $m >/dev/null 2>&1; then
          if ! grep -xq "#include \"qgis.h\"" $header; then
              gawk -i inplace '{print} /^#include/ && !n {print "#include \"qgis.h\""; n++}' $header
          fi
        fi
        rm $m
    fi
done < <( find . -regex ".*\.sip$" )
rm $DIFF
popd > /dev/null
