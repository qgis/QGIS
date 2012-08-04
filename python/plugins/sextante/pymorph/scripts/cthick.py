##Conditioning_image=raster
from sextante.pymorph.mmorph import cthick, binary
img2 = gdal.Open(Conditioning_image)
input_array2 = img2.ReadAsArray()
output_array=cthick(binary(input_array), binary(input_array2))