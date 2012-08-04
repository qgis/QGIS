##Additional_image=raster
##Additional_image_2=raster
from sextante.pymorph.mmorph import toggle
img2 = gdal.Open(Additional_image)
input_array2 = img2.ReadAsArray()
img3 = gdal.Open(Additional_image_2)
input_array3 = img2.ReadAsArray()

output_array = toggle(input_array, input_array2, input_array3)

