##Additional_image=raster
from sextante.pymorph.mmorph import suprec
img2 = gdal.Open(Additional_image)
input_array2 = img2.ReadAsArray()
output_array = suprec(input_array, input_array2)
