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

  def fillInputFilesEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      files = Utils.FileDialog.getOpenFileNames(self, self.tr( "Select the files to Merge" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if files.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.inputFilesEdit.setText(files.join(","))

      if files.count() < 2:
        self.intersectCheck.setChecked( False )
        self.intersectCheck.setEnabled( False )
      else:
        self.intersectCheck.setEnabled( True )
        (self.xmax, self.ymax, self.xmin, self.ymin) = self.getExtent()
 
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
        arguments << "-ul_lr"
        arguments << str( self.xmin ) << str( self.ymax ) << str( self.xmax ) << str( self.ymin )
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
    while i < files.count():
      if i == 0:
        rect1 = self.getRectangle( files[ 0 ] )
        rect2 = self.getRectangle( files[ 1 ] )
        res = rect1.intersect( rect2 )
        rect1 = res
        i = 2
        continue
      rect2 = self.getRectangle( files[ i ] )
      res = rect1.intersect( rect2 )
      i = i + 1

    xMax = res.xMaximum()
    xMin = res.xMinimum()
    yMax = res.yMaximum()
    yMin = res.yMinimum()

    return ( xMax, yMax, xMin, yMin )
