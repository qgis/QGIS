##Additional_image=raster
from sextante.pymorph.mmorph import symdiff
img2 = gdal.Open(Additional_image)
input_array2 = img2.ReadAsArray()
output_array = symdiff(input_array, input_array2)
