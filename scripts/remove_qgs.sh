
#!/bin/env bash


DIR=$(git rev-parse --show-toplevel)

# GNU prefix command for mac os support (gsed, gsplit)
GP=
if [[ "$OSTYPE" =~ darwin* ]]; then
  GP=g
fi

rename='QgsVersion:QgisVersion'

read -r -d '' rename << EOM
QgsVersion:QgisVersion
XXX:BBBBB
EOM

find . -type f \( -path './rsrc/*' -or -path './rpython/*' -or -path './tests/*' \) | while read -r f; do
  if [[ ! $f =~ \.(png|pdf|db|dbf|shp|shx|tab)$ ]]; then
    echo $f

    # rename specific classes
    old=$(echo "$rename" | cut -d: -f1)
    new=$(echo "$rename" | cut -d: -f2)
    ${GP}sed -i "s/$old/$new/g; s/${old,,}/${new,,}/g; s/${old^^}/${new^^}/g; " $f
    if [[ $f =~ ${old,,} ]]; then
      # rename specific files
      newfile=$(echo $f | sed "s/${old,,}/${new,,}/")
      mv $f $newfile
      f=$newfile
    fi

    # remove general occurences
    ${GP}sed -i -r 's/Qgs([A-Z])/\1/g' $f
    ${GP}sed -i -r 's/qgs([a-z])/\1/g' $f
    ${GP}sed -i -r 's/QGS([A-Z])/\1/g' $f
  else
    echo "skip $f"
  fi
  # rename files
  if [[ ! $f =~ \.qgs(\.cfg)?$ ]]; then
    mv $f $(echo $f | sed 's/qgs//')
  fi
done
