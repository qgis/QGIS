#Definition of inputs and outputs
#==================================
##[Example scripts]=group
##input=vector
##class_field=field input
##output=output file

#Algorithm body
#==================================
from sextante.core.QGisLayers import QGisLayers
from qgis.core import *
from PyQt4.QtCore import *
from sextante.core.SextanteVectorWriter import SextanteVectorWriter

# "input" contains the location of the selected layer.
# We get the actual object,
layer = QGisLayers.getObjectFromUri(input)
provider = layer.dataProvider()
allAttrs = provider.attributeIndexes()
provider.select( allAttrs )
fields = provider.fields()
writers = {}

# Fields are defined by their names, but QGIS needs the index for the attributes map
class_field_index = provider.fieldNameIndex(class_field)

inFeat = QgsFeature()
outFeat = QgsFeature()
inGeom = QgsGeometry()
nFeat = provider.featureCount()
nElement = 0
writers = {}

while provider.nextFeature(inFeat):
    progress.setPercentage(int((100 * nElement)/nFeat))
    nElement += 1
    atMap = inFeat.attributeMap()
    clazz = atMap[class_field_index].toString()
    if clazz not in writers:
        outputFile = output + "_" + str(len(writers)) + ".shp"
        writers[clazz] = SextanteVectorWriter(outputFile, None, fields, provider.geometryType(), provider.crs() )
    inGeom = inFeat.geometry()
    outFeat.setGeometry(inGeom)
    outFeat.setAttributeMap(atMap)
    writers[clazz].addFeature(outFeat)


for writer in writers.values():
    del writer