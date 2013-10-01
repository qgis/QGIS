##[Example scripts]=group
##input=vector
##class_field=field input
##value_field=field input
##output=output vector

from qgis.core import *
from PyQt4.QtCore import *
from processing.core.VectorWriter import VectorWriter

layer = processing.getObject(input)
provider = layer.dataProvider()
fields = provider.fields()
fields.append(QgsField('UNIQ_COUNT', QVariant.Int))
writer = VectorWriter(output, None, fields, provider.geometryType(),
                      layer.crs())

class_field_index = layer.fieldNameIndex(class_field)
value_field_index = layer.fieldNameIndex(value_field)

inFeat = QgsFeature()
outFeat = QgsFeature()
inGeom = QgsGeometry()
nElement = 0
classes = {}

feats = processing.features(layer)
nFeat = len(feats)
for inFeat in feats:
    progress.setPercentage(int(100 * nElement / nFeat))
    nElement += 1
    attrs = inFeat.attributes()
    clazz = attrs[class_field_index]
    value = attrs[value_field_index]
    if clazz not in classes:
        classes[clazz] = []
    if value not in classes[clazz]:
        classes[clazz].append(value)

feats = processing.features(layer)
nElement = 0
for inFeat in feats:
    progress.setPercentage(int(100 * nElement / nFeat))
    nElement += 1
    inGeom = inFeat.geometry()
    outFeat.setGeometry(inGeom)
    attrs = inFeat.attributes()
    clazz = attrs[class_field_index]
    attrs.append(len(classes[clazz]))
    outFeat.setAttributes(attrs)
    writer.addFeature(outFeat)

del writer
