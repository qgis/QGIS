from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *

from ui_frmRandom import Ui_Dialog
import random
class Dialog(QDialog, Ui_Dialog):
    def __init__(self, iface):
        QDialog.__init__(self)
        self.iface = iface
        # Set up the user interface from Designer.
        self.setupUi(self)
        QObject.connect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.changed)
        self.setWindowTitle("Random selection")
        # populate layer list
        self.progressBar.setValue(0)
        mapCanvas = self.iface.mapCanvas()
        for i in range(mapCanvas.layerCount()):
            layer = mapCanvas.layer(i)
            if layer.type() == layer.VectorLayer:
                self.inShape.addItem(layer.name())
        
    def changed(self, inputLayer):
	changedLayer = self.getVectorLayerByName(inputLayer)
	changedProvider = changedLayer.dataProvider()
	upperVal = changedProvider.featureCount()
	self.spnNumber.setMaximum(upperVal)

    def accept(self):
	if self.inShape.currentText() == "":
	    QMessageBox.information(self, "Random Selection Tool", "No input shapefile specified")
	else:
            self.progressBar.setValue(10)
            inName = self.inShape.currentText()
	    self.progressBar.setValue(20)
    	    layer = self.getVectorLayerByName(inName)
    	    self.progressBar.setValue(30)
	    if self.rdoNumber.isChecked():
		value = self.spnNumber.value()
		self.progressBar.setValue(60)
	    else:
		value = self.spnPercent.value()
		self.progressBar.setValue(50)
		value = int(round((value / 100.0000), 4) * layer.featureCount())
		self.progressBar.setValue(60)
	    selran = random.sample(xrange(0, layer.featureCount()), value)
	    self.progressBar.setValue(70)
	    self.progressBar.setValue(80)
	    self.progressBar.setValue(90)
	    self.progressBar.setValue(100)
	    layer.setSelectedFeatures(selran)
	    self.progressBar.setValue(0)

#Gets vector layer by layername in canvas
#Return: QgsVectorLayer            
    def getVectorLayerByName(self, myName):
 	mc = self.iface.mapCanvas()
 	nLayers = mc.layerCount()
 	for l in range(nLayers):
 	    layer = mc.layer(l)
 	    if layer.name() == unicode(myName):
 	        if layer.isValid():
 	           return layer
