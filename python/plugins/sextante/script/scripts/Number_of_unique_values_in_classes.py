#Definition of inputs and outputs
#==================================
##[Example scripts]=group
##input=vector
##class_field=field input
##value_field=field input
##output=output vector

#Algorithm body
#==================================
from sextante.core.QGisLayers import QGisLayers
from qgis.core import *
from PyQt4.QtCore import *
from sextante.core.SextanteVectorWriter import SextanteVectorWriter

# "input" contains the location of the selected layer.
# We get the actual object, so we can get its bounds
layer = QGisLayers.getObjectFromUri(input)
provider = layer.dataProvider()
allAttrs = provider.attributeIndexes()
provider.select( allAttrs )
fields = provider.fields()
fields[len(fields)] = QgsField("UNIQ_COUNT", QVariant.Int)
writer = SextanteVectorWriter(output, None, fields, provider.geometryType(), provider.crs() )

# Fields are defined by their names, but QGIS needs the index for the attributes map
class_field_index = provider.fieldNameIndex(class_field)
value_field_index = provider.fieldNameIndex(value_field)

inFeat = QgsFeature()
outFeat = QgsFeature()
inGeom = QgsGeometry()
nFeat = provider.featureCount()
nElement = 0
classes = {}

#Iterate over input layer to count unique values in each class
while provider.nextFeature(inFeat):
  progress.setPercentage(int((100 * nElement)/nFeat))
  nElement += 1
  atMap = inFeat.attributeMap()
  clazz = atMap[class_field_index].toString()
  value = atMap[value_field_index].toString()
  if clazz not in classes:
      classes[clazz] = []
  if value not in classes[clazz]:
      classes[clazz].append(value)

# Create output vector layer with additional attribute
while provider.nextFeature(inFeat):
  print int((500 * nElement)/nFeat)
  nElement += 1
  inGeom = inFeat.geometry()
  outFeat.setGeometry( inGeom )
  atMap = inFeat.attributeMap()
  clazz = atMap[class_field_index].toString()
  outFeat.setAttributeMap( atMap )
  outFeat.addAttribute( len(provider.fields()), QVariant(len(classes[clazz])))
  writer.addFeature( outFeat )

del writer
