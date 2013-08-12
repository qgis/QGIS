from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from processing.core.VectorWriter import VectorWriter

#Here we define the input and outputs
#====================================
##[Example scripts]=group
##input=vector
##output=output vector

#And here is the body of the algorithm
#=======================================

#input layers values are always a string with its location.
#That string can be converted into a QGIS object (a QgsVectorLayer in this case))
#using the processing.getobject() method
vectorLayer = processing.getobject(input)

#And now we can process

#First we create the output layer.
#To do so, we create a ProcessingVectorWriter, that we can later use to add features.
provider = vectorLayer.dataProvider()
writer = VectorWriter(output, None, provider.fields(), provider.geometryType(), vectorLayer.crs())

#Now we take the selected features and add them to the output layer
selection = vectorLayer.selectedFeatures()
for feat in selection:
    writer.addFeature(feat)
del writer

#There is nothing more to do here. We do not have to open the layer that we have created.
#The processing framework will take care of that, or will handle it if this algorithm is executed within
#a complex model