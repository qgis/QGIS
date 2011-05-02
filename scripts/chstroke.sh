#!/bin/bash

# Globaly change given value of stroke-width parameter in SVG files
# to a new value

# Parameters: old_width new_width

for F in `ls *.svg`
do
    cp $F svg.tmp
    cat svg.tmp | sed "s/stroke-width:$1/stroke-width:$2/" > $F
done

rm svg.tmp
