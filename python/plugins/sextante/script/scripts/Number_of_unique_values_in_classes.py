#Definition of inputs and outputs
#==================================
##[Example scripts]=group
##input=vector
##class_field=field input
##value_field=field input
##output=output vector

#Algorithm body
#==================================
from qgis.core import *
from PyQt4.QtCore import *
from sextante.core.SextanteVectorWriter import SextanteVectorWriter

# "input" contains the location of the selected layer.
# We get the actual object, so we can get its bounds
layer = sextante.getobject(input)
provider = layer.dataProvider()
fields = provider.fields()
fields.append(QgsField("UNIQ_COUNT", QVariant.Int))
writer = SextanteVectorWriter(output, None, fields, provider.geometryType(), layer.crs() )

# Fields are defined by their names, but QGIS needs the index for the attributes map
class_field_index = layer.fieldNameIndex(class_field)
value_field_index = layer.fieldNameIndex(value_field)

inFeat = QgsFeature()
outFeat = QgsFeature()
inGeom = QgsGeometry()
nElement = 0
classes = {}

#Iterate over input layer to count unique values in each class

feats = sextante.getfeatures(layer)
nFeat = len(feats)
for inFeat in feats:
    progress.setPercentage(int((100 * nElement)/nFeat))
    nElement += 1
    attrs = inFeat.attributes()
    clazz = attrs[class_field_index].toString()
    value = attrs[value_field_index].toString()
    if clazz not in classes:
        classes[clazz] = []
    if value not in classes[clazz]:
        classes[clazz].append(value)

# Create output vector layer with additional attribute
feats = sextante.getfeatures(layer)
nElement = 0
for inFeat in feats:
    progress.setPercentage(int((100 * nElement)/nFeat))
    nElement += 1
    inGeom = inFeat.geometry()
    outFeat.setGeometry(inGeom)
    attrs = inFeat.attributes()
    clazz = attrs[class_field_index].toString()
    attrs.append(QVariant(len(classes[clazz])))
    outFeat.setAttributes(attrs)
    writer.addFeature(outFeat)

del writer
