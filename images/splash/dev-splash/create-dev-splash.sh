#!/usr/bin/env bash

# = 12x9-2*9(QGIS blocks)
total_images=90
images_per_block=9
blocks=$((${total_images}/${images_per_block}))

curl  -H "Accept: application/vnd.github.v3+json" \
  https://api.github.com/repos/qgis/QGIS/contributors?per_page=${total_images} \
  | jq -r "sort_by(.contributions) | reverse | .[:${total_images}] | flatten | map(.avatar_url) | .[]" \
  >  avatars.dat

for (( p=0; p<${blocks}; p++ )); do
  odir=avatars_${p}
  mkdir -p ${odir}
  rm -f ${odir}/*.png
done

i=0
while read -r line; do
  p=$((($i - $i%${images_per_block})/${images_per_block}))
  odir=avatars_${p}
  echo "wget ${line} -O ${odir}/${i}.png"
  wget -q ${line} -O ${odir}/${i}.png
  ((i=i+1))
done < avatars.dat

mkdir -p mosaic
rm -f mosaic/*.png
for (( p=0; p<${blocks}; p++ )); do
  montage -background '#2e2e2eff' -geometry 200x200+2+2 -tile 3x3 avatars_${p}/*.png mosaic/mosaic$(printf "%02d" ${p})0.png
done
cp qgis.png mosaic/mosaic041.png
cp dev.png mosaic/mosaic042.png

montage -background '#2e2e2eff' -geometry 200x200 -tile 4x3 mosaic/*.png ../splash.png
