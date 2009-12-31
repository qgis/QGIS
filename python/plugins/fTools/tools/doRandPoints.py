#-----------------------------------------------------------
# 
# Generate Random Points
#
# A QGIS plugin for generating a simple random points
# shapefile. 
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

from qgis.core import *
from random import *
import math, ftools_utils
from ui_frmRandPoints import Ui_Dialog

class Dialog(QDialog, Ui_Dialog):
    def __init__(self, iface):
        QDialog.__init__(self)
        self.iface = iface
        self.setupUi(self)
        QObject.connect(self.toolOut, SIGNAL("clicked()"), self.outFile)
        QObject.connect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.update)
        self.progressBar.setValue(0)
        self.setWindowTitle(self.tr("Random Points"))
        mapCanvas = self.iface.mapCanvas()
        for i in range(mapCanvas.layerCount()):
            layer = mapCanvas.layer(i)
            if (layer.type() == layer.VectorLayer and layer.geometryType() == QGis.Polygon) or layer.type() == layer.RasterLayer:
            	self.inShape.addItem(layer.name())

# If input layer is changed, update field list            		
    def update(self, inputLayer):
        self.cmbField.clear()
        changedLayer = self.getMapLayerByName(inputLayer)
        if changedLayer.type() == changedLayer.VectorLayer:
        	self.rdoStratified.setEnabled(True)
        	self.rdoDensity.setEnabled(True)
        	self.rdoField.setEnabled(True)
        	self.label_4.setEnabled(True)
        	changedLayer = self.getVectorLayerByName(inputLayer)
        	changedFields = self.getFieldList(changedLayer)
        	for i in changedFields:
        		self.cmbField.addItem(unicode(changedFields[i].name()))
        else:
        	self.rdoUnstratified.setChecked(True)
        	self.rdoStratified.setEnabled(False)
        	self.rdoDensity.setEnabled(False)
        	self.rdoField.setEnabled(False)
        	self.spnStratified.setEnabled(False)
        	self.spnDensity.setEnabled(False)
        	self.cmbField.setEnabled(False)
        	self.label_4.setEnabled(False)

# when 'OK' button is pressed, gather required inputs, and initiate random points generation            
    def accept(self):
	if self.inShape.currentText() == "":
	    QMessageBox.information(self, self.tr("Random Points"), self.tr("No input layer specified"))
	elif self.outShape.text() == "":
	    QMessageBox.information(self, self.tr("Random Points"), self.tr("Please specify output shapefile"))
	else:
		inName = self.inShape.currentText()
		self.progressBar.setValue(1)
		outPath = self.outShape.text()
		self.progressBar.setValue(2.5)
		if outPath.contains("\\"):
			outName = outPath.right((outPath.length() - outPath.lastIndexOf("\\")) - 1)
		else:
			outName = outPath.right((outPath.length() - outPath.lastIndexOf("/")) - 1)
		if outName.endsWith(".shp"):
			outName = outName.left(outName.length() - 4)
		self.progressBar.setValue(5)
		mLayer = self.getMapLayerByName(unicode(inName))
		if mLayer.type() == mLayer.VectorLayer:
			inLayer = QgsVectorLayer(unicode(mLayer.source(),'latin1'),  unicode(mLayer.name(),'latin1'),  unicode(mLayer.dataProvider().name()))
			if self.rdoUnstratified.isChecked():
				design = self.tr("unstratified")
				value = self.spnUnstratified.value()
			elif self.rdoStratified.isChecked():
				design = self.tr("stratified")
				value = self.spnStratified.value()
			elif self.rdoDensity.isChecked():
				design = self.tr("density")
				value = self.spnDensity.value()
			else:
				design = self.tr("field")
				value = unicode(self.cmbField.currentText())
		elif mLayer.type() == mLayer.RasterLayer:
			inLayer = QgsRasterLayer(unicode(mLayer.source(),'latin1'), unicode(mLayer.name(),'latin1'))
			design = self.tr("unstratified")
			value = self.spnUnstratified.value()
		else:
			QMessageBox.information(self, self.tr("Random Points"), self.tr("Unknown layer type..."))
		if self.chkMinimum.isChecked():
			minimum = self.spnMinimum.value()
		else:
			minimum = 0.00
		self.progressBar.setValue(10)
		self.randomize(inLayer, outPath, minimum, design, value, self.progressBar)
		self.progressBar.setValue(100)
		self.outShape.clear()
		addToTOC = QMessageBox.question(self, self.tr("Random Points"), self.tr("Created output point shapefile:\n%1\n\nWould you like to add the new layer to the TOC?").arg(outPath), QMessageBox.Yes, QMessageBox.No, QMessageBox.NoButton)
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
			
# combine all polygons in layer to create single polygon (slow for complex polygons)     
    def createSinglePolygon(self, vlayer, progressBar):
		provider = vlayer.dataProvider()
		allAttrs = provider.attributeIndexes()
		provider.select(allAttrs)
		feat = QgsFeature()
		geom = QgsGeometry()
		geom2 = QgsGeometry()
		provider.nextFeature(feat)
		geom = feat.geometry()
		count = 10.00
		add = 40.00 / provider.featureCount()
		provider.rewind()
		provider.nextFeature(feat)
		geom = QgsGeometry(feat.geometry())
		while provider.nextFeature(feat):
			geom = geom.combine(QgsGeometry( feat.geometry() ))
			count = count + add
			progressBar.setValue(count)
		return geom
	
# Generate list of random points     
    def simpleRandom(self, n, bound, xmin, xmax, ymin, ymax):
		seed()
		points = []
		i = 1
		while i <= n:
			pGeom = QgsGeometry().fromPoint(QgsPoint(xmin + (xmax-xmin) * random(), ymin + (ymax-ymin) * random()))
			if pGeom.intersects(bound):
				points.append(pGeom)
				i = i + 1
		return points
	
# Get vector layer by name from TOC     
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
					QMessageBox.information(self, self.tr("Generate Centroids"), self.tr("Vector layer is not valid"))
	
# Get map layer by name from TOC     
    def getMapLayerByName(self, myName):
    	mc = self.iface.mapCanvas()
    	nLayers = mc.layerCount()
    	for l in range(nLayers):
    		layer = mc.layer(l)
    		if layer.name() == unicode(myName):
    			if layer.isValid():
    				return layer
# Retrieve the field map of a vector Layer
    def getFieldList(self, vlayer):
    	fProvider = vlayer.dataProvider()
    	feat = QgsFeature()
    	allAttrs = fProvider.attributeIndexes()
    	fProvider.select(allAttrs)
    	myFields = fProvider.fields()
    	return myFields
	

    def randomize(self, inLayer, outPath, minimum, design, value, progressBar):
		outFeat = QgsFeature()
		if design == self.tr("unstratified"):
			ext = inLayer.extent()
			if inLayer.type() == inLayer.RasterLayer: bound = ext
			else: bound = self.createSinglePolygon(inLayer, progressBar)
			points = self.simpleRandom(int(value), bound, ext.xMinimum(), ext.xMaximum(), ext.yMinimum(), ext.yMaximum())
			progressBar.setValue(70)
		else: points = self.loopThruPolygons(inLayer, value, design, progressBar)

		fields = { 0 : QgsField("ID", QVariant.Int) }
		check = QFile(self.shapefileName)
		if check.exists():
			if not QgsVectorFileWriter.deleteShapeFile(self.shapefileName):
				return
		writer = QgsVectorFileWriter(self.shapefileName, self.encoding, fields, QGis.WKBPoint, None)
		idVar = 0
		count = 70.00
		add = 30.00 / len(points)
		for i in points:
			outFeat.setGeometry(i)
			outFeat.addAttribute(0, QVariant(idVar))
			writer.addFeature(outFeat)
			idVar = idVar + 1
			count = count + add
			progressBar.setValue(count)
		del writer
	
#   
    def loopThruPolygons(self, inLayer, numRand, design, progressBar):
		sProvider = inLayer.dataProvider()
		sAllAttrs = sProvider.attributeIndexes()
		sProvider.select(sAllAttrs)
		sFeat = QgsFeature()
		sGeom = QgsGeometry()
		sPoints = []
		if design == "field":
			for (i, attr) in sProvider.fields().iteritems():
				if (unicode(numRand) == attr.name()): index = i #get input field index
		count = 10.00
		add = 60.00 / sProvider.featureCount()
		while sProvider.nextFeature(sFeat):
			sGeom = sFeat.geometry()
			if design == self.tr("density"):
				sDistArea = QgsDistanceArea()
				value = int(round(numRand * sDistArea.measure(sGeom)))
			elif design == self.tr("field"):
				sAtMap = sFeat.attributeMap()
				value = sAtMap[index].toInt()[0]
			else:
				value = numRand
			sExt = sGeom.boundingBox()
			sPoints.extend(self.simpleRandom(value, sGeom, sExt.xMinimum(), sExt.xMaximum(), sExt.yMinimum(), sExt.yMaximum()))
			count = count + add
			progressBar.setValue(count)
		return sPoints
