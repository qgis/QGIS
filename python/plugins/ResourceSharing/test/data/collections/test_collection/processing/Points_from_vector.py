##Points=group
##Input_raster=raster
##Input_vector=vector
##Output_layer=output vector

from qgis.core import *

vector = processing.getObject(Input_vector)
raster = processing.getObject(Input_raster)

geometryType = vector.geometryType()
if geometryType == Qgis.Point:
    processing.runalg('qgis:saveselectedfeatures', vector, Output_layer)
elif geometryType == Qgis.Line:
    processing.runalg('qgis:generatepointspixelcentroidsalongline', raster, vector, Output_layer)
elif geometryType == Qgis.Polygon:
    processing.runalg('qgis:generatepointspixelcentroidsinsidepolygons', raster, vector, Output_layer)
