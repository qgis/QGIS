#-----------------------------------------------------------
# 
# Vector Split
#
# A QGIS plugin for creating separate output shapefiles
# based on an attribute from a single input vector layer
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
#---------------------------------------------------------------------

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
#import os, sys, string, math
from ui_frmVectorSplit import Ui_Dialog

class Dialog(QDialog, Ui_Dialog):
	def __init__(self, iface):
		QDialog.__init__(self)
		self.iface = iface
		# Set up the user interface from Designer.
		self.setupUi(self)
		QObject.connect(self.toolOut, SIGNAL("clicked()"), self.outFile)
		QObject.connect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.update)
		self.setWindowTitle("Split vector layer")
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
			QMessageBox.information(self, "Vector Split", "No input shapefile specified")
		elif self.outShape.text() == "":
			QMessageBox.information(self, "Vector Split", "Please specify output shapefile")
		else:
			inField = self.inField.currentText()
			inLayer = self.getVectorLayerByName(unicode(self.inShape.currentText()))
			self.progressBar.setValue(5)
			outPath = QString(self.folderName)
			self.progressBar.setValue(10)
			if outPath.contains("\\"):
				outPath.replace("\\", "/")
			self.progressBar.setValue(15)
			if not outPath.endsWith("/"): outPath = outPath + "/"
			self.split(inLayer, unicode(outPath), unicode(inField), self.progressBar)
			self.progressBar.setValue(100)
			self.outShape.clear()
			QMessageBox.information(self, "Vector Split", "Created output shapefiles in folder:\n" + unicode(outPath))
			self.progressBar.setValue(0)

	def outFile(self):
		fileDialog = QFileDialog()
		settings = QSettings()
		dirName = settings.value("/UI/lastShapefileDir").toString()
		fileDialog.setDirectory(dirName)
		encodingBox = QComboBox()
		l = QLabel("Encoding:",fileDialog)
		fileDialog.layout().addWidget(l)
		fileDialog.layout().addWidget(encodingBox)
		encodingBox.addItem("BIG5") 
		encodingBox.addItem("BIG5-HKSCS")
		encodingBox.addItem("EUCJP")
		encodingBox.addItem("EUCKR")
		encodingBox.addItem("GB2312")
		encodingBox.addItem("GBK") 
		encodingBox.addItem("GB18030")
		encodingBox.addItem("JIS7") 
		encodingBox.addItem("SHIFT-JIS")
		encodingBox.addItem("TSCII")
		encodingBox.addItem("UTF-8")
		encodingBox.addItem("UTF-16")
		encodingBox.addItem("KOI8-R")
		encodingBox.addItem("KOI8-U") 
		encodingBox.addItem("ISO8859-1")
		encodingBox.addItem("ISO8859-2")
		encodingBox.addItem("ISO8859-3")
		encodingBox.addItem("ISO8859-4")
		encodingBox.addItem("ISO8859-5")
		encodingBox.addItem("ISO8859-6")
		encodingBox.addItem("ISO8859-7")
		encodingBox.addItem("ISO8859-8") 
		encodingBox.addItem("ISO8859-8-I")
		encodingBox.addItem("ISO8859-9")
		encodingBox.addItem("ISO8859-10")
		encodingBox.addItem("ISO8859-13")
		encodingBox.addItem("ISO8859-14")
		encodingBox.addItem("ISO8859-15")
		encodingBox.addItem("IBM 850")
		encodingBox.addItem("IBM 866")
		encodingBox.addItem("CP874") 
		encodingBox.addItem("CP1250")
		encodingBox.addItem("CP1251")
		encodingBox.addItem("CP1252")
		encodingBox.addItem("CP1253")
		encodingBox.addItem("CP1254")
		encodingBox.addItem("CP1255")
		encodingBox.addItem("CP1256")
		encodingBox.addItem("CP1257") 
		encodingBox.addItem("CP1258") 
		encodingBox.addItem("Apple Roman")
		encodingBox.addItem("TIS-620")
		encodingBox.setItemText(encodingBox.currentIndex(), QString(QTextCodec.codecForLocale().name()))
		fileDialog.setAcceptMode(QFileDialog.AcceptSave)
		fileDialog.setFileMode(QFileDialog.DirectoryOnly)
		fileDialog.setConfirmOverwrite(False)
		if not fileDialog.exec_() == 1:
			return
		folders = fileDialog.selectedFiles()
		self.folderName = unicode(folders.first())
		self.encoding = unicode(encodingBox.currentText())
		self.outShape.clear()
		self.outShape.insert(self.folderName)

	def split(self, vlayer, outPath, inField, progressBar):
		provider = vlayer.dataProvider()
		#unique = []
		index = provider.fieldNameIndex(inField)
		#provider.uniqueValues(index, unique)
		unique = self.getUniqueValues(vlayer.dataProvider(), int(index))
		baseName = outPath + vlayer.name() + "_" + inField + "_"
		allAttrs = provider.attributeIndexes()
		provider.select(allAttrs)
		fieldList = self.getFieldList(vlayer)
		sRs = provider.crs()
		inFeat = QgsFeature()
		progressBar.setValue(20)
		start = 20.00
		add = 80.00 / len(unique)
		for i in unique:
			check = QFile(baseName + "_" + unicode(i) + ".shp")
			if check.exists():
				if not QgsVectorFileWriter.deleteShapeFile(baseName + "_" + unicode(i.toString().trimmed()) + ".shp"):
					return
			writer = QgsVectorFileWriter(baseName + "_" + unicode(i.toString().trimmed()) + ".shp", self.encoding, fieldList, vlayer.dataProvider().geometryType(), sRs)
			provider.rewind()
			while provider.nextFeature(inFeat):
				atMap = inFeat.attributeMap()
				#changed from QVariant(i) to below:
				if atMap[index] == i:
					writer.addFeature(inFeat)
			del writer
			start = start + add
			progressBar.setValue(start)

	def getVectorLayerByName(self, myName):
		mc = self.iface.mapCanvas()
		nLayers = mc.layerCount()
		for l in range(nLayers):
			layer = mc.layer(l)
			if unicode(layer.name()) == unicode(myName):
				vlayer = QgsVectorLayer(unicode(layer.source()),  unicode(myName),  unicode(layer.dataProvider().name()))
				if vlayer.isValid():
					return vlayer

	def getFieldList(self, vlayer):
		fProvider = vlayer.dataProvider()
		feat = QgsFeature()
		allAttrs = fProvider.attributeIndexes()
		# fetch all attributes for each feature
		fProvider.select(allAttrs)
		# retrieve all fields
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

