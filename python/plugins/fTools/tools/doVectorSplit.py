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
#import os, sys, string, math
import ftools_utils
from ui_frmVectorSplit import Ui_Dialog

class Dialog(QDialog, Ui_Dialog):
    def __init__(self, iface):
        QDialog.__init__(self)
        self.iface = iface
        # Set up the user interface from Designer.
        self.setupUi(self)
        QObject.connect(self.toolOut, SIGNAL("clicked()"), self.outFile)
        QObject.connect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.update)
        self.setWindowTitle(self.tr("Split vector layer"))
        self.buttonOk = self.buttonBox_2.button( QDialogButtonBox.Ok )
        # populate layer list
        self.progressBar.setValue(0)
        mapCanvas = self.iface.mapCanvas()
        layers = ftools_utils.getLayerNames([QGis.Point, QGis.Line, QGis.Polygon])
        self.inShape.addItems(layers)

    def update(self, inputLayer):
        self.inField.clear()
        changedLayer = ftools_utils.getVectorLayerByName(inputLayer)
        changedField = ftools_utils.getFieldList(changedLayer)
        for i in changedField:
            self.inField.addItem(unicode(changedField[i].name()))

    def accept(self):
        self.buttonOk.setEnabled( False )
        if self.inShape.currentText() == "":
            QMessageBox.information(self, self.tr("Vector Split"), self.tr("No input shapefile specified"))
        elif self.outShape.text() == "":
            QMessageBox.information(self, self.tr("Vector Split"), self.tr("Please specify output shapefile"))
        else:
            inField = self.inField.currentText()
            inLayer = ftools_utils.getVectorLayerByName(unicode(self.inShape.currentText()))
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
            QMessageBox.information(self, self.tr("Vector Split"), self.tr("Created output shapefiles in folder:\n%1").arg( outPath))
        self.progressBar.setValue(0)
        self.buttonOk.setEnabled( True )

    def outFile(self):
        self.outShape.clear()
        ( self.folderName, self.encoding ) = ftools_utils.dirDialog( self )
        if self.folderName is None or self.encoding is None:
            return
        self.outShape.setText( QString( self.folderName ) )

    def split(self, vlayer, outPath, inField, progressBar):
        provider = vlayer.dataProvider()
        #unique = []
        index = provider.fieldNameIndex(inField)
        #provider.uniqueValues(index, unique)
        unique = ftools_utils.getUniqueValues(vlayer.dataProvider(), int(index))
        baseName = unicode( outPath + vlayer.name() + "_" + inField + "_" )
        allAttrs = provider.attributeIndexes()
        provider.select(allAttrs)
        fieldList = ftools_utils.getFieldList(vlayer)
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
