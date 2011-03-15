# -*- coding: utf-8 -*-
"""
/***************************************************************************
Name                  : GdalTools
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

# Import required modules and if missing show the right module's name
req_mods = { "osgeo": "osgeo [python-gdal]" }

try:
  from osgeo import gdal
  from osgeo import ogr
except ImportError, e:
  error_str = e.args[0]
  error_mod = error_str.replace( "No module named ", "" )
  if req_mods.has_key( error_mod ):
    error_str = error_str.replace( error_mod, req_mods[error_mod] )
  raise ImportError( error_str )


class GdalTools:

  def __init__( self, iface ):
    # Save reference to the QGIS interface
    self.iface = iface
    try:
      self.QgisVersion = unicode( QGis.QGIS_VERSION_INT )
    except:
      self.QgisVersion = unicode( QGis.qgisVersion )[ 0 ]

    if QGis.QGIS_VERSION[0:3] < "1.5":
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

    from tools.GdalTools_utils import Version, GdalConfig, LayerRegistry
    self.GdalVersion = Version( GdalConfig.version() )
    LayerRegistry.setIface( self.iface )

    # find the Raster menu
    rasterMenu = None
    menu_bar = self.iface.mainWindow().menuBar()
    actions = menu_bar.actions()

    rasterText = QCoreApplication.translate( "QgisApp", "&Raster" )

    for a in actions:
        if a.menu() != None and a.menu().title() == rasterText:
            rasterMenu = a.menu()
            break

    if rasterMenu == None:
        # no Raster menu, create and insert it before the Help menu
        self.menu = QMenu()
        self.menu.setTitle( QCoreApplication.translate( "GdalTools", "&Raster" ) )
        lastAction = actions[ len( actions ) - 1 ]
        menu_bar.insertMenu( lastAction, self.menu )
    else:
        self.menu = rasterMenu
        self.menu.addSeparator()

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

    self.tileindex = QAction( QIcon( ":icons/tileindex.png" ), QCoreApplication.translate( "GdalTools", "Tile index" ), self.iface.mainWindow() )
    self.tileindex.setStatusTip( QCoreApplication.translate( "GdalTools", "Build a shapefile as a raster tileindex" ) )
    QObject.connect( self.tileindex, SIGNAL( "triggered()" ), self.doTileIndex )
    self.menu.addAction(self.tileindex)

    if self.GdalVersion >= "1.7":
      self.dem = QAction( QIcon( ":icons/dem.png" ), QCoreApplication.translate( "GdalTools", "DEM" ), self.iface.mainWindow() )
      self.dem.setStatusTip( QCoreApplication.translate( "GdalTools", "Tool to analyze and visualize DEMs" ) )
      QObject.connect( self.dem, SIGNAL( "triggered()" ), self.doDEM )
      self.menu.addAction(self.dem)

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
    from tools.doBuildVRT import GdalToolsDialog as BuildVRT
    d = BuildVRT( self.iface )
    d.show_()

  def doContour( self ):
    from tools.doContour import GdalToolsDialog as Contour
    d = Contour( self.iface )
    d.show_()

  def doRasterize( self ):
    from tools.doRasterize import GdalToolsDialog as Rasterize
    d = Rasterize( self.iface )
    d.show_()

  def doPolygonize( self ):
    from tools.doPolygonize import GdalToolsDialog as Polygonize
    d = Polygonize( self.iface )
    d.show_()

  def doMerge( self ):
    from tools.doMerge import GdalToolsDialog as Merge
    d = Merge( self.iface )
    d.show_()

  def doSieve( self ):
    from tools.doSieve import GdalToolsDialog as Sieve
    d = Sieve( self.iface )
    d.show_()

  def doProximity( self ):
    from tools.doProximity import GdalToolsDialog as Proximity
    d = Proximity( self.iface )
    d.show_()

  def doNearBlack( self ):
    from tools.doNearBlack import GdalToolsDialog as NearBlack
    d = NearBlack( self.iface )
    d.show_()

  def doWarp( self ):
    from tools.doWarp import GdalToolsDialog as Warp
    d = Warp( self.iface )
    d.show_()

  def doGrid( self ):
    from tools.doGrid import GdalToolsDialog as Grid
    d = Grid( self.iface )
    d.show_()

  def doTranslate( self ):
    from tools.doTranslate import GdalToolsDialog as Translate
    d = Translate( self.iface )
    d.show_()

  def doInfo( self ):
    from tools.doInfo import GdalToolsDialog as Info
    d = Info( self.iface )
    d.show_()

  def doProjection( self ):
    from tools.doProjection import GdalToolsDialog as Projection
    d = Projection( self.iface )
    d.show_()

  def doOverview( self ):
    from tools.doOverview import GdalToolsDialog as Overview
    d = Overview( self.iface )
    d.show_()

  def doClipper( self ):
    from tools.doClipper import GdalToolsDialog as Clipper
    d = Clipper( self.iface )
    d.show_()

  def doPaletted( self ):
    from tools.doRgbPct import GdalToolsDialog as RgbPct
    d = RgbPct( self.iface )
    d.show_()

  def doRGB( self ):
    from tools.doPctRgb import GdalToolsDialog as PctRgb
    d = PctRgb( self.iface )
    d.show_()

  def doTileIndex( self ):
    from tools.doTileIndex import GdalToolsDialog as TileIndex
    d = TileIndex( self.iface )
    d.show_()

  def doDEM( self ):
    from tools.doDEM import GdalToolsDialog as DEM
    d = DEM( self.iface )
    d.show_()

  def doSettings( self ):
    from tools.doSettings import GdalToolsSettingsDialog as Settings
    d = Settings( self.iface )
    d.exec_()

  def doAbout( self ):
    from tools.doAbout import GdalToolsAboutDialog as About
    d = About( self.iface )
    d.exec_()
