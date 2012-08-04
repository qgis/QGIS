#******************************************************************************
#  Adapted from gdal_proximity.py, Copyright (c) 2008, Frank Warmerdam
#******************************************************************************
##Analysis=group
##src_filename=raster
##dst_filename=output raster
##values=string
##maxdist=number 0
##nodata=number 0
##distunits=selection PIXEL;GEO
##fixed_buf_val=number 0
from sextante.gdal.GdalUtils import GdalUtils
try:
    from osgeo import gdal
except ImportError:
    import gdal

creation_options = []
options = []
src_band_n = 1
dst_band_n = 1
creation_type = 'Float32'

gdal.AllRegister()
options.append( 'MAXDIST=' + str(maxdist))
options.append( 'VALUES=' + values )
options.append( 'DISTUNITS=' + str(distunits))
options.append( 'NODATA=' + str(nodata))
options.append( 'FIXED_BUF_VAL=' +str(fixed_buf_val))

src_ds = gdal.Open( src_filename )
srcband = src_ds.GetRasterBand(src_band_n)

# =============================================================================
#       Try opening the destination file as an existing file.
# =============================================================================

try:
    driver = gdal.IdentifyDriver( dst_filename )
    if driver is not None:
        dst_ds = gdal.Open( dst_filename, gdal.GA_Update )
        dstband = dst_ds.GetRasterBand(dst_band_n)
    else:
        dst_ds = None
except:
    dst_ds = None

# =============================================================================
#     Create output file.
# =============================================================================
if dst_ds is None:
    drv = gdal.GetDriverByName(GdalUtils.getFormatShortNameFromFilename(dst_filename))
    dst_ds = drv.Create( dst_filename,
                         src_ds.RasterXSize, src_ds.RasterYSize, 1,
                         gdal.GetDataTypeByName(creation_type) )

    dst_ds.SetGeoTransform( src_ds.GetGeoTransform() )
    dst_ds.SetProjection( src_ds.GetProjectionRef() )

    dstband = dst_ds.GetRasterBand(1)

# =============================================================================
#    Invoke algorithm.
# =============================================================================

prog_func = gdal.TermProgress
gdal.ComputeProximity( srcband, dstband, options,
                       callback = prog_func )

srcband = None
dstband = None
src_ds = None
dst_ds = None





