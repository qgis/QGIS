##[Example scripts]aster processing=group
##input=raster
##round_values_to_ndigits=number 3
##output_file=output html

from osgeo import gdal
import sys
import math

# load raster
gdalData = gdal.Open(str(input))

# get width and heights of the raster
xsize = gdalData.RasterXSize
ysize = gdalData.RasterYSize

# get number of bands
bands = gdalData.RasterCount

# start writing html output
f = open(output_file, 'a')
f.write('<TABLE>\n<TH>Band Number  </TH> <TH>Cell Value  </TH> <TH>Count</TH>\n')

# process the raster
for i in xrange(1, bands + 1):
  band_i = gdalData.GetRasterBand(i)
  raster = band_i.ReadAsArray()

  # create dictionary for unique values count
  count = {}

  # count unique values for the given band
  for col in range( xsize ):
    for row in range( ysize ):
      cell_value = raster[row, col]

      # check if cell_value is NaN
      if math.isnan(cell_value):
        cell_value = 'Null'

      # round floats if needed
      elif round_values_to_ndigits:
        try:
          cell_value = round(cell_value, int(round_values_to_ndigits))
        except:
          cell_value = round(cell_value)

      # add cell_value to dictionary
      try:
        count[cell_value] += 1
      except:
        count[cell_value] = 1

  # print results sorted by cell_value
  for key in sorted(count.iterkeys()):
    line = "<TD>%s</TD> <TD>%s</TD> <TD>%s</TD>" %(i, key, count[key])
    f.write('<TR>'+ line + '</TR>' + '\n')

f.write('</TABLE>')
f.close

