##Centroids=name
##Geometry=group
##INPUT_LAYER=vector
##OUTPUT_LAYER=output vector

from qgis.core import QgsWkbTypes, QgsProcessingUtils

layer = QgsProcessingUtils.mapLayerFromString(INPUT_LAYER, context)
fields = layer.fields()

writer, writer_dest = QgsProcessingUtils.createFeatureSink(OUTPUT_LAYER, 'utf-8', fields, QgsWkbTypes.Point, layer.crs(),
                                                                         context)

features = QgsProcessingUtils.getFeatures(layer, context)
count = QgsProcessingUtils.featureCount(layer, context)
if count == 0:
    raise GeoAlgorithmExecutionException('Input layer contains no features.')

total = 100.0 / count

for count, f in enumerate(features):
    outputFeature = f
    if f.hasGeometry():
        outputGeometry = f.geometry().centroid()
        outputFeature.setGeometry(outputGeometry)

    writer.addFeature(outputFeature)
    feedback.setProgress(int(count * total))
