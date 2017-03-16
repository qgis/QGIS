##Centroids=name
##Geometry=group
##INPUT_LAYER=vector
##OUTPUT_LAYER=output vector

from qgis.core import QgsWkbTypes, QgsGeometry

from processing.tools.vector import VectorWriter

layer = processing.getObject(INPUT_LAYER)
fields = layer.fields()

writer = VectorWriter(OUTPUT_LAYER, 'utf-8', fields, QgsWkbTypes.Point, layer.crs())

features = processing.features(layer)
count = len(features)
if count == 0:
    raise GeoAlgorithmExecutionException('Input layer contains no features.')

total = 100.0 / len(features)

for count, f in enumerate(features):
    outputFeature = f
    if f.geometry():
        outputGeometry = f.geometry().centroid()
        outputFeature.setGeometry(outputGeometry)

    writer.addFeature(outputFeature)
    feedback.setProgress(int(count * total))

del writer
