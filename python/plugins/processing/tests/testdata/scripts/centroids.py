##Centroids=name
##Geometry=group

#inputs

##INPUT_LAYER=source
##OUTPUT_LAYER=sink point

from qgis.core import QgsWkbTypes, QgsProcessingUtils, QgsProcessingException

fields = INPUT_LAYER.fields()

(sink, OUTPUT_LAYER) = self.parameterAsSink(parameters, 'OUTPUT_LAYER', context,
                                            fields, QgsWkbTypes.Point, INPUT_LAYER.sourceCrs())

features = INPUT_LAYER.getFeatures()
count = INPUT_LAYER.featureCount()
if count == 0:
    raise QgsProcessingException('Input layer contains no features.')

total = 100.0 / count

for count, f in enumerate(features):
    outputFeature = f
    if f.hasGeometry():
        outputGeometry = f.geometry().centroid()
        outputFeature.setGeometry(outputGeometry)

    sink.addFeature(outputFeature)
    feedback.setProgress(int(count * total))
