#-----------------------------------------------------------
# 
# fTools
# Copyright (C) 2009  Carson Farmer
# EMAIL: carson.farmer (at) gmail.com
# WEB  : http://www.ftools.ca/fTools.html
#
# A collection of data management and analysis tools for vector data
#
# Geoprocessing functions adapted from 'Geoprocessing Plugin', 
# (C) 2008 by Dr. Horst Duester, Stefan Ziegler
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
import resources
from qgis.core import *
import resources
import os.path, sys
# Set up current path, so that we know where to look for mudules
currentPath = os.path.dirname( __file__ )
sys.path.append( os.path.abspath( os.path.dirname( __file__) + '/tools' ) )
# Multi-function modules
import doGeometry, doGeoprocessing, doVisual
# Single function modules
# TODO: Eliminate the following modules in favour of above multi-function formats
import doIntersectLines, doJoinAttributes, doSelectByLocation, doVectorSplit, doMeanCoords
import doPointDistance, doPointsInPolygon, doRandom, doRandPoints, doRegPoints, doDefineProj
import doReProject, doSpatialJoin, doSubsetSelect, doSumLines, doVectorGrid, doAbout

class fToolsPlugin:
	def __init__( self,iface ):
		self.iface = iface
		try:
			self.QgisVersion = unicode( QGis.QGIS_VERSION_INT )
		except:
			self.QgisVersion = unicode( QGis.qgisVersion )[ 0 ]
			
	def getThemeIcon( self, icon ):
		settings = QSettings()
		pluginPath = QString( os.path.dirname( __file__ ) )
		themePath = QString( "icons" ) + QDir.separator() + QString( settings.value( "/Themes" ).toString() ) + QDir.separator() + QString( icon)
		defaultPath = QString( "icons" ) + QDir.separator() + QString( "default" ) + QDir.separator() + QString( icon )
		if QFile.exists( pluginPath + QDir.separator() + themePath ):
			return QIcon( ":" + themePath )
		elif QFile.exists( pluginPath + QDir.separator() + defaultPath ):
			return QIcon( ":" + defaultPath )
		else:
			return QIcon()

	def initGui( self ):
		if int( self.QgisVersion ) < 1:
			QMessageBox.warning( self.iface.getMainWindow(), "fTools", 
			QCoreApplication.translate( "fTools", "Quantum GIS version detected: " ) +unicode( self.QgisVersion )+".xx\n"
			+ QCoreApplication.translate( "fTools", "This version of fTools requires at least QGIS version 1.0.0\nPlugin will not be enabled." ) )
			return None
		
		self.menu = QMenu()
		self.menu.setTitle( QCoreApplication.translate( "fTools", "&Tools" ) )
		self.analysisMenu = QMenu( QCoreApplication.translate( "fTools", "&Analysis Tools" ) )
		self.analysisMenu.setIcon( QIcon( self.getThemeIcon( "analysis.png" ) ) )
		self.distMatrix = QAction( QIcon( self.getThemeIcon( "matrix.png" ) ), 
		QCoreApplication.translate( "fTools", "Distance matrix" ),self.iface.mainWindow( ) )
		self.sumLines = QAction( QIcon( self.getThemeIcon( "sum_lines.png" ) ), 
		QCoreApplication.translate( "fTools", "Sum line lengths" ), self.iface.mainWindow() )
		self.pointsPoly = QAction( QIcon( self.getThemeIcon( "sum_points.png" ) ), 
		QCoreApplication.translate( "fTools", "Points in polygon" ),self.iface.mainWindow() )
		self.compStats = QAction( QIcon( self.getThemeIcon( "basic_statistics.png" ) ), 
		QCoreApplication.translate( "fTools",  "Basic statistics" ),self.iface.mainWindow() )
		self.listUnique = QAction( QIcon( self.getThemeIcon( "unique.png" ) ), 
		QCoreApplication.translate( "fTools", "List unique values" ),self.iface.mainWindow() )
		self.nearestNeigh = QAction( QIcon( self.getThemeIcon( "neighbour.png" ) ), 
		QCoreApplication.translate( "fTools", "Nearest neighbour analysis" ),self.iface.mainWindow() )
		self.meanCoords = QAction( QIcon( self.getThemeIcon( "mean.png" ) ), 
		QCoreApplication.translate( "fTools", "Mean coordinate(s)" ),self.iface.mainWindow() )
		self.intLines = QAction( QIcon( self.getThemeIcon( "intersections.png" ) ), 
		QCoreApplication.translate( "fTools", "Line intersections" ) ,self.iface.mainWindow() )
		self.analysisMenu.addActions( [ self.distMatrix, self.sumLines, self.pointsPoly,
		self.listUnique, self.compStats, self.nearestNeigh, self.meanCoords, self.intLines ] )

		self.samplingMenu = QMenu( QCoreApplication.translate( "fTools", "&Sampling Tools" ) )
		self.samplingMenu.setIcon( QIcon( self.getThemeIcon( "sampling.png" ) ) )
		self.randSel = QAction( QIcon( self.getThemeIcon( "random_selection.png" ) ), 
		QCoreApplication.translate( "fTools", "Random selection" ),self.iface.mainWindow() )
		self.randSub = QAction( QIcon( self.getThemeIcon( "sub_selection.png" ) ), 
		QCoreApplication.translate( "fTools", "Random selection within subsets" ),self.iface.mainWindow() )
		self.randPoints = QAction( QIcon( self.getThemeIcon( "random_points.png" ) ), 
		QCoreApplication.translate( "fTools", "Random points" ),self.iface.mainWindow() )
		self.regPoints = QAction( QIcon( self.getThemeIcon( "regular_points.png" ) ), 
		QCoreApplication.translate( "fTools", "Regular points" ), self.iface.mainWindow() )
		self.vectGrid = QAction( QIcon( self.getThemeIcon( "vector_grid.png" ) ), 
		QCoreApplication.translate( "fTools", "Vector grid" ), self.iface.mainWindow() )
		self.selectLocation = QAction( QIcon( self.getThemeIcon( "select_location.png" ) ), 
		QCoreApplication.translate( "fTools", "Select by location" ), self.iface.mainWindow() )
		self.samplingMenu.addActions( [ self.randSel, self.randSub, self.randPoints, 
		self.regPoints, self.vectGrid, self.selectLocation ] )

		self.geoMenu = QMenu( QCoreApplication.translate( "fTools", "&Geoprocessing Tools" ) )
		self.geoMenu.setIcon( QIcon( self.getThemeIcon( "geoprocessing.png" ) ) )
		self.minConvex = QAction( QIcon( self.getThemeIcon( "convex_hull.png" ) ), 
		QCoreApplication.translate( "fTools", "Convex hull(s)" ),self.iface.mainWindow() )
		self.dynaBuffer = QAction( QIcon( self.getThemeIcon( "buffer.png" ) ), 
		QCoreApplication.translate( "fTools", "Buffer(s)" ),self.iface.mainWindow() )
		self.intersect = QAction( QIcon( self.getThemeIcon( "intersect.png" ) ), 
		QCoreApplication.translate( "fTools", "Intersect" ),self.iface.mainWindow() )
		self.union = QAction( QIcon( self.getThemeIcon( "union.png" ) ), 
		QCoreApplication.translate( "fTools", "Union" ),self.iface.mainWindow() )
		self.symDifference = QAction( QIcon( self.getThemeIcon( "sym_difference.png" ) ), 
		QCoreApplication.translate( "fTools", "Symetrical difference" ),self.iface.mainWindow() )
		self.clip = QAction( QIcon( self.getThemeIcon( "clip.png" ) ), 
		QCoreApplication.translate( "fTools", "Clip" ),self.iface.mainWindow() )
		self.dissolve = QAction( QIcon( self.getThemeIcon( "dissolve.png" ) ), 
		QCoreApplication.translate( "fTools", "Dissolve" ),self.iface.mainWindow() )
		self.erase = QAction( QIcon( self.getThemeIcon( "difference.png" ) ), 
		QCoreApplication.translate( "fTools", "Difference" ),self.iface.mainWindow() )
		self.geoMenu.addActions( [ self.minConvex, self.dynaBuffer, self.intersect, 
		self.union, self.symDifference, self.clip, self.erase, self.dissolve ] )

		self.conversionMenu = QMenu( QCoreApplication.translate( "fTools", "G&eometry Tools" ) )
		self.conversionMenu.setIcon( QIcon( self.getThemeIcon( "geometry.png" ) ) )
		self.compGeo = QAction( QIcon( self.getThemeIcon( "export_geometry.png") ), 
		QCoreApplication.translate( "fTools", "Export/Add geometry columns" ),self.iface.mainWindow() )
		self.checkGeom = QAction( QIcon( self.getThemeIcon( "check_geometry.png") ), 
		QCoreApplication.translate( "fTools", "Check geometry validity" ),self.iface.mainWindow() )
		self.centroids = QAction( QIcon( self.getThemeIcon( "centroids.png") ),  
		QCoreApplication.translate( "fTools", "Polygon centroids" ),self.iface.mainWindow() )
		self.extNodes = QAction( QIcon( self.getThemeIcon( "extract_nodes.png") ),  
		QCoreApplication.translate( "fTools", "Extract nodes" ),self.iface.mainWindow() )
		self.simplify = QAction( QIcon( self.getThemeIcon( "simplify.png") ),  
		QCoreApplication.translate( "fTools", "Simplify geometries" ),self.iface.mainWindow() )
		self.multiToSingle = QAction( QIcon( self.getThemeIcon( "multi_to_single.png") ),  
		QCoreApplication.translate( "fTools", "Multipart to singleparts" ),self.iface.mainWindow() )
		self.singleToMulti = QAction( QIcon( self.getThemeIcon( "single_to_multi.png") ),  
		QCoreApplication.translate( "fTools", "Singleparts to multipart" ),self.iface.mainWindow() )
		self.polysToLines = QAction( QIcon( self.getThemeIcon( "to_lines.png") ),  
		QCoreApplication.translate( "fTools", "Polygons to lines" ),self.iface.mainWindow() )
		self.conversionMenu.addActions( [ self.checkGeom, self.compGeo, self.centroids, self.simplify, 
		self.multiToSingle, self.singleToMulti, self.polysToLines, self.extNodes ] )

		self.dataManageMenu = QMenu( QCoreApplication.translate( "fTools", "&Data Management Tools") )
		self.dataManageMenu.setIcon( QIcon( self.getThemeIcon( "management.png") ) )
		self.project = QAction( QIcon( self.getThemeIcon( "export_projection.png") ),  
		QCoreApplication.translate( "fTools", "Export to new projection" ), self.iface.mainWindow() )
		self.define = QAction( QIcon( self.getThemeIcon( "define_projection.png" ) ),  
		QCoreApplication.translate( "fTools", "Define current projection" ), self.iface.mainWindow() )
		self.joinAttr = QAction( QIcon( self.getThemeIcon( "join_attributes.png" ) ),  
		QCoreApplication.translate( "fTools", "Join attributes" ), self.iface.mainWindow() )
		self.spatJoin = QAction( QIcon( self.getThemeIcon( "join_location.png" ) ),  
		QCoreApplication.translate( "fTools", "Join attributes by location" ), self.iface.mainWindow() )
		self.splitVect = QAction( QIcon( self.getThemeIcon( "split_layer.png" ) ),  
		QCoreApplication.translate( "fTools", "Split vector layer" ), self.iface.mainWindow() )
		self.dataManageMenu.addActions( [ self.project, self.define, self.joinAttr, self.spatJoin, self.splitVect ] )

		self.about_ftools = QAction( QIcon( self.getThemeIcon( "ftools_logo.png" ) ), 
		QCoreApplication.translate( "fTools", "About fTools" ), self.iface.mainWindow() )

		self.menu.addMenu( self.analysisMenu )
		self.menu.addMenu( self.samplingMenu )
		self.menu.addMenu( self.geoMenu )
		self.menu.addMenu( self.conversionMenu )
		self.menu.addMenu( self.dataManageMenu )
		self.menu.addSeparator()
		self.menu.addAction( self.about_ftools )

		menuBar = self.iface.mainWindow().menuBar()
		actions = menuBar.actions()
		helpAction = actions[ len( actions ) - 1 ]
		menuBar.insertMenu( helpAction, self.menu )
        
		QObject.connect( self.distMatrix, SIGNAL("triggered()"), self.dodistMatrix )
		QObject.connect( self.sumLines, SIGNAL("triggered()"), self.dosumLines )
		QObject.connect( self.pointsPoly, SIGNAL("triggered()"), self.dopointsPoly )
		QObject.connect( self.compStats, SIGNAL("triggered()"), self.docompStats )
		QObject.connect( self.listUnique, SIGNAL("triggered()"), self.dolistUnique )
		QObject.connect( self.nearestNeigh, SIGNAL("triggered()"), self.donearestNeigh )
		QObject.connect( self.meanCoords, SIGNAL("triggered()"), self.domeanCoords )
		QObject.connect( self.intLines, SIGNAL("triggered()"), self.dointLines )

		QObject.connect( self.randSel, SIGNAL("triggered()"), self.dorandSel )
		QObject.connect( self.randSub, SIGNAL("triggered()"), self.dorandSub )
		QObject.connect( self.randPoints, SIGNAL("triggered()"), self.dorandPoints )
		QObject.connect( self.regPoints, SIGNAL("triggered()"), self.doregPoints )
		QObject.connect( self.vectGrid, SIGNAL("triggered()"), self.dovectGrid )
		QObject.connect( self.selectLocation, SIGNAL("triggered()"), self.doselectLocation )

		QObject.connect( self.minConvex, SIGNAL("triggered()"), self.dominConvex )
		QObject.connect( self.intersect, SIGNAL("triggered()"), self.dointersect )
		QObject.connect( self.dissolve, SIGNAL("triggered()"), self.dodissolve )
		QObject.connect( self.symDifference, SIGNAL("triggered()"), self.dosymdifference )
		QObject.connect( self.erase, SIGNAL("triggered()"), self.doerase )
		QObject.connect( self.union, SIGNAL("triggered()"), self.dounion )
		QObject.connect( self.clip, SIGNAL("triggered()"), self.doclip )
		QObject.connect( self.dynaBuffer, SIGNAL("triggered()"), self.dodynaBuffer )

		QObject.connect( self.multiToSingle, SIGNAL("triggered()"), self.domultiToSingle )
		QObject.connect( self.singleToMulti, SIGNAL("triggered()"), self.dosingleToMulti )
		QObject.connect( self.checkGeom, SIGNAL("triggered()"), self.docheckGeom )
		QObject.connect( self.simplify, SIGNAL("triggered()"), self.dosimplify )
		QObject.connect( self.centroids, SIGNAL("triggered()"), self.docentroids )
		QObject.connect( self.polysToLines, SIGNAL("triggered()"), self.dopolysToLines )
		QObject.connect( self.compGeo, SIGNAL("triggered()"), self.docompGeo )
		QObject.connect( self.extNodes, SIGNAL("triggered()"), self.doextNodes )

		QObject.connect( self.project, SIGNAL("triggered()"), self.doproject )
		QObject.connect( self.define, SIGNAL("triggered()"), self.dodefine )
		QObject.connect( self.joinAttr, SIGNAL("triggered()"), self.dojoinAttr )
		QObject.connect( self.spatJoin, SIGNAL("triggered()"), self.dospatJoin )
		QObject.connect( self.splitVect, SIGNAL("triggered()"), self.dosplitVect )

		QObject.connect( self.about_ftools, SIGNAL("triggered()"), self.doabout )

	def unload( self ):
		pass

	def dosimplify( self ):
		d = doGeometry.GeometryDialog( self.iface, 6 )
		d.exec_()

	def dopolysToLines( self ):
		d = doGeometry.GeometryDialog( self.iface, 4 )
		d.exec_()

	def docheckGeom( self ):
		d = doVisual.VisualDialog( self.iface, 1 )
		d.exec_()

	def domultiToSingle( self ):
		d = doGeometry.GeometryDialog( self.iface, 2 )
		d.exec_()

	def dosingleToMulti( self ):
		d = doGeometry.GeometryDialog( self.iface, 1 )
		d.exec_()

	def doselectLocation( self ):
		d = doSelectByLocation.Dialog( self.iface )
		d.exec_()

	def domeanCoords( self ):
		d = doMeanCoords.Dialog( self.iface, 1 )
		d.exec_()

	def dominConvex( self):
		d = doGeoprocessing.GeoprocessingDialog( self.iface, 2 )
		d.exec_()

	def dodynaBuffer( self):
		d = doGeoprocessing.GeoprocessingDialog( self.iface, 1 )
		d.exec_()

	def dointersect( self):
		d = doGeoprocessing.GeoprocessingDialog( self.iface, 5 )
		d.exec_()

	def dodissolve( self):
		d = doGeoprocessing.GeoprocessingDialog( self.iface, 4 )
		d.exec_()

	def doerase( self):
		d = doGeoprocessing.GeoprocessingDialog( self.iface, 3 )
		d.exec_()

	def dosymdifference( self):
		d = doGeoprocessing.GeoprocessingDialog( self.iface, 7 )
		d.exec_()

	def dounion( self):
		d = doGeoprocessing.GeoprocessingDialog( self.iface, 6 )
		d.exec_()

	def doclip( self):
		d = doGeoprocessing.GeoprocessingDialog( self.iface, 8 )
		d.exec_()

	def donearestNeigh( self ):
		d = doVisual.VisualDialog( self.iface, 4 )
		d.exec_()

	def dodistMatrix( self ):
		d = doPointDistance.Dialog( self.iface )
		d.exec_()

	def docentroids( self ):
		d = doGeometry.GeometryDialog( self.iface, 7 )
		d.exec_()

	def dosumLines(self):
		d = doSumLines.Dialog(self.iface)
		d.exec_()

	def dopointsPoly( self ):
		d = doPointsInPolygon.Dialog( self.iface )
		d.exec_()

	def dorandSel( self ):
		d = doRandom.Dialog( self.iface )
		d.exec_()

	def dorandSub( self ):
		d = doSubsetSelect.Dialog( self.iface )
		d.exec_()

	def dorandPoints( self ):
		d = doRandPoints.Dialog( self.iface )
		d.exec_()

	def doregPoints( self ):
		d = doRegPoints.Dialog( self.iface )
		d.exec_()

	def dovectGrid( self ):
		d = doVectorGrid.Dialog( self.iface )
		d.exec_()

	def doextNodes( self ):
		d = doGeometry.GeometryDialog( self.iface, 3 )
		d.exec_()

	def dointLines( self ):
		d = doIntersectLines.Dialog( self.iface )
		d.exec_()

	def dosplitVect( self ):
		d = doVectorSplit.Dialog( self.iface )
		d.exec_()

	def docompGeo( self ):
		d = doGeometry.GeometryDialog( self.iface, 5 )
		d.exec_()

	def dolistUnique( self ):
		d = doVisual.VisualDialog( self.iface, 2 )
		d.exec_()

	def docompStats( self ):
		d = doVisual.VisualDialog( self.iface, 3 )
		d.exec_()

	def doproject( self ):
		d = doReProject.Dialog( self.iface )
		d.exec_()

	def dodefine( self ):
		d = doDefineProj.Dialog( self.iface )
		d.exec_()

	def dojoinAttr( self ):
		d = doJoinAttributes.Dialog( self.iface )
		d.exec_()

	def dospatJoin( self ):
		d = doSpatialJoin.Dialog( self.iface )
		d.exec_()

	def doabout( self ):
		d = doAbout.Dialog( self.iface )
		d.exec_()
