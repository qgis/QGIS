##Conditioning_image=raster
##iterations=number 1
from sextante.pymorph.mmorph import cerode
img2 = gdal.Open(Conditioning_image)
input_array2 = img2.ReadAsArray()
output_array = cerode(input_array, input_array2, n=iterations)
