##Markers_image=raster
from sextante.pymorph.mmorph import cwatershed
img2 = gdal.Open(Markers_image)
input_array2 = img2.ReadAsArray()
output_arrary=cwatershed(input_array, input_array2)