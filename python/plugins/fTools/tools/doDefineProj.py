# -*- coding: utf-8 -*-
#-----------------------------------------------------------
#
# fTools
# Copyright (C) 2008-2011  Carson Farmer
# EMAIL: carson.farmer (at) gmail.com
# WEB  : http://www.ftools.ca/fTools.html
#
# A collection of data management and analysis tools for vector data
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
from qgis.gui import *
import ftools_utils
from ui_frmReProject import Ui_Dialog

class Dialog(QDialog, Ui_Dialog):
    def __init__(self, iface):
        QDialog.__init__(self, iface.mainWindow())
        self.iface = iface
        self.setupUi(self)
        self.toolOut.setEnabled(False)
        self.toolOut.setVisible(False)
        self.outShape.setEnabled(False)
        self.outShape.setVisible(False)
        self.label_2.setVisible(False)
        self.label_2.setEnabled(False)
        self.setWindowTitle(self.tr("Define current projection"))
        self.buttonOk = self.buttonBox_2.button( QDialogButtonBox.Ok )
        QObject.connect(self.btnProjection, SIGNAL("clicked()"), self.outProjFile)
        QObject.connect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.updateProj1)
        QObject.connect(self.cmbLayer, SIGNAL("currentIndexChanged(QString)"), self.updateProj2)
        # populate layer list
        self.progressBar.setValue(0)
        mapCanvas = self.iface.mapCanvas()
        layers = ftools_utils.getLayerNames([QGis.Point, QGis.Line, QGis.Polygon])
        self.inShape.addItems(layers)
        self.cmbLayer.addItems(layers)

        self.crs = None

    def updateProj1(self, layerName):
        self.inRef.clear()
        tempLayer = ftools_utils.getVectorLayerByName(layerName)
        crs = tempLayer.dataProvider().crs()
        if crs.isValid():
          self.inRef.insert(crs.authid() + " - " +  crs.description())
        else:
          self.inRef.insert( self.tr( "Missing or invalid CRS" ) )

    def updateProj2(self, layerName):
        self.outRef.clear()
        tempLayer = ftools_utils.getVectorLayerByName(layerName)
        crs = tempLayer.dataProvider().crs()
        if crs.isValid():
          self.outRef.insert(crs.authid() + " - " +  crs.description())
        else:
          self.outRef.insert( self.tr( "Missing or invalid CRS" ) )

    def accept(self):
        self.buttonOk.setEnabled( False )
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
                srsDefine = None
                if self.rdoProjection.isChecked():
                    outProj = self.txtProjection.text().split( " - " )[ 0 ]
                    srsDefine = self.crs
                else:
                    destLayer = ftools_utils.getVectorLayerByName(self.cmbLayer.currentText())
                    srsDefine = destLayer.crs()
                if srsDefine == vLayer.crs():
                    responce = QMessageBox.question(self, self.tr("Define current projection"),
                    self.tr("Identical output spatial reference system chosen\n\nAre you sure you want to proceed?"),
                    QMessageBox.Yes, QMessageBox.No, QMessageBox.NoButton)
                    if responce == QMessageBox.No:
                        return
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
                    outputPrj << outputWkt
                    outputPrj.flush()
                    outputFile.close()
                    self.progressBar.setValue(70)
                    checkFile = QFile( inPath + ".qpj" )
                    if checkFile.exists():
                        checkFile.open( QIODevice.WriteOnly | QIODevice.Text )
                        outputPrj = QTextStream( checkFile )
                        outputPrj << outputWkt
                        outputPrj.flush()
                        checkFile.close()
                    self.progressBar.setValue(95)
                    vLayer.setCrs(srsDefine)
                    self.progressBar.setValue(100)
                    QMessageBox.information(self, self.tr("Define current projection"), self.tr("Defined Projection For:\n%1.shp").arg( inPath ) )
        self.progressBar.setValue(0)
        self.buttonOk.setEnabled( True )

    def outProjFile(self):
        format = QString( "<h2>%1</h2>%2 <br/> %3" )
        header = QString( "Define layer CRS:" )
        sentence1 = self.tr( "Please select the projection system that defines the current layer." )
        sentence2 = self.tr( "Layer CRS information will be updated to the selected CRS." )
        projSelector = QgsGenericProjectionSelector(self)
        projSelector.setMessage( format.arg( header ).arg( sentence1 ).arg( sentence2 ))
        if projSelector.exec_():
            self.crs = QgsCoordinateReferenceSystem( projSelector.selectedCrsId(), QgsCoordinateReferenceSystem.InternalCrsId )
            if projSelector.selectedAuthId().isEmpty():
                QMessageBox.information(self, self.tr("Export to new projection"), self.tr("No Valid CRS selected"))
                return
            else:
                self.txtProjection.clear()
                self.txtProjection.insert(self.crs.authid() + " - " + self.crs.description())
        else:
            return
