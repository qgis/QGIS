#!/usr/bin/env bash

if [[ "$1" == "--keep-avatars" ]]; then
  keep_avatars=true
else
  keep_avatars=false
fi

# = 12x9-2*9(QGIS blocks)
tile_side=3
avatars_per_tile=$((tile_side*tile_side))
mosaic_width_tiles=6
mosaic_height_tiles=3
number_of_tiles=$((mosaic_width_tiles*mosaic_height_tiles-2))
number_of_avatars=$((number_of_tiles*avatars_per_tile))

font=Source-Sans-Pro-Bold
grey="#303030"
green="#5d9933"

! convert -list font | grep -Fq Source-Sans-Pro-Bold && echo "Font ${font} not found" && exit 1

# get avatars
if [[ "${keep_avatars}" == "true" ]]; then
  echo "skipping avatars"
else
  rm -f avatars.dat || true
  # touch avatars.dat
  for p in 1 2; do
    echo "downloading avatars page $p"
    curl -H "Accept: application/vnd.github.v3+json" \
      "https://api.github.com/repos/qgis/QGIS/contributors?page=${p}&per_page=$((number_of_avatars/2))" \
      | jq -r "sort_by(.contributions) | reverse | .[] | \"\(.login) \(.avatar_url)\""  >>  avatars.dat
  done

  for (( t=0; t<number_of_tiles; t++ )); do
    odir=avatars_${t}
    mkdir -p ${odir}
    rm -f ${odir}/*.png
  done

  i=0
  while read -r line; do
    login=$(echo "${line}" | cut -d ' ' -f1)
    url=$(echo "${line}" | cut -d ' ' -f2)

    t=$(((i - i%avatars_per_tile)/avatars_per_tile))
    odir=avatars_${t}
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
for (( t=0; t<number_of_tiles; t++ )); do
  echo "tile $t"
  montage -background "${grey}" -geometry 200x200+${tile_side-1}+${tile_side-1} -tile ${tile_side}x${tile_side} avatars_${t}/*.png mosaic/mosaic$(printf "%02d" ${t})0.png
done

# create QGIS block
echo "create QGIS block"
convert -background "${grey}" -density 1200 -resize 320x320 -gravity Center -extent 400x400 ../../icons/qgis_icon.svg mosaic/mosaic071.png

# create dev block
echo "create dev block"
convert -size 400x400 xc:${green} \
  -gravity Center \
  -font "${font}" \
  -fill "${grey}" \
  -pointsize 200 \
  -annotate 0 "dev" \
  mosaic/mosaic072.png

echo "compute mosaic"
tile_size=400
montage -background "${grey}" -geometry ${tile_size}x${tile_size} -tile ${mosaic_width_tiles}x${mosaic_height_tiles} mosaic/*.png mosaic.png
mosaic_width=$((tile_size*mosaic_width_tiles))
mosaic_height=$((tile_size*mosaic_height_tiles+tile_size/tile_side))
convert mosaic.png -resize ${mosaic_width}x${mosaic_height} -background "${grey}" -gravity North -extent ${mosaic_width}x${mosaic_height} ../splash.png
