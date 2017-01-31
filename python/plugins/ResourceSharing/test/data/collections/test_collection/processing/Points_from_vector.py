##Points=group
##Input_raster=raster
##Input_vector=vector
##Output_layer=output vector

from qgis.core import *

vector = processing.getObject(Input_vector)
raster = processing.getObject(Input_raster)

geometryType = vector.geometryType()
if geometryType == QGis.Point:
    processing.runalg('qgis:saveselectedfeatures', vector, Output_layer)
elif geometryType == QGis.Line:
    processing.runalg('qgis:generatepointspixelcentroidsalongline', raster, vector, Output_layer)
elif geometryType == QGis.Polygon:
    processing.runalg('qgis:generatepointspixelcentroidsinsidepolygons', raster, vector, Output_layer)
