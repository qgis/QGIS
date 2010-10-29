# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_widgetMerge import Ui_GdalToolsWidget as Ui_Widget
from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils

class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

  def __init__(self, iface):
      QWidget.__init__(self)
      self.iface = iface

      self.setupUi(self)
      BasePluginWidget.__init__(self, self.iface, "gdal_merge.py")

      self.outputFormat = Utils.fillRasterOutputFormat()
      self.extent = None

      self.setParamsStatus(
        [
          (self.inputFilesEdit, SIGNAL("textChanged(const QString &)")), 
          (self.outputFileEdit, SIGNAL("textChanged(const QString &)")), 
          (self.noDataSpin, SIGNAL("valueChanged(int)"), self.noDataCheck),
	      ( self.separateCheck, SIGNAL( "stateChanged( int )" ) ),
          ( self.pctCheck, SIGNAL( "stateChanged( int )" ) ),
          ( self.intersectCheck, SIGNAL( "stateChanged( int )" ) ),
          (self.creationOptionsTable, [SIGNAL("cellValueChanged(int, int)"), SIGNAL("rowRemoved()")], self.creationGroupBox)
        ]
      )

      self.connect(self.selectInputFilesButton, SIGNAL("clicked()"), self.fillInputFilesEdit)
      self.connect(self.selectOutputFileButton, SIGNAL("clicked()"), self.fillOutputFileEdit)
      self.connect(self.intersectCheck, SIGNAL("stateChanged(int)"), self.refreshExtent)

  def fillInputFilesEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      files = Utils.FileDialog.getOpenFileNames(self, self.tr( "Select the files to Merge" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if files.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.inputFilesEdit.setText(files.join(","))
      self.intersectCheck.setEnabled( files.count() > 1 )
      self.refreshExtent()

  def refreshExtent(self):
      files = self.inputFilesEdit.text().split( "," )
      if files.count() < 2:
        self.intersectCheck.setChecked( False )
        self.extent = None
        return

      self.extent = self.getExtent()
      self.someValueChanged()

      if not self.intersectCheck.isChecked():
        return

      if self.extent == None:
        QMessageBox.warning( self, self.tr( "Error retrieving the extent" ), self.tr( 'GDAL was unable to retrieve the extent from any file. \nThe "Use intersected extent" option will be unchecked.' ) )
        self.intersectCheck.setChecked( False )
      elif self.extent.isEmpty():
        QMessageBox.warning( self, self.tr( "Empty extent" ), self.tr( 'The computed extent is empty. \nDisable the "Use intersected extent" option to have a nonempty output.' ) )
 
  def fillOutputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      outputFile = Utils.FileDialog.getSaveFileName(self, self.tr( "Select where to save the Merge output" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if outputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.outputFormat = Utils.fillRasterOutputFormat( lastUsedFilter, outputFile )
      self.outputFileEdit.setText( outputFile )

  def getArguments(self):
      arguments = QStringList()
      if self.intersectCheck.isChecked():
        if self.extent != None:
          arguments << "-ul_lr"
          arguments << str( self.extent.xMinimum() )
          arguments << str( self.extent.yMaximum() )
          arguments << str( self.extent.xMaximum() )
          arguments << str( self.extent.yMinimum() )
      if self.noDataCheck.isChecked():
        arguments << "-n"
        arguments << str(self.noDataSpin.value())
      if self.separateCheck.isChecked():
        arguments << "-separate"
      if self.pctCheck.isChecked():
        arguments << "-pct"
      if self.creationGroupBox.isChecked():
        for opt in self.creationOptionsTable.options():
          arguments << "-co"
          arguments << opt
      if not self.outputFileEdit.text().isEmpty():
        arguments << "-of"
        arguments << self.outputFormat
        arguments << "-o"
        arguments << self.outputFileEdit.text()
      arguments << self.inputFilesEdit.text().split(",")
      return arguments

  def getOutputFileName(self):
      return self.outputFileEdit.text()

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())

  def getRectangle( self, file ):
    processSRS = QProcess( self )
    processSRS.start( "gdalinfo", QStringList() << file, QIODevice.ReadOnly )
    arr = QByteArray()
    if processSRS.waitForFinished():
      arr = processSRS.readAllStandardOutput()
      processSRS.close()
      
    if arr.isEmpty():
      return None
      
    info = QString( arr ).split( "\n" )
    ulCoord = info[ info.indexOf( QRegExp( "^Upper\sLeft.*" ) ) ].simplified()
    lrCoord = info[ info.indexOf( QRegExp( "^Lower\sRight.*" ) ) ].simplified()
    ul = ulCoord.split( " " )
    lr = lrCoord.split( " " )
    xUL = ul[ 3 ].replace( ",", "" ).toDouble() [ 0 ]
    yUL = ul[ 4 ].replace( ")", "" ).toDouble()[ 0 ]
    xLR = lr[ 3 ].replace( ",", "" ).toDouble()[ 0 ]
    yLR = lr[ 4 ].replace( ")", "" ).toDouble()[ 0 ]

    return QgsRectangle( xUL, yLR, xLR, yUL )

  def getExtent( self ):
    files = self.inputFilesEdit.text().split( "," )

    i = 0
    res = rect2 = None
    for fileName in files:
      if res == None:
        res = self.getRectangle( fileName )
        continue
      rect2 = self.getRectangle( fileName )
      if rect2 == None:
        continue
      res = res.intersect( rect2 )

    return res
