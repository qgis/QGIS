#******************************************************************************
#  Adapted from gdal_sieve.py, Copyright (c) 2008, Frank Warmerdam
#******************************************************************************
##Analysis=group
##src_filename=raster
##dst_filename=output raster
##connectedness=selection 4;8
from sextante.gdal.GdalUtils import GdalUtils

try:
    from osgeo import gdal, ogr
except ImportError:
    import gdal
    import ogr

threshold = 2
connectedness=int(connectedness)
options = []

src_ds = gdal.Open( src_filename, gdal.GA_ReadOnly )
srcband = src_ds.GetRasterBand(1)
maskband = srcband.GetMaskBand()

drv = gdal.GetDriverByName(GdalUtils.getFormatShortNameFromFilename(dst_filename))
dst_ds = drv.Create( dst_filename,src_ds.RasterXSize, src_ds.RasterYSize,1,
                         srcband.DataType )
wkt = src_ds.GetProjection()
if wkt != '':
    dst_ds.SetProjection( wkt )
dst_ds.SetGeoTransform( src_ds.GetGeoTransform() )
dstband = dst_ds.GetRasterBand(1)

prog_func = gdal.TermProgress

result = gdal.SieveFilter( srcband, maskband, dstband,
                           threshold, connectedness,
                           callback = prog_func )

src_ds = None
dst_ds = None
mask_ds = None






