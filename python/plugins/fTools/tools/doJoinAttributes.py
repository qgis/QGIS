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
    for i in range(mapCanvas.layerCount()):
      layer = mapCanvas.layer(i)
      if layer.type() == layer.VectorLayer:
        self.inShape.addItem(layer.name())
        self.joinShape.addItem(layer.name())

  def iupdate(self, inputLayer):
    changedLayer = self.getVectorLayerByName(unicode(inputLayer))
    changedField = self.getFieldList(changedLayer)
    self.inField.clear()
    for i in changedField:
      self.inField.addItem(unicode(changedField[i].name()))

  def jupdate(self):
    inputLayer = self.joinShape.currentText()
    if inputLayer != "":
      changedLayer = self.getVectorLayerByName(unicode(inputLayer))
      changedField = self.getFieldList(changedLayer)
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
      if outPath.contains("\\"):
        outName = outPath.right((outPath.length() - outPath.lastIndexOf("\\")) - 1)
      else:
        outName = outPath.right((outPath.length() - outPath.lastIndexOf("/")) - 1)
      if outName.endsWith(".shp"):
        outName = outName.left(outName.length() - 4)
      self.compute(inName, inField, joinName, joinField, outPath, keep, useTable, self.progressBar)
      self.outShape.clear()
      addToTOC = QMessageBox.question(self, self.tr("Join Attributes"), self.tr("Created output shapefile:\n%1\n\nWould you like to add the new layer to the TOC?").arg( unicode(self.shapefileName) ), QMessageBox.Yes, QMessageBox.No, QMessageBox.NoButton)
      if addToTOC == QMessageBox.Yes:
        vlayer = QgsVectorLayer(self.shapefileName, outName, "ogr")
        QgsMapLayerRegistry.instance().addMapLayer(vlayer)
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
    outName = fileDialog.getOpenFileName(self, self.tr("Join Table"), ".", "DBase Files (*.dbf)")
    fileCheck = QFile(outName)
    if fileCheck.exists():
      filePath = QFileInfo(outName).absoluteFilePath()
      if filePath.right(4).toLower() != ".dbf": filePath = filePath + ".dbf"
      if not outName.isEmpty():
        self.inTable.clear()
        self.inTable.insert(filePath)
        self.updateTableFields()
    else:
      QMessageBox.warning(self, self.tr("Join Attributes"), self.tr("Input table does not exist"))

  def updateTableFields(self):
    if self.inTable.text() != "":
      filePath = self.inTable.text()
      f = open(unicode(filePath), 'rb')
      table = list(self.dbfreader(f))
      f.close()
      self.joinField.clear()
      for i in table[0]:
        self.joinField.addItem(unicode(i))
      table = None

  def compute(self, inName, inField, joinName, joinField, outName, keep, useTable, progressBar):
    layer1 = self.getVectorLayerByName(inName)
    provider1 = layer1.dataProvider()
    allAttrs = provider1.attributeIndexes()
    provider1.select(allAttrs)
    fieldList1 = self.getFieldList(layer1).values()
    index1 = provider1.fieldNameIndex(inField)
    if useTable:
      f = open(unicode(joinName), 'rb')
      table = list(self.dbfreader(f))
      f.close()
      (fieldList2, index2) = self.createFieldList(table, joinField)
      table = table[2:]
      func = lambda x: (unicode(type(x)) != "<type 'str'>" and QVariant(float(x))) or (QVariant(x))
      table = map(lambda f: map(func, f), table)

    else:
      layer2 = self.getVectorLayerByName(joinName)
      provider2 = layer2.dataProvider()
      allAttrs = provider2.attributeIndexes()
      provider2.select(allAttrs)
      fieldList2 = self.getFieldList(layer2)
      index2 = provider2.fieldNameIndex(joinField)
    fieldList2 = self.testForUniqueness(fieldList1, fieldList2.values())
    seq = range(0, len(fieldList1) + len(fieldList2))
    fieldList1.extend(fieldList2)
    fieldList1 = dict(zip(seq, fieldList1))
    sRs = provider1.crs()
    progressBar.setValue(13)
    check = QFile(self.shapefileName)
    if check.exists():
      if not QgsVectorFileWriter.deleteShapeFile(self.shapefileName):
        return
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
      inGeom = inFeat.geometry()
      atMap1 = inFeat.attributeMap()
      outFeat.setAttributeMap(atMap1)
      outFeat.setGeometry(inGeom)
      none = True
      if useTable:
        for i in table:
          #sequence = range(0, len(table[0]))
          #atMap2 = dict(zip(sequence, i))
          if atMap1[index1].toString().trimmed() == i[index2].toString().trimmed():
            count = count + 1
            none = False
            atMap = atMap1.values()
            atMap2 = i
            atMap.extend(atMap2)
            atMap = dict(zip(seq, atMap))
            break
      else:
        provider2.rewind()
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

  def testForUniqueness(self, fieldList1, fieldList2):
    changed = True
    while changed:
      changed = False
      for i in fieldList1:
        for j in fieldList2:
          if j.name() == i.name():
            j = self.createUniqueFieldName(j)
            changed = True
    return fieldList2
  
  def createUniqueFieldName(self, field):
    check = field.name().right(2)
    if check.startsWith("_"):
      (val,test) = check.right(1).toInt()
      if test:
        if val < 2:
          val = 2
        else:
          val = val + 1
        field.setName(field.name().left(len(field.name())-1) + unicode(val))
      else:
        field.setName(field.name() + "_2")
    else:
      field.setName(field.name() + "_2")
    return field
  
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
          QMessageBox.information(self, self.tr("Join Attributes"), self.tr("Vector layer is not valid"))

  def getFieldList(self, vlayer):
    fProvider = vlayer.dataProvider()
    feat = QgsFeature()
    allAttrs = fProvider.attributeIndexes()
    fProvider.select(allAttrs)
    myFields = fProvider.fields()
    return myFields
  def dbfreader(self, f):
      """Returns an iterator over records in a Xbase DBF file.

      The first row returned contains the field names.
      The second row contains field specs: (type, size, decimal places).
      Subsequent rows contain the data records.
      If a record is marked as deleted, it is skipped.

      File should be opened for binary reads.

      """
      numrec, lenheader = struct.unpack('<xxxxLH22x', f.read(32))
      numfields = (lenheader - 33) // 32

      fields = []
      for fieldno in xrange(numfields):
          name, typ, size, deci = struct.unpack('<11sc4xBB14x', f.read(32))
          name = name.replace('\0', '')       # eliminate NULs from string
          fields.append((name, typ, size, deci))
      yield [field[0] for field in fields]
      yield [tuple(field[1:]) for field in fields]

      terminator = f.read(1)
      assert terminator == '\r'

      fields.insert(0, ('DeletionFlag', 'C', 1, 0))
      fmt = ''.join(['%ds' % fieldinfo[2] for fieldinfo in fields])
      fmtsiz = struct.calcsize(fmt)
      for i in xrange(numrec):
          record = struct.unpack(fmt, f.read(fmtsiz))
          if record[0] != ' ':
              continue                        # deleted record
          result = []
          for (name, typ, size, deci), value in itertools.izip(fields, record):
              if name == 'DeletionFlag':
                  continue
              if typ == "N":
                  value = value.replace('\0', '').lstrip()
                  if value == '':
                      value = 0
                  elif deci:
                      value = decimal.Decimal(value)
                  else:
                      value = int(value)
              elif typ == 'D':
                  y, m, d = int(value[:4]), int(value[4:6]), int(value[6:8])
                  value = datetime.date(y, m, d)
              elif typ == 'L':
                  value = (value in 'YyTt' and 'T') or (value in 'NnFf' and 'F') or '?'
              result.append(value)
          yield result
