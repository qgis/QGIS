# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_widgetInfo import Ui_GdalToolsWidget as Ui_Widget
from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils

import platform

class GdalToolsDialog( QWidget, Ui_Widget, BasePluginWidget ):

  def __init__( self, iface ):
      QWidget.__init__( self )
      self.iface = iface

      self.setupUi( self )
      BasePluginWidget.__init__( self, self.iface, "gdalinfo" )

      # we don't need load to canvas functionality
      self.base.loadCheckBox.hide()
      # make window large
      self.base.resize( 400, 360 )

      self.setParamsStatus(
        [
          (self.inputLayerCombo, [SIGNAL("currentIndexChanged(int)"), SIGNAL("editTextChanged(const QString &)")] ),
          ( self.suppressGCPCheck, SIGNAL( "stateChanged( int )" ) ),
          ( self.suppressMDCheck, SIGNAL( "stateChanged( int )" ) )
        ]
      )

      self.connect( self.selectInputFileButton, SIGNAL( "clicked()" ), self.fillInputFileEdit )

      # helper actions for copying info output
      self.copyLine = QAction( self.tr( "Copy" ), self )
      QObject.connect( self.copyLine, SIGNAL( "triggered()" ), self.doCopyLine )
      self.copyAll = QAction( self.tr( "Copy all" ), self )
      QObject.connect( self.copyAll, SIGNAL( "triggered()" ), self.doCopyAll )

      # fill layers combo
      self.fillInputLayerCombo()

  def doCopyLine( self ):
      output = QString()
      items = self.rasterInfoList.selectedItems()
      for r in items:
        output.append( r.text() + "\n" )
      if not output.isEmpty():
        clipboard = QApplication.clipboard()
        clipboard.setText( output )

  def doCopyAll( self ):
      output = QString()
      for r in range( self.rasterInfoList.count() ):
        output.append( self.rasterInfoList.item( r ).text() + "\n" )
      if not output.isEmpty():
        clipboard = QApplication.clipboard()
        clipboard.setText( output )

  def keyPressEvent( self, e ):
      if ( e.modifiers() == Qt.ControlModifier or e.modifiers() == Qt.MetaModifier ) and e.key() == Qt.Key_C:
        items = QString()
        for r in range( self.rasterInfoList.count() ):
          items.append( self.rasterInfoList.item( r ).text() + "\n" )
        if not items.isEmpty():
          clipboard = QApplication.clipboard()
          clipboard.setText( items )
      else:
        QWidget.keyPressEvent( self, e )

  def fillInputLayerCombo( self ):
      self.inputLayerCombo.clear()
      ( self.layers, names ) = Utils.getRasterLayers()
      self.inputLayerCombo.addItems( names )

  def finished( self ):
      self.rasterInfoList.clear()
      arr = QByteArray()
      arr = self.base.process.readAllStandardOutput()
      if platform.system() == "Windows":
        info = QString( arr ).trimmed().split( "\r\n" )
      else:
        info = QString( arr ).trimmed().split( "\n" )
      self.rasterInfoList.addItems( info )

  def fillInputFileEdit( self ):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName( self, self.tr( "Select the file to analyse" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if inputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter( lastUsedFilter )

      self.inputLayerCombo.setCurrentIndex(-1)
      self.inputLayerCombo.setEditText( inputFile )

  def getArguments( self ):
      arguments = QStringList()
      if self.suppressGCPCheck.isChecked():
        arguments << "-nogcp"
      if self.suppressMDCheck.isChecked():
        arguments << "-nomd"
      if self.inputLayerCombo.currentIndex() >= 0:
        arguments << self.layers[ self.inputLayerCombo.currentIndex() ].source()
      else:
        arguments << self.inputLayerCombo.currentText()
      return arguments

#  def getOutputFileName( self ):
#      return self.inputFileEdit.text()

  def contextMenuEvent( self, event ):
      menu = QMenu( self )
      menu.addAction( self.copyLine )
      menu.addAction( self.copyAll )
      menu.exec_( event.globalPos() )
