##Vector analysis=group

# inputs


##input=source
##class_field=field input
##value_field=field input
##N_unique_values=sink


from qgis.PyQt.QtCore import QVariant
from qgis.core import QgsFeature, QgsField, QgsProcessingUtils

fields = input.fields()
fields.append(QgsField('UNIQ_COUNT', QVariant.Int))

(sink, N_unique_values) = self.parameterAsSink(parameters, 'N_unique_values', context,
                                               fields, input.wkbType(), input.sourceCrs())


class_field_index = input.fields().lookupField(class_field)
value_field_index = input.fields().lookupField(value_field)

outFeat = QgsFeature()
classes = {}
feats = input.getFeatures()
nFeat = input.featureCount()
for n, inFeat in enumerate(feats):
    if feedback.isCanceled():
        break
    feedback.setProgress(int(100 * n / nFeat))
    attrs = inFeat.attributes()
    clazz = attrs[class_field_index]
    value = attrs[value_field_index]
    if clazz not in classes:
        classes[clazz] = []
    if value not in classes[clazz]:
        classes[clazz].append(value)

feats = input.getFeatures()
for n, inFeat in enumerate(feats):
    if feedback.isCanceled():
        break
    feedback.setProgress(int(100 * n / nFeat))
    inGeom = inFeat.geometry()
    outFeat.setGeometry(inGeom)
    attrs = inFeat.attributes()
    clazz = attrs[class_field_index]
    attrs.append(len(classes[clazz]))
    outFeat.setAttributes(attrs)
    sink.addFeature(outFeat)
