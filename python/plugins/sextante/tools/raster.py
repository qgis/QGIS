from osgeo import gdal
from gdalconst import *
import struct

def scanraster(layer, progress):
    filename = unicode(layer.source())
    dataset = gdal.Open(filename, GA_ReadOnly)
    band = dataset.GetRasterBand(1)
    nodata = band.GetNoDataValue()    
    for y in xrange(band.YSize): 
        progress.setPercentage(y / float(band.YSize) * 100)   
        scanline = band.ReadRaster(0, y, band.XSize, 1,band.XSize, 1, band.DataType)   
        values = struct.unpack('f' * band.XSize, scanline)
        for value in values:
            if value == nodata:
                value = None
            yield value