# -*- coding: utf-8 -*-

"""
***************************************************************************
    doSettings.py
    ---------------------
    Date                 : June 2010
    Copyright            : (C) 2010 by Giuseppe Sucameli
    Email                : brush dot tyler at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Giuseppe Sucameli'
__date__ = 'June 2010'
__copyright__ = '(C) 2010, Giuseppe Sucameli'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

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
    self.leGdalBinPath.setText( Utils.getGdalBinPath() )
    QObject.connect( self.btnSetBinPath, SIGNAL( "clicked()" ), self.setBinPath )
    self.bin_tooltip_label.setPixmap( QPixmap(':/icons/tooltip.png') )
    self.bin_tooltip_label.setToolTip( self.tr( \
u"""A list of colon-separated (Linux and MacOS) or
semicolon-separated (Windows) paths to both binaries
and python executables.

MacOS users usually need to set it to something like
/Library/Frameworks/GDAL.framework/Versions/1.8/Programs""") )

    # python modules
    self.leGdalPymodPath.setText( Utils.getGdalPymodPath() )
    QObject.connect( self.btnSetPymodPath, SIGNAL( "clicked()" ), self.setPymodPath )
    self.pymod_tooltip_label.setPixmap( QPixmap(':/icons/tooltip.png') )
    self.pymod_tooltip_label.setToolTip( self.tr( \
u"""A list of colon-separated (Linux and MacOS) or
semicolon-separated (Windows) paths to python modules.""") )

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

  def setPymodPath( self ):
    inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select directory with GDAL python modules" ) )
    if inputDir.isEmpty():
      return

    self.leGdalPymodPath.setText( inputDir )

  def setHelpPath( self ):
    inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select directory with the GDAL documentation" ) )
    if inputDir.isEmpty():
      return

    self.leGdalHelpPath.setText( inputDir )


  def accept( self ):
    Utils.setGdalBinPath( self.leGdalBinPath.text() )
    Utils.setGdalPymodPath( self.leGdalPymodPath.text() )
    Utils.setHelpPath( self.leGdalHelpPath.text() )
    QDialog.accept( self )

