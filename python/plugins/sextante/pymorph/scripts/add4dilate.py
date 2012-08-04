##Additional_image=raster
from sextante.pymorph.mmorph import add4dilate
img2 = gdal.Open(Aditional_image)
input_array2 = img2.ReadAsArray()
output_array = add4dilate(input_array, input_array2)