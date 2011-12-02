# -*- coding: utf-8 -*-

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_dialogSettings import Ui_GdalToolsSettingsDialog as Ui_Dialog
import GdalTools_utils as Utils

from .. import resources_rc

class GdalToolsSettingsDialog( QDialog, Ui_Dialog ):
  def __init__( self, iface ):
    QDialog.__init__( self, iface.mainWindow() )
    self.setAttribute(Qt.WA_DeleteOnClose)
    self.iface = iface
    self.setupUi( self )

    # binaries
    self.leGdalBinPath.setText( Utils.getGdalPath() )
    QObject.connect( self.btnSetBinPath, SIGNAL( "clicked()" ), self.setBinPath )
    self.bin_tooltip_label.setPixmap( QPixmap(':/icons/tooltip.png') )
    self.bin_tooltip_label.setToolTip( self.tr( \
u"""A list of colon-separated (Linux and MacOS) or 
semicolon-separated (Windows) paths to executables.

MacOS users usually need to set it to something like
/Library/Frameworks/GDAL.framework/Versions/1.8/Programs""") )

    # help
    self.leGdalHelpPath.setText( Utils.getHelpPath() )
    QObject.connect( self.btnSetHelpPath, SIGNAL( "clicked()" ), self.setHelpPath )
    self.help_tooltip_label.setPixmap( QPixmap(':/icons/tooltip.png') )
    self.help_tooltip_label.setToolTip( self.tr( \
u"""Useful to open local GDAL documentation instead of online help 
when pressing on the tool dialog's Help button.""") )


  def setBinPath( self ):
    inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select directory with GDAL executables" ) )
    if inputDir.isEmpty():
      return

    self.leGdalBinPath.setText( inputDir )

  def setHelpPath( self ):
    inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select directory with the GDAL documentation" ) )
    if inputDir.isEmpty():
      return

    self.leGdalHelpPath.setText( inputDir )


  def accept( self ):
    Utils.setGdalPath( self.leGdalBinPath.text() )
    Utils.setHelpPath( self.leGdalHelpPath.text() )
    QDialog.accept( self )
