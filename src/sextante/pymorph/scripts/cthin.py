##Conditioning_image=raster
from sextante.pymorph.mmorph import cthin, binary
img2 = gdal.Open(Conditioning_image)
input_array2 = img2.ReadAsArray()
output_arrary=cthin(binary(input_array), binary(input_array2))