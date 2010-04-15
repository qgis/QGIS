# -*- coding: utf-8 -*-
#-----------------------------------------------------------
# 
# Join Attributes
#
# A QGIS plugin for performing an attribute join between vector layers.
# Attributes from one vector layer are appended to the attribute table of
# another layer through a field common to both vector layers.
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
import struct, itertools, datetime, decimal, ftools_utils
from ui_frmJoinAttributes import Ui_Dialog

class Dialog(QDialog, Ui_Dialog):

  def __init__(self, iface):
    QDialog.__init__(self)
    self.iface = iface
    # Set up the user interface from Designer.
    self.setupUi(self)
    QObject.connect(self.toolOut, SIGNAL("clicked()"), self.outFile)
    QObject.connect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.iupdate)
    QObject.connect(self.joinShape, SIGNAL("currentIndexChanged(QString)"), self.jupdate)
    QObject.connect(self.toolTable, SIGNAL("clicked()"), self.inFile)
    QObject.connect(self.rdoTable, SIGNAL("clicked()"), self.updateTableFields)
    QObject.connect(self.rdoVector, SIGNAL("clicked()"), self.jupdate)
    self.setWindowTitle( self.tr("Join attributes") )
    # populate layer list
    self.progressBar.setValue(0)
    mapCanvas = self.iface.mapCanvas()
    layers = ftools_utils.getLayerNames([QGis.Point, QGis.Line, QGis.Polygon])
    self.inShape.addItems(layers)
    self.joinShape.addItems(layers)

  def iupdate(self, inputLayer):
    changedLayer = ftools_utils.getVectorLayerByName(unicode(inputLayer))
    changedField = ftools_utils.getFieldList(changedLayer)
    self.inField.clear()
    for i in changedField:
      self.inField.addItem(unicode(changedField[i].name()))

  def jupdate(self):
    inputLayer = self.joinShape.currentText()
    if inputLayer != "":
      changedLayer = ftools_utils.getVectorLayerByName(unicode(inputLayer))
      changedField = ftools_utils.getFieldList(changedLayer)
      self.joinField.clear()
      for i in changedField:
        self.joinField.addItem(unicode(changedField[i].name()))
  
  def accept(self):
    if self.inShape.currentText() == "":
      QMessageBox.information(self, self.tr("Join Attributes"), self.tr("Please specify target vector layer"))
    elif self.outShape.text() == "":
      QMessageBox.information(self, self.tr("Join Attributes"), self.tr("Please specify output shapefile"))
    elif self.joinShape.currentText() == "" and self.rdoVector.isChecked():
      QMessageBox.information(self, self.tr("Join Attributes"), self.tr("Please specify join vector layer"))
    elif self.inField.currentText() == "":
      QMessageBox.information(self, self.tr("Join Attributes"), self.tr("Please specify target join field"))
    elif self.joinField.currentText() == "":
      QMessageBox.information(self, self.tr("Join Attributes"), self.tr("Please specify join field"))
    elif self.inTable.text() == "" and self.rdoTable.isChecked():
      QMessageBox.information(self, self.tr("Join Attributes"), self.tr("Please specify input table"))
    else:
      keep = self.rdoKeep.isChecked()
      inName = self.inShape.currentText()
      inField = self.inField.currentText()
      if self.rdoVector.isChecked():
        joinName = self.joinShape.currentText()
        useTable = False
      else:
        joinName = self.inTable.text()
        useTable = True
      joinField = self.joinField.currentText()
      outPath = self.outShape.text()
      self.compute(inName, inField, joinName, joinField, outPath, keep, useTable, self.progressBar)
      self.outShape.clear()
      if res:
        addToTOC = QMessageBox.question(self, self.tr("Join Attributes"),
            self.tr("Created output shapefile:\n%1\n\nWould you like to add the new layer to the TOC?")
            .arg( unicode(self.shapefileName) ), QMessageBox.Yes, QMessageBox.No, QMessageBox.NoButton)
        if addToTOC == QMessageBox.Yes:
          if not ftools_utils.addShapeToCanvas( unicode( outPath ) ):
            QMessageBox.warning( self, self.tr("Geoprocessing"), self.tr( "Error loading output shapefile:\n%1" )
            .arg( unicode( outPath ) ))
    self.progressBar.setValue(0)

  def outFile(self):
    self.outShape.clear()
    ( self.shapefileName, self.encoding ) = ftools_utils.saveDialog( self )
    if self.shapefileName is None or self.encoding is None:
      return
    self.outShape.setText( QString( self.shapefileName ) )

  def inFile(self):
    self.outShape.clear()
    fileDialog = QFileDialog()
    fileDialog.setConfirmOverwrite(False)
    outName = fileDialog.getOpenFileName(self, self.tr("Join Table"), ".", "Tables (*.dbf *.csv)")
    fileCheck = QFile(outName)
    if fileCheck.exists():
      filePath = QFileInfo(outName).absoluteFilePath()
      if not outName.isEmpty():
        self.inTable.clear()
        self.inTable.insert(filePath)
        self.updateTableFields()
    else:
      QMessageBox.warning(self, self.tr("Join Attributes"), self.tr("Input table does not exist"))

  def updateTableFields(self):
    if self.inTable.text() != "":
      filePath = self.inTable.text()
      joinInfo = QFileInfo(filePath)
      joinPath = joinInfo.absoluteFilePath()
      joinName = joinInfo.completeBaseName()
      self.joinField.clear()
      changedLayer = QgsVectorLayer(joinPath, joinName, 'ogr')
      try:
        changedField = ftools_utils.getFieldList(changedLayer)
      except:
        QMessageBox.warning(self, self.tr("Join Attributes"), self.tr("Unable to read input table!"))
        return
      for i in changedField:
        self.joinField.addItem(unicode(changedField[i].name()))

  def compute(self, inName, inField, joinName, joinField, outName, keep, useTable, progressBar):
    layer1 = ftools_utils.getVectorLayerByName(inName)
    provider1 = layer1.dataProvider()
    allAttrs = provider1.attributeIndexes()
    provider1.select(allAttrs)
    fieldList1 = ftools_utils.getFieldList(layer1).values()
    index1 = provider1.fieldNameIndex(inField)
    if useTable:
      joinInfo = QFileInfo(joinName)
      joinPath = joinInfo.absoluteFilePath()
      joinName = joinInfo.completeBaseName()
      layer2 = QgsVectorLayer(joinPath, joinName, 'ogr')
      useTable = False
    else:
      layer2 = ftools_utils.getVectorLayerByName(joinName)
    provider2 = layer2.dataProvider()
    allAttrs = provider2.attributeIndexes()
    provider2.select(allAttrs, QgsRectangle(), False, False)
    fieldList2 = ftools_utils.getFieldList(layer2)
    index2 = provider2.fieldNameIndex(joinField)
    fieldList2 = self.testForUniqueness(fieldList1, fieldList2.values())
    seq = range(0, len(fieldList1) + len(fieldList2))
    fieldList1.extend(fieldList2)
    fieldList1 = dict(zip(seq, fieldList1))
    # check for correct field names
    longNames = ftools_utils.checkFieldNameLenght( fieldList1 )
    if not longNames.isEmpty():
      QMessageBox.warning( self, self.tr( 'Incorrect field names' ),
                  self.tr( 'No output will be created.\nFollowing field names are longer then 10 characters:\n%1' )
                  .arg( longNames.join( '\n' ) ) )
      return False
    sRs = provider1.crs()
    progressBar.setValue(13)
    check = QFile(self.shapefileName)
    if check.exists():
      if not QgsVectorFileWriter.deleteShapeFile(self.shapefileName):
        QMessageBox.warning( self, self.tr( 'Error deleting shapefile' ),
                    self.tr( "Can't delete existing shapefile\n%1" ).arg( self.shapefileName ) )
        return False
    writer = QgsVectorFileWriter(self.shapefileName, self.encoding, fieldList1, provider1.geometryType(), sRs)
    inFeat = QgsFeature()
    outFeat = QgsFeature()
    joinFeat = QgsFeature()
    inGeom = QgsGeometry()
    nElement = 0
    nFeats = provider1.featureCount()
    progressBar.setRange(nElement, nFeats)
    count = 0
    provider1.rewind()
    while provider1.nextFeature(inFeat):
      inGeom = QgsGeometry(inFeat.geometry())
      atMap1 = inFeat.attributeMap()
      outFeat.setAttributeMap(atMap1)
      outFeat.setGeometry(inGeom)
      none = True
      provider2.select(allAttrs, QgsRectangle(), False, False)
      while provider2.nextFeature(joinFeat):
          atMap2 = joinFeat.attributeMap()
          if atMap1[index1] == atMap2[index2]:
            none = False
            atMap = atMap1.values()
            atMap2 = atMap2.values()
            atMap.extend(atMap2)
            atMap = dict(zip(seq, atMap))
            break
      if none:
        outFeat.setAttributeMap(atMap1)
      else:
        outFeat.setAttributeMap(atMap)
      if keep: # keep all records
        writer.addFeature(outFeat)
      else: # keep only matching records
        if not none:
          writer.addFeature(outFeat)
      nElement += 1
      progressBar.setValue(nElement)
    del writer
    return True

  def createFieldList(self, table, joinField):
    fieldList = {}
    item = 0
    for i in table[0]:
      if unicode(i) == unicode(joinField):
        index2 = item
      info = table[1][item]
      name = i
      if info[0] == "C" or info[0] == "M" or info[0] == "D":
        qtype = QVariant.String
        ntype = "string"
      else:
        ntype = "double"
        qtype = QVariant.Double
      length = info[1]
      prec = info[2]
      field = QgsField(name, qtype, ntype, length, prec, self.tr("joined fields"))
      fieldList[item] = field
      item = item + 1
    return (fieldList, index2)
