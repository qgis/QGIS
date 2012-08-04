##Binary_marker_image=raster
from sextante.pymorph.mmorph import inpos, binary
img2 = gdal.Open(Binary_marker_image)
input_array2 = img2.ReadAsArray()
output_array=inpos(binary(input_array2), input_array)