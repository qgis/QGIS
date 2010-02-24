from PyQt4.QtCore import *
from PyQt4.QtGui import *
import ftools_utils
from qgis.core import *
from qgis.gui import *

from ui_frmReProject import Ui_Dialog
import types

class Dialog(QDialog, Ui_Dialog):
	def __init__(self, iface):
		QDialog.__init__(self)
		self.iface = iface
		self.setupUi(self)
		QObject.connect(self.toolOut, SIGNAL("clicked()"), self.outFile)
		QObject.connect(self.btnProjection, SIGNAL("clicked()"), self.outProjFile)
		QObject.connect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.updateProj1)
		QObject.connect(self.cmbLayer, SIGNAL("currentIndexChanged(QString)"), self.updateProj2)
		self.setWindowTitle( self.tr("Export to new projection") )
		self.progressBar.setValue(0)
		mapCanvas = self.iface.mapCanvas()
		for i in range(mapCanvas.layerCount()):
			layer = mapCanvas.layer(i)
			if layer.type() == layer.VectorLayer:
				self.inShape.addItem(layer.name())
				self.cmbLayer.addItem(layer.name())

	def updateProj1(self, layerName):
		tempLayer = self.getVectorLayerByName(layerName)
		crs = tempLayer.dataProvider().crs().toProj4()
		self.inRef.insert(unicode(crs))

	def updateProj2(self, layerName):
		tempLayer = self.getVectorLayerByName(layerName)
		crs = tempLayer.dataProvider().crs().toProj4()
		self.outRef.insert(unicode(crs))		

	def accept(self):
		if self.inShape.currentText() == "":
			QMessageBox.information(self, self.tr("Export to new projection"), self.tr("No input layer specified"))
		elif self.outShape.text() == "":
			QMessageBox.information(self, self.tr("Export to new projection"), self.tr("Please specify output shapefile"))
		elif self.txtProjection.text() == "" and self.rdoProjection.isChecked():
			QMessageBox.information(self, self.tr("Define current projection"), self.tr("Please specify spatial reference system"))
		elif self.cmbLayer.currentText() == "" and self.rdoLayer.isChecked():
			QMessageBox.information(self, self.tr("Define current projection"), self.tr("Please specify spatial reference system"))
		else:
			inName = self.inShape.currentText()
			self.progressBar.setValue(5)
			outPath = self.outShape.text()
			self.progressBar.setValue(10)
			if self.rdoProjection.isChecked(): outProj = self.txtProjection.text()
			else: outProj = self.cmbLayer.currentText()
			self.progressBar.setValue(15)
			if outPath.contains("\\"): outName = outPath.right((outPath.length() - outPath.lastIndexOf("\\")) - 1)
			else: outName = outPath.right((outPath.length() - outPath.lastIndexOf("/")) - 1)
			if outName.endsWith(".shp"): outName = outName.left(outName.length() - 4)
			if self.reProject(inName, unicode(outPath), unicode(outProj), self.rdoProjection.isChecked(), self.progressBar):
				self.outShape.clear()
				self.progressBar.setValue(100)
				addToTOC = QMessageBox.question(self, self.tr("Export to new projection"), self.tr("Created projected shapefile:\n%1\n\nWould you like to add the new layer to the TOC?").arg( outPath ), QMessageBox.Yes, QMessageBox.No, QMessageBox.NoButton)
				if addToTOC == QMessageBox.Yes:
					self.vlayer = QgsVectorLayer(outPath, unicode(outName), "ogr")
					QgsMapLayerRegistry.instance().addMapLayer(self.vlayer)
		self.progressBar.setValue(0)

	def outProjFile(self):
		format = QString( "<h2>%1</h2>%2 <br/> %3" )
		header = self.tr( "Choose output CRS:" )
		sentence1 = self.tr( "Please select the projection system to be used by the output layer." )
		sentence2 = self.tr( "Output layer will be projected from it's current CRS to the output CRS." )
		self.projSelect = QgsGenericProjectionSelector(self, Qt.Widget)
		self.projSelect.setMessage( format.arg( header ).arg( sentence1 ).arg( sentence2 ))
		if self.projSelect.exec_():
			projString = self.projSelect.selectedProj4String()
			if projString == "":
				QMessageBox.information(self, self.tr("Export to new projection"), self.tr("No Valid CRS selected"))
				return
			else:
				self.txtProjection.clear()
				self.txtProjection.insert(projString)
		else:
			return

	def outFile(self):
		self.outShape.clear()
		( self.shapefileName, self.encoding ) = ftools_utils.saveDialog( self )
		if self.shapefileName is None or self.encoding is None:
			return
		self.outShape.setText( QString( self.shapefileName ) )

	def reProject(self, inName, outPath, outProj, predefined, progressBar):
		vlayer = self.getVectorLayerByName(inName)
		provider = vlayer.dataProvider()
		feat = QgsFeature()
		allAttrs = provider.attributeIndexes()
		progressBar.setValue(2)
		provider.select(allAttrs)
		fieldList = self.getFieldList(vlayer)
		crsDest = QgsCoordinateReferenceSystem()
		if predefined:
			crsDest.createFromProj4(outProj)
		else:
			destLayer = self.getVectorLayerByName(outProj)
			crsDest = destLayer.dataProvider().crs()
		if not crsDest.isValid():
			QMessageBox.information(self, self.tr("Export to new projection"), self.tr("Output spatial reference system is not valid"))
			return False
		else:
			progressBar.setValue(5)
			crsSrc = provider.crs()
			if crsSrc != crsDest:
				xform = QgsCoordinateTransform(crsSrc, crsDest)
				progressBar.setValue(10)
				check = QFile(self.shapefileName)
				if check.exists():
					if not QgsVectorFileWriter.deleteShapeFile(self.shapefileName):
						return
				writer = QgsVectorFileWriter(self.shapefileName, self.encoding, fieldList, provider.geometryType(), crsDest)
				#writer = QgsVectorFileWriter(outPath, "UTF-8", fieldList, provider.geometryType(), crsDest)
				if self.pointReproject(vlayer, xform, writer, progressBar): return True
				else: return False
			else:
				QMessageBox.information(self, self.tr("Export to new projection"), self.tr("Identical output spatial reference system chosen"))
				return False

# Gets vector layer by layername in canvas
	def getVectorLayerByName(self, myName):
		mc = self.iface.mapCanvas()
		nLayers = mc.layerCount()
		for l in range(nLayers):
			layer = mc.layer(l)
			if unicode(layer.name()) == unicode(myName):
				vlayer = QgsVectorLayer(unicode(layer.source()),  unicode(myName),  unicode(layer.dataProvider().name()))
				if vlayer.isValid():
					return vlayer

#Retrieve the field map of a vector Layer
#Return: QgsFieldMap
	def getFieldList(self, vlayer):
		fProvider = vlayer.dataProvider()
		feat = QgsFeature()
		allAttrs = fProvider.attributeIndexes()
		fProvider.select(allAttrs)
		myFields = fProvider.fields()
		return myFields

#Converts all geometries to points, and reprojects using specified QgsCoordinateTransformation   
#Output: Reprojected shapefile with attributes...
	def pointReproject(self, vlayer, xform, writer, progressBar):
		provider = vlayer.dataProvider()
		if provider.featureCount() == 0:
		  return

		allAttrs = provider.attributeIndexes()
		provider.select(allAttrs)
		feat = QgsFeature()
		outfeat = QgsFeature()
		count = 90.00 / provider.featureCount()
		
		while provider.nextFeature(feat):
			geom = feat.geometry()
			multi_geom = QgsGeometry()
			temp_geom = []
			if geom.type() == 0: # it's a point
				if geom.isMultipart():
					multi_geom = geom.asMultiPoint() #multi_geom is a multipoint
					outfeat.setGeometry(QgsGeometry.fromMultiPoint([xform.transform(item) for item in multi_geom]))
				else:
					tpoint = xform.transform(geom.asPoint())
					outfeat.setGeometry(QgsGeometry.fromPoint(tpoint))
					#outfeat.setGeometry(QgsGeometry.fromPoint(xform.transform(geom.asPoint()))
			elif geom.type() == 1: # it's a line
				temp_lines = []
				if geom.isMultipart():
					multi_geom = geom.asMultiPolyline() #multi_geog is a multiline
					outfeat.setGeometry(QgsGeometry.fromMultiPolyline([mappedTransform(xform, item) for item in multi_geom]))
				else:
					multi_geom = geom.asPolyline() #multi_geom is a line
					outfeat.setGeometry(QgsGeometry.fromPolyline([xform.transform(item) for item in multi_geom]))
			elif geom.type() == 2: # it's a polygon
				if geom.isMultipart():
					multi_geom = geom.asMultiPolygon() #multi_geom is a multipolygon
					#outfeat.setGeometry(QgsGeometry.fromMultiPolygon(multi_geom))
					outfeat.setGeometry(QgsGeometry.fromMultiPolygon([[mappedTransform(xform, item) for item in temp] for temp in multi_geom]))
				else:
					multi_geom = geom.asPolygon() #multi_geom is a polygon
					outfeat.setGeometry(QgsGeometry.fromPolygon([mappedTransform(xform, item) for item in multi_geom]))
			else: # don't know what it is...
				return False
			atMap = feat.attributeMap()
			outfeat.setAttributeMap(atMap)
			writer.addFeature(outfeat)
			progressBar.setValue(progressBar.value() + count)
		del writer
		return True

def mappedTransform(xform, inList):
	return [xform.transform(elem) for elem in inList]

