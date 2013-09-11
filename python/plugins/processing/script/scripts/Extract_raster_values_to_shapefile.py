##[Example scripts]=group
##Input_raster=raster
##Input_vector=vector
##Transform_vector_to_raster_CRS=boolean
##Output_layer=output vector

import os
from osgeo import gdal, ogr, osr
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.tools.raster import mapToPixel

raster = gdal.Open(Input_raster)

rasterBaseName = os.path.splitext(os.path.basename(Input_raster))[0]

bandCount = raster.RasterCount
rasterXSize = raster.RasterXSize
rasterYSize = raster.RasterYSize
geoTransform = raster.GetGeoTransform()
rasterCRS = osr.SpatialReference()
rasterCRS.ImportFromWkt(raster.GetProjectionRef())

vector = ogr.Open(Input_vector, False)
layer = vector.GetLayer(0)
featureCount = layer.GetFeatureCount()
if featureCount == 0:
    raise GeoAlgorithmExecutionException("There are no features in input vector.")

vectorCRS = layer.GetSpatialRef()

drv = ogr.GetDriverByName("ESRI Shapefile")
if drv is None:
    raise GeoAlgorithmExecutionException("'ESRI Shapefile' driver is not available.")

outputDataset = drv.CreateDataSource(Output_layer)
if outputDataset is None:
    raise GeoAlgorithmExecutionException("Creation of output file failed.")

outputLayer = outputDataset.CreateLayer(str(os.path.splitext(os.path.basename(Output_layer))[0]), vectorCRS, ogr.wkbPoint)
if outputLayer is None:
    raise GeoAlgorithmExecutionException("Layer creation failed.")

featureDefn = layer.GetLayerDefn()
for i in xrange(featureDefn.GetFieldCount()):
    fieldDefn = featureDefn.GetFieldDefn(i)
    if outputLayer.CreateField(fieldDefn) != 0:
        raise GeoAlgorithmExecutionException("Can't create field '%s'." % fieldDefn.GetNameRef())

columnName = str(rasterBaseName[:8])
for i in xrange(bandCount):
    fieldDefn = ogr.FieldDefn(columnName + "_" + str(i + 1), ogr.OFTReal)
    fieldDefn.SetWidth(18)
    fieldDefn.SetPrecision(8)
    if outputLayer.CreateField(fieldDefn) != 0:
        raise GeoAlgorithmExecutionException("Can't create field '%s'." % fieldDefn.GetNameRef())

outputFeature = ogr.Feature(outputLayer.GetLayerDefn())

current = 0
total = bandCount + featureCount * bandCount + featureCount

layer.ResetReading()
feature = layer.GetNextFeature()
while feature is not None:
    current += 1
    progress.setPercentage(int(current * total))

    outputFeature.SetFrom(feature)
    if outputLayer.CreateFeature(outputFeature) != 0:
        raise GeoAlgorithmExecutionException("Failed to add feature.")
    feature = layer.GetNextFeature()

vector.Destroy()
outputFeature.Destroy()
outputDataset.Destroy()

vector = ogr.Open(Output_layer, True)
layer = vector.GetLayer(0)

if Transform_vector_to_raster_CRS:
    coordTransform = osr.CoordinateTransformation(vectorCRS, rasterCRS)
    if coordTransform is None:
        raise GeoAlgorithmExecutionException("Error while creating coordinate transformation.")

for i in xrange(bandCount):
    current += 1
    progress.setPercentage(int(current * total))

    rasterBand = raster.GetRasterBand(i + 1)
    data = rasterBand.ReadAsArray()
    layer.ResetReading()
    feature = layer.GetNextFeature()
    while feature is not None:
        current += 1
        progress.setPercentage(int(current * total))

        geometry = feature.GetGeometryRef()
        x = geometry.GetX()
        y = geometry.GetY()
        if Transform_vector_to_raster_CRS:
            pnt = coordTransform.TransformPoint(x, y, 0)
            x = pnt[0]
            y = pnt[1]
        rX, rY = mapToPixel(x, y, geoTransform)
        if rX > rasterXSize or rY > rasterYSize:
            feature = layer.GetNextFeature()
            continue
        value = data[rY, rX]

        feature.SetField(columnName + '_' + str(i + 1), float(value))
        if layer.SetFeature(feature) != 0:
            raise GeoAlgorithmExecutionException("Failed to update feature.")

        feature = layer.GetNextFeature()

    rasterBand = None

raster = None
vector.Destroy()
