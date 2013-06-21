#Definition of inputs and outputs
#==================================
##[Example scripts]=group
##input=vector
##class_field=field input
##output=output file

#Algorithm body
#==================================
from qgis.core import *
from PyQt4.QtCore import *
from sextante.core.SextanteVectorWriter import SextanteVectorWriter

# "input" contains the location of the selected layer.
# We get the actual object,
layer = sextante.getobject(input)
provider = layer.dataProvider()
allAttrs = provider.attributeIndexes()
provider.select( allAttrs )
fields = provider.fields()
writers = {}

# Fields are defined by their names, but QGIS needs the index for the attributes map
class_field_index = layer.fieldNameIndex(class_field)

inFeat = QgsFeature()
outFeat = QgsFeature()
inGeom = QgsGeometry()
nElement = 0
writers = {}

feats = sextante.getfeatures(layer)
nFeat = len(feats)
for inFeat in feats:
    progress.setPercentage(int((100 * nElement)/nFeat))
    nElement += 1
    atMap = inFeat.attributes()
    clazz = atMap[class_field_index]
    if clazz not in writers:
        outputFile = output + "_" + str(len(writers)) + ".shp"
        writers[clazz] = SextanteVectorWriter(outputFile, None, fields, provider.geometryType(), layer.crs() )
    inGeom = inFeat.geometry()
    outFeat.setGeometry(inGeom)
    outFeat.setAttributes(atMap)
    writers[clazz].addFeature(outFeat)


for writer in writers.values():
    del writer