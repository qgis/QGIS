##Vector table tools=group
##input=vector
##class_field=field input
##value_field=field input
##N_unique_values=output vector

from qgis.PyQt.QtCore import QVariant
from qgis.core import QgsFeature, QgsField, QgsProcessingUtils

layer = QgsProcessingUtils.mapLayerFromString(input, context)
fields = layer.fields()
fields.append(QgsField('UNIQ_COUNT', QVariant.Int))
writer, writer_dest = QgsProcessingUtils.createFeatureSink(N_unique_values, None, fields, layer.wkbType(), layer.crs(),
                                                           context)

class_field_index = layer.fields().lookupField(class_field)
value_field_index = layer.fields().lookupField(value_field)

outFeat = QgsFeature()
classes = {}
feats = QgsProcessingUtils.getFeatures(layer, context)
nFeat = QgsProcessingUtils.featureCount(layer, context)
for n, inFeat in enumerate(feats):
    feedback.setProgress(int(100 * n / nFeat))
    attrs = inFeat.attributes()
    clazz = attrs[class_field_index]
    value = attrs[value_field_index]
    if clazz not in classes:
        classes[clazz] = []
    if value not in classes[clazz]:
        classes[clazz].append(value)

feats = processing.features(layer)
for n, inFeat in enumerate(feats):
    feedback.setProgress(int(100 * n / nFeat))
    inGeom = inFeat.geometry()
    outFeat.setGeometry(inGeom)
    attrs = inFeat.attributes()
    clazz = attrs[class_field_index]
    attrs.append(len(classes[clazz]))
    outFeat.setAttributes(attrs)
    writer.addFeature(outFeat)

del writer
