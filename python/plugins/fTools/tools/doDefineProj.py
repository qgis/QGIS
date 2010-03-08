# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *
from qgis.gui import *
import ftools_utils
from ui_frmReProject import Ui_Dialog

class Dialog(QDialog, Ui_Dialog):
    def __init__(self, iface):
        QDialog.__init__(self)
        self.iface = iface
        self.setupUi(self)
        self.toolOut.setEnabled(False)
        self.toolOut.setVisible(False)
        self.outShape.setEnabled(False)
        self.outShape.setVisible(False)
        self.label_2.setVisible(False)
        self.label_2.setEnabled(False)
        self.setWindowTitle(self.tr("Define current projection"))
        QObject.connect(self.btnProjection, SIGNAL("clicked()"), self.outProjFile)
        QObject.connect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.updateProj1)
        QObject.connect(self.cmbLayer, SIGNAL("currentIndexChanged(QString)"), self.updateProj2)
        # populate layer list
        self.progressBar.setValue(0)
        mapCanvas = self.iface.mapCanvas()
        layers = ftools_utils.getLayerNames([QGis.Point, QGis.Line, QGis.Polygon])
        self.inShape.addItems(layers)
        self.cmbLayer.addItems(layers)

    def updateProj1(self, layerName):
        tempLayer = ftools_utils.getVectorLayerByName(layerName)
        crs = tempLayer.dataProvider().crs().toProj4()
        self.inRef.insert(unicode(crs))

    def updateProj2(self, layerName):
        tempLayer = ftools_utils.getVectorLayerByName(layerName)
        crs = tempLayer.dataProvider().crs().toProj4()
        self.outRef.insert(unicode(crs))

    def accept(self):
        if self.inShape.currentText() == "":
            QMessageBox.information(self, self.tr("Define current projection"), self.tr("No input shapefile specified"))
        elif self.txtProjection.text() == "" and self.rdoProjection.isChecked():
            QMessageBox.information(self, self.tr("Define current projection"), self.tr("Please specify spatial reference system"))
        elif self.cmbLayer.currentText() == "" and self.rdoLayer.isChecked():
            QMessageBox.information(self, self.tr("Define current projection"), self.tr("Please specify spatial reference system"))
        else:
            self.progressBar.setValue(5)
            inName = self.inShape.currentText()
            self.progressBar.setValue(10)
            vLayer = ftools_utils.getVectorLayerByName(inName)
            self.progressBar.setValue(30)
            if vLayer == "Error":
                QMessageBox.information(self, self.tr("Define current projection"), self.tr("Cannot define projection for PostGIS data...yet!"))
            else:
                srsDefine = QgsCoordinateReferenceSystem()
                if self.rdoProjection.isChecked():
                    outProj = self.txtProjection.text()
                    srsDefine.createFromProj4(outProj)
                else:
                    destLayer = self.getVectorLayerByName(self.cmbLayer.currentText())
                    srsDefine = destLayer.srs()
                if srsDefine == vLayer.srs():
                    QMessageBox.information(self, self.tr("Define current projection"), self.tr("Identical output spatial reference system chosen"))
                else:
                    provider = vLayer.dataProvider()
                    self.progressBar.setValue(35)
                    inPath = provider.dataSourceUri()
                    inPath = inPath.remove( QRegExp( "\|.*" ) )
                    self.progressBar.setValue(40)
                    if inPath.endsWith(".shp"):
                        inPath = inPath.left(inPath.length() - 4)
                    self.progressBar.setValue(55)
                    if not srsDefine.isValid():
                        QMessageBox.information(self, self.tr("Define current projection"), self.tr("Output spatial reference system is not valid"))
                    else:
                        self.progressBar.setValue(60)
                        outputWkt = srsDefine.toWkt()
                        self.progressBar.setValue(65)
                        outputFile = QFile( inPath + ".prj" )
                        outputFile.open( QIODevice.WriteOnly | QIODevice.Text )
                        outputPrj = QTextStream( outputFile )
                        self.progressBar.setValue(70)
                        outputPrj << outputWkt
                        self.progressBar.setValue(75)
                        outputPrj.flush()
                        outputFile.close()
                        self.progressBar.setValue(95)
                        vLayer.setCrs(srsDefine)
                        self.progressBar.setValue(100)
                        QMessageBox.information(self, self.tr("Define current projection"), self.tr("Defined Projection For:\n%1.shp").arg( inPath ) )
        self.progressBar.setValue(0)

    def outProjFile(self):
        format = QString( "<h2>%1</h2>%2 <br/> %3" )
        header = QString( "Define layer CRS:" )
        sentence1 = self.tr( "Please select the projection system that defines the current layer." )
        sentence2 = self.tr( "Layer CRS information will be updated to the selected CRS." )
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

#Gets map layer by layername in canvas
#Return: QgsMapLayer
    def getMapLayerByName(self,myName):
        mc = self.iface.mapCanvas()
        nLayers = mc.layerCount()
        for l in range(nLayers):
            layer = mc.layer(l)
            if layer.name() == unicode(myName):
                return layer
