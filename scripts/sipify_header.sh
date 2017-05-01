#!/usr/bin/env bash

set -e

annot_sip=("SIP_FACTORY" "SIP_OUT" "SIP_IN" "SIP_INOUT" "SIP_TRANSFER" "SIP_KEEPREFERENCE" "SIP_TRANSFERTHIS" "SIP_TRANSFERBACK" "SIP_RELEASEGIL" "SIP_ARRAY" "SIP_ARRAYSIZE" "SIP_PYNAME\(\1\)")
annot_head=("Factory" "Out" "In" "InOut" "Transfer" "KeepReference" "TransferThis" "TransferBack" "ReleaseGIL" "Array" "ArraySize" "PyName=(.*)")

DIR=$(git rev-parse --show-toplevel)

pushd ${DIR} > /dev/null

DIFF=sip.diff
>$DIFF

while read -r sipfile; do
    #echo "$sipfile"
    sipfile=$(echo $sipfile | gsed -r 's/\.\/python\///g')

    if grep -Fxq "$sipfile" python/auto_sip.blacklist; then
        echo "$sipfile   blacklisted"

        # align pointers (?!(\s*(\/\/|\* )))
        # gsed -i -r 's/^(.*[^\/])([*&]) (\w+.*;)\s*(\/\/.*)?$/\1 \2\3/g' python/$sipfile
        # gsed -i -r 's/^(.*[^\/])([*&]) (\w+.*;)\s*(\/\/.*)?$/\1 \2\3/g' python/$sipfile
        # gsed -i -r 's/^(.*[^\/])([*&]) (\w+.*;)\s*(\/\/.*)?$/\1 \2\3/g' python/$sipfile
        # gsed -i -r 's/^(.*[^\/])([*&]) (\w+.*;)\s*(\/\/.*)?$/\1 \2\3/g' python/$sipfile
        # gsed -i -r 's/^(.*[^\/])([*&]) (\w+.*;)\s*(\/\/.*)?$/\1 \2\3/g' python/$sipfile
        # gsed -i -r 's/^(.*[^\/])([*&]) (\w+.*;)\s*(\/\/.*)?$/\1 \2\3/g' python/$sipfile
        # gsed -i -r 's/^(.*[^\/])([*&]) (\w+.*;)\s*(\/\/.*)?$/\1 \2\3/g' python/$sipfile
        #

        header=$(sed -E 's/(.*)\.sip/src\/\1.h/' <<< $sipfile)
        echo "$header"

        #continue

        if [ ! -f $header ]; then
            continue
        fi
        if [[ $sipfile =~ core/qgsexception.sip ]]; then
            continue
        fi
        if [[ $sipfile =~ core/qgsdataitem.sip ]]; then
            continue
        fi


        for ((i=0;i<${#annot_head[@]};i++)); do
            #echo ${annot_head[$i]}
            while read -r line; do

                echo $line

                line=$(gsed -r 's/^\s+//; s/\s+/ /g; s/\s+$//g;' <<< $line)
                orig_line=$(gsed -r "s@ /${annot_head[$i]}/@@g" <<< $line)
                dest_line=$(gsed -r "s@/${annot_head[$i]}/@${annot_sip[$i]}@g" <<< $line)
                echo $line
                esc_orig_line=$(gsed -r 's/([(){}*+?$^&])/\\\1/g' <<< $orig_line)
                esc_orig_line=$(gsed -r 's/0/(0|nullptr)/g' <<< $esc_orig_line)
                esc_dest_line=$(gsed -r 's/([&])/\\\1/g' <<< $dest_line)
                echo $esc_orig_line
                echo $esc_dest_line

                echo "gsed -i -r \"s/$esc_orig_line/$esc_dest_line/\" $header"

                m=$header.copy
                cp $header $m

                gsed -i -r "s/$esc_orig_line/$esc_dest_line/" $header

                if diff -u $m $header >>$DIFF ; then
                    rm $m
                    echo "could not replace"
                    rm $DIFF
                    popd > /dev/null
                    exit 1
                fi
                rm $m
            done < <(egrep "\/${annot_head[$i]}\/" python/$sipfile)
        done
        #echo $header;



        #gsed ''
    fi
done < <( find . -regex ".*\.sip$" )
rm $DIFF
popd > /dev/null
