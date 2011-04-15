# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_widgetTileIndex import Ui_GdalToolsWidget as Ui_Widget
from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils

import os.path

class GdalToolsDialog( QWidget, Ui_Widget, BasePluginWidget ):

  def __init__( self, iface ):
      QWidget.__init__( self )
      self.iface = iface

      self.setupUi( self )
      BasePluginWidget.__init__( self, self.iface, "gdaltindex" )

      self.inSelector.setType( self.inSelector.FILE )
      self.outSelector.setType( self.outSelector.FILE )

      self.setParamsStatus(
        [
          ( self.inSelector, SIGNAL( "filenameChanged()" ) ),
          #( self.recurseCheck, SIGNAL( "stateChanged( int )" ),
          ( self.outSelector, SIGNAL( "filenameChanged()" ) ),
          ( self.indexFieldEdit, SIGNAL( "textChanged( const QString & )" ), self.indexFieldCheck),
          ( self.absolutePathCheck, SIGNAL( "stateChanged( int )" ), None, "1.5.0" ),
          ( self.skipDifferentProjCheck, SIGNAL( "stateChanged( int )" ), None, "1.5.0" )
        ]
      )

      self.connect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputDirEdit )
      self.connect( self.outSelector, SIGNAL( "selectClicked()" ), self.fillOutputFileEdit )

  def fillInputDirEdit( self ):
      inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select the input directory with raster files" ))
      if inputDir.isEmpty():
        return
      self.inSelector.setFilename( inputDir )

  def fillOutputFileEdit( self ):
      lastUsedFilter = Utils.FileFilter.lastUsedVectorFilter()
      outputFile, encoding = Utils.FileDialog.getSaveFileName( self, self.tr( "Select where to save the TileIndex output" ), Utils.FileFilter.allVectorsFilter(), lastUsedFilter, True )
      if outputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedVectorFilter(lastUsedFilter)

      self.outputFormat = Utils.fillVectorOutputFormat( lastUsedFilter, outputFile )
      self.outSelector.setFilename( outputFile )
      self.lastEncoding = encoding

  def getArguments( self ):
      arguments = QStringList()
      if self.indexFieldCheck.isChecked() and not self.indexFieldEdit.text().isEmpty():
        arguments << "-tileindex"
        arguments << self.indexFieldEdit.text()
      if self.absolutePathCheck.isChecked():
        arguments << "-write_absolute_path"
      if self.skipDifferentProjCheck.isChecked():
        arguments << "-skip_different_projection"
      arguments << self.getOutputFileName()
      arguments << Utils.getRasterFiles( self.getInputFileName(), self.recurseCheck.isChecked() )
      return arguments

  def getOutputFileName( self ):
      return self.outSelector.filename()

  def getInputFileName( self ):
      return self.inSelector.filename()

  def addLayerIntoCanvas( self, fileInfo ):
      vl = self.iface.addVectorLayer( fileInfo.filePath(), fileInfo.baseName(), "ogr" )
      if vl != None and vl.isValid():
        if hasattr( self, 'lastEncoding' ):
          vl.setProviderEncoding( self.lastEncoding )

