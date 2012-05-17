##Conditioning_image=raster
##iterations=number 1
from sextante.pymorph.mmorph import cdilate, binary
img2 = gdal.Open(Conditioning_image)
input_array2 = img2.ReadAsArray()
output_array = cdilate(binary(input_array), binary(input_array2), n=iterations)
