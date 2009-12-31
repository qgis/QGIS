from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *
from ui_frmPointsInPolygon import Ui_Dialog

class Dialog(QDialog, Ui_Dialog):

	def __init__(self, iface):
		QDialog.__init__(self)
		self.iface = iface
		# Set up the user interface from Designer.
		self.setupUi(self)

		# populate layer list
		self.progressBar.setValue(0)
		mapCanvas = self.iface.mapCanvas()
		for i in range(mapCanvas.layerCount()):
			layer = mapCanvas.layer(i)
			if layer.type() == layer.VectorLayer:
				self.inPolygon.addItem(layer.name())
				self.inPoint.addItem(layer.name())
		self.updateUI()
		self.cmbModify.addItems([self.tr("creating new selection"), self.tr("adding to current selection"), self.tr("removing from current selection")])

	def updateUI(self):
		self.label_5.setVisible(False)
		self.lnField.setVisible(False)
		self.outShape.setVisible(False)
		self.toolOut.setVisible(False)
		self.label_2.setVisible(False)
		self.setWindowTitle(self.tr("Select by location"))
		self.label_3.setText(self.tr("Select features in:"))
		self.label_4.setText(self.tr("that intersect features in:"))
		self.label_mod = QLabel(self)
		self.label_mod.setObjectName("label_mod")
		self.label_mod.setText(self.tr("Modify current selection by:"))
		self.cmbModify = QComboBox(self)
		self.cmbModify.setObjectName("cmbModify")
		self.gridLayout.addWidget(self.label_mod,2,0,1,1)
		self.gridLayout.addWidget(self.cmbModify,3,0,1,1)
		self.resize(381, 100)

	def accept(self):
		if self.inPolygon.currentText() == "":
			QMessageBox.information(self, self.tr("Select by location"), self.tr( "Please specify input layer"))
		elif self.inPoint.currentText() == "":
			QMessageBox.information(self, self.tr("Select by location"), self.tr("Please specify select layer"))
		else:
			inPoly = self.inPolygon.currentText()
			inPts = self.inPoint.currentText()
			self.compute(inPoly, inPts, self.cmbModify.currentText())
		self.progressBar.setValue(0)

	def compute(self, inPoly, inPts, modify):
		inputLayer = self.getVectorLayerByName(inPoly)
		selectLayer = self.getVectorLayerByName(inPts)
		inputProvider = inputLayer.dataProvider()
		selectProvider = selectLayer.dataProvider()
		feat = QgsFeature()
		geom = QgsGeometry()
		selectedSet = []
		selectProvider.nextFeature(feat)
		geomLayer = QgsGeometry(feat.geometry())
		while selectProvider.nextFeature(feat):
			geomLayer = geomLayer.combine(QgsGeometry(feat.geometry()))
		while inputProvider.nextFeature(feat):
			geom = QgsGeometry(feat.geometry())
			if geom.intersects(geomLayer):
				selectedSet.append(feat.id())
		if modify == self.tr("adding to current selection"):
			selectedSet = list(set(inputLayer.selectedFeaturesIds()).union(selectedSet))
		elif modify == self.tr("removing from current selection"):
			selectedSet = list(set(inputLayer.selectedFeaturesIds()).difference(selectedSet))
		inputLayer.setSelectedFeatures(selectedSet)
				
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
