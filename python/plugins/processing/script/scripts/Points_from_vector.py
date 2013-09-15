##[Example scripts]=group
##Input_raster=raster
##Input_vector=vector
##Output_layer=output vector

from qgis.core import *

vector = processing.getobject(Input_vector)
raster = processing.getobject(Input_raster)

geometryType = vector.geometryType()
if geometryType == QGis.Point:
    processing.runalg("qgis:saveselectedfeatures", vector, Output_layer)
elif geometryType == QGis.Line:
    processing.runalg("qgis:pointsfromlines", raster, vector, Output_layer)
elif geometryType == QGis.Polygon:
    processing.runalg("qgis:pointsfrompolygons", raster, vector, Output_layer)
