#-----------------------------------------------------------
# 
# Points in Polygon
#
# A QGIS plugin for counting the number of input points that
# occur inside each polygon of an input polygon vector layer
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
#---------------------------------------------------------------------

from PyQt4.QtCore import *
from PyQt4.QtGui import *
import ftools_utils
from qgis.core import *
from frmPointsInPolygon import Ui_Dialog

class Dialog(QDialog, Ui_Dialog):

	def __init__(self, iface):
		QDialog.__init__(self)
		self.iface = iface
		# Set up the user interface from Designer.
		self.setupUi(self)
		QObject.connect(self.toolOut, SIGNAL("clicked()"), self.outFile)
		self.setWindowTitle(self.tr("Points in polygon"))
		# populate layer list
		self.progressBar.setValue(0)
		mapCanvas = self.iface.mapCanvas()
		for i in range(mapCanvas.layerCount()):
			layer = mapCanvas.layer(i)
			if layer.type() == layer.VectorLayer:
				if layer.geometryType() == QGis.Polygon:
					self.inPolygon.addItem(layer.name())
				elif layer.geometryType() == QGis.Point:
					self.inPoint.addItem(layer.name())
		
	def accept(self):
		if self.inPolygon.currentText() == "":
			QMessageBox.information(self, "Count Points In Polyon", "Please specify input polygon vector layer")
		elif self.outShape.text() == "":
			QMessageBox.information(self, "Count Points In Polygon", "Please specify output shapefile")
		elif self.inPoint.currentText() == "":
			QMessageBox.information(self, "Count Points In Polygon", "Please specify input point vector layer")
		elif self.lnField.text() == "":
			QMessageBox.information(self, "Count Points In Polygon", "Please specify output count field")
		else:
			inPoly = self.inPolygon.currentText()
			inPts = self.inPoint.currentText()
			inField = self.lnField.text()
			outPath = self.outShape.text()
			if outPath.contains("\\"):
				outName = outPath.right((outPath.length() - outPath.lastIndexOf("\\")) - 1)
			else:
				outName = outPath.right((outPath.length() - outPath.lastIndexOf("/")) - 1)
			if outName.endsWith(".shp"):
				outName = outName.left(outName.length() - 4)
			self.compute(inPoly, inPts, inField, outPath, self.progressBar)
			self.outShape.clear()
			addToTOC = QMessageBox.question(self, "Count Points in Polygons", self.tr("Created output Shapefile:\n") + outPath 
			+ "\n\nWould you like to add the new layer to the TOC?", QMessageBox.Yes, QMessageBox.No, QMessageBox.NoButton)
			if addToTOC == QMessageBox.Yes:
				self.vlayer = QgsVectorLayer(outPath, unicode(outName), "ogr")
				QgsMapLayerRegistry.instance().addMapLayer(self.vlayer)
		self.progressBar.setValue(0)

	def outFile(self):
		self.outShape.clear()
		( self.shapefileName, self.encoding ) = ftools_utils.saveDialog( self )
		if self.shapefileName is None or self.encoding is None:
			return
		self.outShape.setText( QString( self.shapefileName ) )

	def compute(self, inPoly, inPts, inField, outPath, progressBar):
		polyLayer = self.getVectorLayerByName(inPoly)
		pointLayer = self.getVectorLayerByName(inPts)
		polyProvider = polyLayer.dataProvider()
		pointProvider = pointLayer.dataProvider()
		if polyProvider.crs() <> pointProvider.crs():
			QMessageBox.warning(self, "CRS warning!", "Warning: Input layers have non-matching CRS.\nThis may cause unexpected results.")
		allAttrs = polyProvider.attributeIndexes()
		polyProvider.select(allAttrs)
		allAttrs = pointProvider.attributeIndexes()
		pointProvider.select(allAttrs)
		fieldList = self.getFieldList(polyLayer)
		index = polyProvider.fieldNameIndex(unicode(inField))
		if index == -1:
			index = polyProvider.fieldCount()
			field = QgsField(unicode(inField), QVariant.Int, "real", 24, 15, "point count field")
			fieldList[index] = field
		sRs = polyProvider.crs()
		check = QFile(self.shapefileName)
		if check.exists():
			if not QgsVectorFileWriter.deleteShapeFile(self.shapefileName):
				return
		writer = QgsVectorFileWriter(self.shapefileName, self.encoding, fieldList, polyProvider.geometryType(), sRs)
		#writer = QgsVectorFileWriter(outPath, "UTF-8", fieldList, polyProvider.geometryType(), sRs)
		inFeat = QgsFeature()
		outFeat = QgsFeature()
		inGeom = QgsGeometry()
		start = 15.00
		add = 85.00 / polyProvider.featureCount()
		while polyProvider.nextFeature(inFeat):
			inGeom = inFeat.geometry()
			atMap = inFeat.attributeMap()
			outFeat.setAttributeMap(atMap)
			outFeat.setGeometry(inGeom)
			pointList = []
			count = 0
			#(check, pointList) = pointLayer.featuresInRectangle(inGeom.boundingBox(), True, True)
			pointLayer.select(inGeom.boundingBox(), False)
			pointList = pointLayer.selectedFeatures()
			if len(pointList) > 0: check = 0
			else: check = 1
			if check == 0:
				for i in pointList:
					if inGeom.contains(i.geometry().asPoint()):
						count = count + 1
			outFeat.setAttributeMap(atMap)
			outFeat.addAttribute(index, QVariant(count))
			writer.addFeature(outFeat)
			start = start + 1
			progressBar.setValue(start)
		del writer
				
	def getVectorLayerByName(self, myName):
		mc = self.iface.mapCanvas()
		nLayers = mc.layerCount()
		for l in range(nLayers):
			layer = mc.layer(l)
			if layer.name() == unicode(myName):
				vlayer = QgsVectorLayer(unicode(layer.source()),  unicode(myName),  unicode(layer.dataProvider().name()))
				if vlayer.isValid():
					return vlayer
				else:
					QMessageBox.information(self, "Counts Points In Polygons", self.tr("Vector layer is not valid"))

	def getFieldList(self, vlayer):
		fProvider = vlayer.dataProvider()
		feat = QgsFeature()
		allAttrs = fProvider.attributeIndexes()
		fProvider.select(allAttrs)
		myFields = fProvider.fields()
		return myFields
