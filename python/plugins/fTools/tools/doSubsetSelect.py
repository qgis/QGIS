#-----------------------------------------------------------
# 
# Random selection within subsets
#
# A QGIS plugin for randomly selecting features from 
# within multiple user defined subsets based on an input field.
#
# Copyright (C) 2008  Carson Farmer
#
# EMAIL: carson.farmer (at) gmail.com
# WEB  : www.geog.uvic.ca/spar/carson
#
#-----------------------------------------------------------
# 
# licensed under the terms of GNU GPL 2
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
# 
#--------------------------------------------------------------------
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import random
from qgis.core import *
from ui_frmSubsetSelect import Ui_Dialog

class Dialog(QDialog, Ui_Dialog):

	def __init__(self, iface):
		QDialog.__init__(self)
		self.iface = iface
		# Set up the user interface from Designer.
		self.setupUi(self)
		QObject.connect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.update)		
		self.setWindowTitle(self.tr("Random selection within subsets"))
		# populate layer list
		self.progressBar.setValue(0)
		mapCanvas = self.iface.mapCanvas()
		for i in range(mapCanvas.layerCount()):
			layer = mapCanvas.layer(i)
			if layer.type() == layer.VectorLayer:
				self.inShape.addItem(layer.name())

	def update(self, inputLayer):
		self.inField.clear()
		changedLayer = self.getVectorLayerByName(inputLayer)
		changedField = self.getFieldList(changedLayer)
		for i in changedField:
			self.inField.addItem(unicode(changedField[i].name()))

	def accept(self):
		if self.inShape.currentText() == "":
			QMessageBox.information(self, self.tr("Random selection within subsets"), self.tr("Please specify input vector layer"))
		elif self.inField.currentText() == "":
			QMessageBox.information(self, self.tr("Random selection within subsets"), self.tr("Please specify an input field"))
		else:
			inVect = self.inShape.currentText()
			uidField = self.inField.currentText()
			if self.rdoNumber.isChecked():
				value = self.spnNumber.value()
				perc = False
			else:
				value = self.spnPercent.value()
				perc = True
			self.compute(inVect, uidField, value, perc, self.progressBar)
			self.progressBar.setValue(100)
		self.progressBar.setValue(0)

	def outFile(self):
		self.outShape.clear()
		fileDialog = QFileDialog()
		fileDialog.setConfirmOverwrite(False)
		outName = fileDialog.getSaveFileName(self, self.tr("Output Shapefile"),".", self.tr("Shapefiles (*.shp)"))
		fileCheck = QFile(outName)
		if fileCheck.exists():
			QMessageBox.warning(self, self.tr("Random selection within subsets"), self.tr("Cannot overwrite existing shapefile..."))
		else:
			filePath = QFileInfo(outName).absoluteFilePath()
			if filePath.right(4) != ".shp": filePath = filePath + ".shp"
			if not outName.isEmpty():
				self.outShape.clear()
				self.outShape.insert(filePath)

	def compute(self, inVect, inField, value, perc, progressBar):
		vlayer = self.getVectorLayerByName(inVect)
		vprovider = vlayer.dataProvider()
		mlayer = self.getMapLayerByName(inVect)
		allAttrs = vprovider.attributeIndexes()
		vprovider.select(allAttrs)
		index = vprovider.fieldNameIndex(inField)
		#unique = []
		#vprovider.uniqueValues(index, unique)
		unique = self.getUniqueValues(vprovider, int(index))
		inFeat = QgsFeature()
		selran = []
		mlayer.removeSelection(True)
		nFeat = vprovider.featureCount() * len(unique)
		nElement = 0
		self.progressBar.setValue(0)
		self.progressBar.setRange(0, nFeat)
		if not len(unique) == mlayer.featureCount():
			for i in unique:
				vprovider.rewind()
				FIDs= []
				while vprovider.nextFeature(inFeat):
					atMap = inFeat.attributeMap()
					if atMap[index] == QVariant(i):
						FID = inFeat.id()
						FIDs.append(FID)
					nElement += 1
					self.progressBar.setValue(nElement)
				if perc: selVal = int(round((value / 100.0000) * len(FIDs), 0))
				else: selVal = value
				if selVal >= len(FIDs): selran = FIDs
				else: selran = random.sample(FIDs, selVal)
				selran.extend(mlayer.selectedFeaturesIds())
				mlayer.setSelectedFeatures(selran)
		else:
			mlayer.setSelectedFeatures(range(0, mlayer.featureCount()))

	def getVectorLayerByName(self, myName):
		mc = self.iface.mapCanvas()
		nLayers = mc.layerCount()
		for l in range(nLayers):
			layer = mc.layer(l)
			if layer.name() == unicode(myName,'latin1'):
				vlayer = QgsVectorLayer(unicode(layer.source()),  unicode(myName),  unicode(layer.dataProvider().name()))
				if vlayer.isValid():
					return vlayer
				else:
					QMessageBox.information(self, self.tr("Random selection within subsets"), self.tr("Vector layer is not valid"))

	def getMapLayerByName(self, myName):
		mc = self.iface.mapCanvas()
		nLayers = mc.layerCount()
		for l in range(nLayers):
			layer = mc.layer(l)
			if layer.name() == unicode(myName,'latin1'):
				return layer

	def getFieldList(self, vlayer):
		fProvider = vlayer.dataProvider()
		feat = QgsFeature()
		allAttrs = fProvider.attributeIndexes()
		fProvider.select(allAttrs)
		myFields = fProvider.fields()
		return myFields

	def getUniqueValues(self, provider, index):
		allAttrs = provider.attributeIndexes()    
		provider.select(allAttrs)
		feat = QgsFeature()
		values = []
		check = []
		provider.rewind()
		while provider.nextFeature(feat):
			if not feat.attributeMap()[index].toString() in check:
				values.append( feat.attributeMap()[index] )
				check.append( feat.attributeMap()[index].toString() )
		return values
