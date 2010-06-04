# -*- coding: utf-8 -*-
"""
/***************************************************************************
Name			 	 : GdalTools
Description          : Integrate gdal tools into qgis
Date                 : 17/Sep/09
copyright            : (C) 2009 by Lorenzo Masini (Faunalia)
email                : lorenxo86@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""
# Import the PyQt and QGIS libraries
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *

# Initialize Qt resources from file resources_rc.py
import resources_rc

# Import required modules and if missing show the right name of them
req_mods = { "osgeo": "osgeo [python-gdal]" }
for k, v in req_mods.iteritems():
  try:
    exec( "import %s" % k )
  except ImportError, e:
    errorStr = str(e)
    if len(v) > 0:
      errorStr = errorStr.replace( k, v ) 
    raise ImportError( errorStr )

# Set up current path, so that we know where to look for modules
import os.path, sys
currentPath = os.path.dirname( __file__ )
sys.path.append( os.path.abspath(os.path.dirname( __file__ ) + '/tools' ) )
import doBuildVRT, doContour, doRasterize, doPolygonize, doMerge, doSieve, doProximity, doNearBlack, doWarp, doGrid, doTranslate, doClipper
import doInfo, doProjection, doOverview, doRgbPct, doPctRgb, doSettings, doAbout

import GdalTools_utils as Utils

class GdalTools:

  def __init__( self, iface ):
    # Save reference to the QGIS interface
    self.iface = iface
    try:
      self.QgisVersion = unicode( QGis.QGIS_VERSION_INT )
    except:
      self.QgisVersion = unicode( QGis.qgisVersion )[ 0 ]

    # For i18n support
    userPluginPath = QFileInfo( QgsApplication.qgisUserDbFilePath() ).path() + "/python/plugins/GdalTools"
    systemPluginPath = QgsApplication.prefixPath() + "/python/plugins/GdalTools"

    overrideLocale = QSettings().value( "locale/overrideFlag", QVariant( False ) ).toBool()
    if not overrideLocale:
      localeFullName = QLocale.system().name()
    else:
      localeFullName = QSettings().value( "locale/userLocale", QVariant( "" ) ).toString()

    if QFileInfo( userPluginPath ).exists():
      translationPath = userPluginPath + "/i18n/GdalTools_" + localeFullName + ".qm"
    else:
      translationPath = systemPluginPath + "/i18n/GdalTools_" + localeFullName + ".qm"

    self.localePath = translationPath
    if QFileInfo( self.localePath ).exists():
      self.translator = QTranslator()
      self.translator.load( self.localePath )
      QCoreApplication.installTranslator( self.translator )

  def initGui( self ):
    if int( self.QgisVersion ) < 1:
      QMessageBox.warning( self.iface.getMainWindow(), "Gdal Tools",
      QCoreApplication.translate( "GdalTools", "Quantum GIS version detected: " ) +unicode( self.QgisVersion )+".xx\n"
      + QCoreApplication.translate( "GdalTools", "This version of Gdal Tools requires at least QGIS version 1.0.0\nPlugin will not be enabled." ) )
      return None

    self.GdalVersion = Utils.Version( Utils.GdalConfig.version() )

    self.menu = QMenu()
    self.menu.setTitle( QCoreApplication.translate( "GdalTools", "&Raster" ) )

    if self.GdalVersion >= "1.6":
      self.buildVRT = QAction( QIcon(":/icons/vrt.png"), QCoreApplication.translate( "GdalTools", "Build Virtual Raster (catalog)" ), self.iface.mainWindow() )
      self.buildVRT.setStatusTip( QCoreApplication.translate( "GdalTools", "Builds a VRT from a list of datasets") )
      QObject.connect( self.buildVRT, SIGNAL( "triggered()" ), self.doBuildVRT )
      self.menu.addAction(self.buildVRT)

    if self.GdalVersion >= "1.6":
      self.contour = QAction( QIcon(":/icons/contour.png"), QCoreApplication.translate( "GdalTools", "Contour" ), self.iface.mainWindow() )
      self.contour.setStatusTip( QCoreApplication.translate( "GdalTools", "Builds vector contour lines from a DEM") )
      QObject.connect( self.contour, SIGNAL( "triggered()" ), self.doContour )
      self.menu.addAction(self.contour)

    if self.GdalVersion >= "1.3":
      self.rasterize = QAction( QIcon(":/icons/rasterize.png"), QCoreApplication.translate( "GdalTools", "Rasterize" ), self.iface.mainWindow() )
      self.rasterize.setStatusTip( QCoreApplication.translate( "GdalTools", "Burns vector geometries into a raster") )
      QObject.connect( self.rasterize, SIGNAL( "triggered()" ), self.doRasterize )
      self.menu.addAction(self.rasterize)

    if self.GdalVersion >= "1.6":
      self.polygonize = QAction( QIcon(":/icons/polygonize.png"), QCoreApplication.translate( "GdalTools", "Polygonize" ), self.iface.mainWindow() )
      self.polygonize.setStatusTip( QCoreApplication.translate( "GdalTools", "Produces a polygon feature layer from a raster") )
      QObject.connect( self.polygonize, SIGNAL( "triggered()" ), self.doPolygonize )
      self.menu.addAction(self.polygonize)

    self.merge = QAction( QIcon(":/icons/merge.png"), QCoreApplication.translate( "GdalTools", "Merge" ), self.iface.mainWindow() )
    self.merge.setStatusTip( QCoreApplication.translate( "GdalTools", "Build a quick mosaic from a set of images") )
    QObject.connect( self.merge, SIGNAL( "triggered()" ), self.doMerge )
    self.menu.addAction(self.merge)

    if self.GdalVersion >= "1.6":
      self.sieve = QAction( QIcon(":/icons/sieve.png"), QCoreApplication.translate( "GdalTools", "Sieve" ), self.iface.mainWindow() )
      self.sieve.setStatusTip( QCoreApplication.translate( "GdalTools", "Removes small raster polygons") )
      QObject.connect( self.sieve, SIGNAL( "triggered()" ), self.doSieve )
      self.menu.addAction(self.sieve)

    if self.GdalVersion >= "1.6":
      self.proximity = QAction( QIcon(":/icons/proximity.png"),  QCoreApplication.translate( "GdalTools", "Proximity" ), self.iface.mainWindow() )
      self.proximity.setStatusTip( QCoreApplication.translate( "GdalTools", "Produces a raster proximity map") )
      QObject.connect( self.proximity, SIGNAL( "triggered()" ), self.doProximity )
      self.menu.addAction(self.proximity)

    if self.GdalVersion >= "1.5":
      self.nearBlack = QAction( QIcon(":/icons/nearblack.png"),  QCoreApplication.translate( "GdalTools", "Near black" ), self.iface.mainWindow() )
      self.nearBlack.setStatusTip( QCoreApplication.translate( "GdalTools", "Convert nearly black/white borders to exact value") )
      QObject.connect( self.nearBlack, SIGNAL( "triggered()" ), self.doNearBlack )
      self.menu.addAction(self.nearBlack)

    self.warp = QAction( QIcon(":/icons/warp.png"),  QCoreApplication.translate( "GdalTools", "Warp" ), self.iface.mainWindow() )
    self.warp.setStatusTip( QCoreApplication.translate( "GdalTools", "Warp an image into a new coordinate system") )
    QObject.connect( self.warp, SIGNAL( "triggered()" ), self.doWarp )
    self.menu.addAction(self.warp)

    if self.GdalVersion >= "1.5":
      self.grid = QAction( QIcon(":/icons/grid.png"), QCoreApplication.translate( "GdalTools", "Grid" ), self.iface.mainWindow() )
      self.grid.setStatusTip( QCoreApplication.translate( "GdalTools", "Create raster from the scattered data") )
      QObject.connect( self.grid, SIGNAL( "triggered()" ), self.doGrid )
      self.menu.addAction(self.grid)

    self.translate = QAction( QIcon(":/icons/translate.png"), QCoreApplication.translate( "GdalTools", "Translate" ), self.iface.mainWindow() )
    self.translate.setStatusTip( QCoreApplication.translate( "GdalTools", "Converts raster data between different formats") )
    QObject.connect( self.translate, SIGNAL( "triggered()" ), self.doTranslate )
    self.menu.addAction(self.translate)

    self.info = QAction( QIcon( ":/icons/raster-info.png" ), QCoreApplication.translate( "GdalTools", "Information" ), self.iface.mainWindow() )
    self.info.setStatusTip( QCoreApplication.translate( "GdalTools", "Lists information about raster dataset" ) )
    QObject.connect( self.info, SIGNAL("triggered()"), self.doInfo )
    self.menu.addAction( self.info )

    self.projection = QAction( QIcon( ":icons/projection-add.png" ), QCoreApplication.translate( "GdalTools", "Assign projection" ), self.iface.mainWindow() )
    self.projection.setStatusTip( QCoreApplication.translate( "GdalTools", "Add projection info to the raster" ) )
    QObject.connect( self.projection, SIGNAL( "triggered()" ), self.doProjection )
    self.menu.addAction( self.projection )

    self.overview = QAction( QIcon( ":icons/raster-overview.png" ), QCoreApplication.translate( "GdalTools", "Build overviews" ), self.iface.mainWindow() )
    self.overview.setStatusTip( QCoreApplication.translate( "GdalTools", "Builds or rebuilds overview images" ) )
    QObject.connect( self.overview, SIGNAL( "triggered()" ), self.doOverview )
    self.menu.addAction( self.overview )

    self.clipper = QAction( QIcon( ":icons/raster-clip.png" ), QCoreApplication.translate( "GdalTools", "Clipper" ), self.iface.mainWindow() )
    #self.clipper.setStatusTip( QCoreApplication.translate( "GdalTools", "Converts raster data between different formats") )
    QObject.connect( self.clipper, SIGNAL( "triggered()" ), self.doClipper )
    self.menu.addAction(self.clipper)

    self.paletted = QAction( QIcon( ":icons/raster-paletted.png" ), QCoreApplication.translate( "GdalTools", "RGB to PCT" ), self.iface.mainWindow() )
    self.paletted.setStatusTip( QCoreApplication.translate( "GdalTools", "Convert a 24bit RGB image to 8bit paletted" ) )
    QObject.connect( self.paletted, SIGNAL( "triggered()" ), self.doPaletted )
    self.menu.addAction(self.paletted)

    self.rgb = QAction( QIcon( ":icons/raster-paletted.png" ), QCoreApplication.translate( "GdalTools", "PCT to RGB" ), self.iface.mainWindow() )
    self.rgb.setStatusTip( QCoreApplication.translate( "GdalTools", "Convert an 8bit paletted image to 24bit RGB" ) )
    QObject.connect( self.rgb, SIGNAL( "triggered()" ), self.doRGB )
    self.menu.addAction(self.rgb)

    self.settings = QAction( QCoreApplication.translate( "GdalTools", "GdalTools settings" ), self.iface.mainWindow() )
    self.settings.setStatusTip( QCoreApplication.translate( "GdalTools", "Various settings for Gdal Tools" ) )
    QObject.connect( self.settings, SIGNAL( "triggered()" ), self.doSettings )
    self.menu.addAction( self.settings )

    self.about = QAction( QIcon( ":icons/about.png" ), QCoreApplication.translate( "GdalTools", "About GdalTools" ), self.iface.mainWindow() )
    self.about.setStatusTip( QCoreApplication.translate( "GdalTools", "Displays information about Gdal Tools" ) )
    QObject.connect( self.about, SIGNAL( "triggered()" ), self.doAbout )
    self.menu.addSeparator()
    self.menu.addAction( self.about )

    menu_bar = self.iface.mainWindow().menuBar()
    actions = menu_bar.actions()
    lastAction = actions[ len( actions ) - 1 ]
    menu_bar.insertMenu( lastAction, self.menu )

  def unload( self ):
    pass

  def doBuildVRT( self ):
    d = doBuildVRT.GdalToolsDialog( self.iface )
    d.show_()

  def doContour( self ):
    d = doContour.GdalToolsDialog( self.iface )
    d.show_()

  def doRasterize( self ):
    d = doRasterize.GdalToolsDialog( self.iface )
    d.show_()

  def doPolygonize( self ):
    d = doPolygonize.GdalToolsDialog( self.iface )
    d.show_()

  def doMerge( self ):
    d = doMerge.GdalToolsDialog( self.iface )
    d.show_()

  def doSieve( self ):
    d = doSieve.GdalToolsDialog( self.iface )
    d.show_()

  def doProximity( self ):
    d = doProximity.GdalToolsDialog( self.iface )
    d.show_()

  def doNearBlack( self ):
    d = doNearBlack.GdalToolsDialog( self.iface )
    d.show_()

  def doWarp( self ):
    d = doWarp.GdalToolsDialog( self.iface )
    d.show_()

  def doGrid( self ):
    d = doGrid.GdalToolsDialog( self.iface )
    d.show_()

  def doTranslate( self ):
    d = doTranslate.GdalToolsDialog( self.iface )
    d.show_()

  def doInfo( self ):
    d = doInfo.GdalToolsDialog( self.iface )
    d.show_()

  def doProjection( self ):
    d = doProjection.GdalToolsDialog( self.iface )
    d.show_()

  def doOverview( self ):
    d = doOverview.GdalToolsDialog( self.iface )
    d.show_()

  def doClipper( self ):
    d = doClipper.GdalToolsDialog( self.iface )
    d.show_()

  def doPaletted( self ):
    d = doRgbPct.GdalToolsDialog( self.iface )
    d.show_()

  def doRGB( self ):
    d = doPctRgb.GdalToolsDialog( self.iface )
    d.show_()

  def doSettings( self ):
    d = doSettings.GdalToolsSettingsDialog( self.iface )
    d.exec_()

  def doAbout( self ):
    d = doAbout.GdalToolsAboutDialog( self.iface )
    d.exec_()
