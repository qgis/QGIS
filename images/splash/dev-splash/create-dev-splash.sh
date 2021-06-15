#!/usr/bin/env bash

if [[ "$1" == "--keep-avatars" ]]; then
  keep_avatars=true
else
  keep_avatars=false
fi

# = 12x9-2*9(QGIS blocks)
total_images=90
images_per_block=9
blocks=$((total_images/images_per_block))

font=Source-Sans-Pro-Bold
grey="#303030"
green="#5d9933"

! convert -list font | grep -Fq Source-Sans-Pro-Bold && echo "Font ${font} not found" && exit 1

# get avatars
if [[ "${keep_avatars}" == "true" ]]; then
  echo "skipping avatars"
else
  echo "downloading avatars"
  curl  -H "Accept: application/vnd.github.v3+json" \
    https://api.github.com/repos/qgis/QGIS/contributors?per_page=${total_images} \
    | jq -r "sort_by(.contributions) | reverse | .[] | \"\(.login) \(.avatar_url)\"" \
    >  avatars.dat

  for (( p=0; p<${blocks}; p++ )); do
    odir=avatars_${p}
    mkdir -p ${odir}
    rm -f ${odir}/*.png
  done

  i=0
  while read -r line; do
    login=$(echo "${line}" | cut -d ' ' -f1)
    url=$(echo "${line}" | cut -d ' ' -f2)

    p=$(((i - i%images_per_block)/images_per_block))
    odir=avatars_${p}
    output=${odir}/${i}.png

    if [[ -f alternate-avatars/${login}.png ]]; then
      echo "alternate avatar for ${login}"
      cp alternate-avatars/${login}.png ${output}
    else
      echo "downloading avatar for ${login}"
      wget -q ${url} -O ${odir}/${i}.png
    fi
    ((i=i+1))
  done < avatars.dat
fi

echo "create tiles"
mkdir -p mosaic
rm -f mosaic/*.png
for (( p=0; p<blocks; p++ )); do
  echo "tile $p"
  montage -background '#2e2e2eff' -geometry 200x200+2+2 -tile 3x3 avatars_${p}/*.png mosaic/mosaic$(printf "%02d" ${p})0.png
done

# create QGIS block
echo "create QGIS block"
convert -background "${grey}" -density 1200 -resize 500x500 ../../icons/qgis_icon.svg mosaic/mosaic041.png

# create dev block
echo "create dev block"
convert -size 400x400 xc:${green} \
  -gravity Center \
  -font "${font}" \
  -fill "${grey}" \
  -pointsize 200 \
  -annotate 0 "dev" \
  mosaic/mosaic042.png

echo "compute mosaic"
montage -background "${grey}" -geometry 200x200 -tile 4x3 mosaic/*.png ../splash.png
