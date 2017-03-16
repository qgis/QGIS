#!/bin/env bash

DIR=$(git rev-parse --show-toplevel)

# GNU prefix command for mac os support (gsed, gsplit)
GP=
if [[ "$OSTYPE" =~ darwin* ]]; then
  GP=g
fi

rename='QgsVersion:QgisVersion'

# manual rename
read -r -d '' rename << EOM
QgsVersion:QgisVersion
Qgs25DRendererWidget:Renderer25DWidget
Qgs25DRenderer:Renderer25D
QgsException:QgisException
QgsConfig:QgisConfig
EOM

find . -type f \( -path './src/*' -or -path './python/*' -or -path './tests/*' -or -path './cmake_templates/*' \) | while read -r f; do
  if [[ ! $f =~ \.(api|png|pdf|db|dbf|shp|shx|tab|qgm|tif|sqlite)$ ]]; then
    # skip files which should not have their content changed (such as binary or API files)
    echo $f

    # rename specific classes
    while read -r line; do
      old=$(echo "$line" | cut -d: -f1)
      new=$(echo "$line" | cut -d: -f2)
      ${GP}sed -i "s/$old/$new/g; s/${old,,}/${new,,}/g; s/${old^^}/${new^^}/g; " $f
      if [[ $f =~ ${old,,} ]]; then
        # rename specific files
        newfile=$(echo $f | sed "s/${old,,}/${new,,}/")
        mv $f $newfile
        f=$newfile
      fi
    done <<< "$rename"

    # remove general occurences
    ${GP}sed -i -r 's/Qgs([A-Z])/\1/g' $f
    ${GP}sed -i -r 's/qgs([a-z])/\1/g' $f
    ${GP}sed -i -r 's/QGS([A-Z])/\1/g' $f
  fi

  # rename files
  if ( [[ ! $f =~ \.qgs(\.cfg)?$ ]] && [[ $f =~ qgs ]] ); then
    mv $f $(echo $f | sed 's/qgs//')
  fi
done


# python backward compat api
# V2s are dropped
while read -r line; do
  section=$(echo "$line" | cut -d: -f1)
  new=$(echo "$line" | cut -d: -f2)
  qgs=$(echo "$line" | cut -d: -f3)

  echo "def ${qgs}(*args, **kwargs): from warnings import warn; warn(' ${qgs} is deprecated, use ${new} instead.'); return ${new}(*args, **kwargs)" >> ${DIR}/python/${section}/__init__.py

done < ${DIR}/scripts/remove_qgs_python.dat
