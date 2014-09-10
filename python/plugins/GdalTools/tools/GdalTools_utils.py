# -*- coding: utf-8 -*-

"""
***************************************************************************
    GdalTools_utils.py
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

# Utility functions
# -------------------------------------------------
# getLastUsedDir()
# setLastUsedDir( QString *file_or_dir path )
# -------------------------------------------------

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *
from qgis.gui import *

from osgeo import gdal, ogr, osr
from osgeo.gdalconst import *

import os
# to know the os
import platform
import sys
import string
import re

# Escapes arguments and return them joined in a string
def escapeAndJoin(strList):
    joined = ''
    for s in strList:
      if s.find(" ") is not -1:
        escaped = '"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"'
      else:
        escaped = s
      joined += escaped + " "
    return joined.strip()

# Retrieves last used dir from persistent settings
def getLastUsedDir():
    settings = QSettings()
    lastProjectDir = settings.value( "/UI/lastProjectDir", u".", type=unicode )
    return settings.value( "/GdalTools/lastUsedDir", lastProjectDir, type=unicode )

# Stores last used dir in persistent settings
def setLastUsedDir(filePath):
    settings = QSettings()
    fileInfo = QFileInfo(filePath)
    if fileInfo.isDir():
      dirPath = fileInfo.filePath()
    else:
      dirPath = fileInfo.path()
    settings.setValue( "/GdalTools/lastUsedDir", dirPath )

# Retrieves GDAL binaries location
def getGdalBinPath():
  settings = QSettings()
  return settings.value( "/GdalTools/gdalPath", u"", type=unicode )

# Stores GDAL binaries location
def setGdalBinPath( path ):
  settings = QSettings()
  settings.setValue( "/GdalTools/gdalPath", path )

# Retrieves GDAL python modules location
def getGdalPymodPath():
  settings = QSettings()
  return settings.value( "/GdalTools/gdalPymodPath", u"", type=unicode )

# Stores GDAL python modules location
def setGdalPymodPath( path ):
  settings = QSettings()
  settings.setValue( "/GdalTools/gdalPymodPath", path )

# Retrieves GDAL help files location
def getHelpPath():
  settings = QSettings()
  return settings.value( "/GdalTools/helpPath", u"", type=unicode )

# Stores GDAL help files location
def setHelpPath( path ):
  settings = QSettings()
  settings.setValue( "/GdalTools/helpPath", path )

# Retrieves last used encoding from persistent settings
def getLastUsedEncoding():
    settings = QSettings()
    return settings.value( "/UI/encoding", u"System", type=unicode )

# Stores last used encoding in persistent settings
def setLastUsedEncoding(encoding):
    settings = QSettings()
    settings.setValue( "/UI/encoding", encoding)

def getRasterExtensions():
  formats = FileFilter.allRastersFilter().split( ";;" )
  extensions = []
  for f in formats:
    if string.find(f, "*.bt" ) is not -1 or string.find(f, "*.mpr" ) is not -1:   # Binary Terrain or ILWIS
      continue
    extensions.extend( FileFilter.getFilterExtensions( f ) )
  return extensions

def getVectorExtensions():
  formats = FileFilter.allVectorsFilter().split( ";;" )
  extensions = []
  for f in formats:
    extensions.extend( FileFilter.getFilterExtensions( f ) )
  return extensions

class LayerRegistry(QObject):

    _instance = None
    _iface = None

    @staticmethod
    def instance():
      if LayerRegistry._instance == None:
        LayerRegistry._instance = LayerRegistry()
      return LayerRegistry._instance

    @staticmethod
    def setIface(iface):
      LayerRegistry._iface = iface

    layers = []

    def __init__(self):
      QObject.__init__(self)
      if LayerRegistry._instance != None:
        return

      LayerRegistry.layers = self.getAllLayers()
      LayerRegistry._instance = self
      self.connect(QgsMapLayerRegistry.instance(), SIGNAL("removeAll()"), self.removeAllLayers)
      self.connect(QgsMapLayerRegistry.instance(), SIGNAL("layerWasAdded(QgsMapLayer *)"), self.layerAdded)
      self.connect(QgsMapLayerRegistry.instance(), SIGNAL("layerWillBeRemoved(QString)"), self.removeLayer)

    def getAllLayers(self):
       if LayerRegistry._iface and hasattr(LayerRegistry._iface, 'legendInterface'):
         return LayerRegistry._iface.legendInterface().layers()
       return QgsMapLayerRegistry.instance().mapLayers().values()

    def layerAdded(self, layer):
       LayerRegistry.layers.append( layer )
       self.emit( SIGNAL( "layersChanged" ) )

    def removeLayer(self, layerId):
       LayerRegistry.layers = filter( lambda x: x.id() != layerId, LayerRegistry.layers)
       self.emit( SIGNAL( "layersChanged" ) )

    def removeAllLayers(self):
       LayerRegistry.layers = []
       self.emit( SIGNAL( "layersChanged" ) )

    @classmethod
    def isRaster(self, layer):
      # only gdal raster layers
      if layer.type() != layer.RasterLayer:
        return False
      if layer.providerType() != 'gdal':
        return False
      return True

    def getRasterLayers(self):
      return filter( self.isRaster, LayerRegistry.layers )

    @classmethod
    def isVector(self, layer):
      if layer.type() != layer.VectorLayer:
        return False
      if layer.providerType() != 'ogr':
        return False
      return True

    def getVectorLayers(self):
      return filter( self.isVector, LayerRegistry.layers )

def getRasterFiles(path, recursive=False):
  rasters = []
  if not QFileInfo(path).exists():
    return rasters

  # TODO remove *.aux.xml
  _filter = getRasterExtensions()
  workDir = QDir( path )
  workDir.setFilter( QDir.Files | QDir.NoSymLinks | QDir.NoDotAndDotDot )
  workDir.setNameFilters( _filter )
  files = workDir.entryList()
  for f in files:
    rasters.append( path + "/" + f )

  if recursive:
    for myRoot, myDirs, myFiles in os.walk( unicode(path) ):
      for dir in myDirs:
        workDir = QDir( myRoot + "/" + dir )
        workDir.setFilter( QDir.Files | QDir.NoSymLinks | QDir.NoDotAndDotDot )
        workDir.setNameFilters( _filter )
        workFiles = workDir.entryList()
        for f in workFiles:
          rasters.append( myRoot + "/" + dir + "/" + f )

  return rasters

def fillRasterOutputFormat(aFilter = None, filename = None):
  shortName = ''

  if aFilter != None:
    supportedRasters = GdalConfig.getSupportedRasters()
    filterName = re.sub('^.*\] ', '', FileFilter.getFilterName( aFilter ))
    if supportedRasters.has_key( filterName ):
      return supportedRasters[ filterName ][ "SHORTNAME" ]

    shortName = GdalConfig.SupportedRasters.long2ShortName(filterName)

  if shortName == '' and filename != None:
    shortName = GdalConfig.SupportedRasters.filename2ShortName(filename)

  if shortName == '':
    shortName = "GTiff"

  return shortName

def fillVectorOutputFormat(aFilter = None, filename = None):
  shortName = ''

  if aFilter != None:
    supportedVectors = GdalConfig.getSupportedVectors()
    filterName = re.sub('^.*\] ', '', FileFilter.getFilterName( aFilter ))
    if supportedVectors.has_key( filterName ):
      return supportedVectors[ filterName ][ "SHORTNAME" ]

    pass #shortName = GdalConfig.SupportedVectors.long2ShortName(filterName)

  if shortName == '' and filename != None:
    pass #shortName = GdalConfig.SupportedVectors.filename2ShortName(filename)

  if shortName == '':
    shortName = "ESRI Shapefile"

  return shortName

class UnsupportedOGRFormat(Exception):
    def __init__(self):
      msg = QCoreApplication.translate( "GdalTools", "The selected file is not a supported OGR format" )
      Exception.__init__(self, msg)

def getVectorFields(vectorFile):
    hds = ogr.Open( unicode(vectorFile).encode('utf8') )
    if hds == None:
      raise UnsupportedOGRFormat()

    fields = []
    names = []

    layer = hds.GetLayer(0)
    defn = layer.GetLayerDefn()

    for i in range(defn.GetFieldCount()):
      fieldDefn = defn.GetFieldDefn(i)
      fieldType = fieldDefn.GetType()
      if fieldType == 0 or fieldType == 2:
        fields.append(fieldDefn)
        names.append(fieldDefn.GetName())

    return (fields, names)

# get raster SRS if possible
def getRasterSRS( parent, fileName ):
    ds = gdal.Open(fileName)
    if ds is None:
        return ''

    proj = ds.GetProjectionRef()
    if proj is None:
      return ''

    sr = osr.SpatialReference()
    if sr.ImportFromWkt(proj) != gdal.CE_None:
        return ''

    name = sr.GetAuthorityName(None)
    code = sr.GetAuthorityCode(None)
    if name is not None and code is not None:
        return '%s:%s' % (name,code)

    return ''

# get raster extent using python API - replaces old method which parsed gdalinfo output
def getRasterExtent(parent, fileName):
    ds = gdal.Open(fileName)
    if ds is None:
        return

    x = ds.RasterXSize
    y = ds.RasterYSize

    gt = ds.GetGeoTransform()
    if gt is None:
        xUL = 0
        yUL = 0
        xLR = x
        yLR = y
    else:
        xUL = gt[0]
        yUL = gt[3]
        xLR = gt[0] + gt[1]*x + gt[2]*y
        yLR = gt[3] + gt[4]*x + gt[5]*y

    return QgsRectangle( xUL, yLR, xLR, yUL )


# This class is used to replace the QFileDialog class.
# Its static methods are used in place of the respective QFileDialog ones to:
# 1. set the last used directory
# 2. append the selected extension to the file name
# 3. bypass the following bug:
# when you use the 'filter' argument, the dialog is enlarged up to the longest filter length,
# so sometimes the dialog excedes the screen width
class FileDialog:
  @classmethod
  def getDialog(self, parent = None, caption = '', acceptMode = QFileDialog.AcceptOpen, fileMode = QFileDialog.ExistingFile, filter = '', selectedFilter = None, useEncoding = False):
    if useEncoding:
      dialog = QgsEncodingFileDialog(parent, caption, getLastUsedDir(), filter, getLastUsedEncoding())
    else:
      dialog = QFileDialog(parent, caption, getLastUsedDir(), filter)
    dialog.setFileMode(fileMode)
    dialog.setAcceptMode(acceptMode)

    if selectedFilter is not None:
      dialog.selectNameFilter(selectedFilter[0])

    if not dialog.exec_():
      if useEncoding:
        return ('', None)
      return ''

    # change the selected filter value
    if selectedFilter is not None:
      selectedFilter[0] = dialog.selectedNameFilter()

    # save the last used dir and return the selected files
    files = dialog.selectedFiles()
    if files != '':
      setLastUsedDir(files[0])

      if fileMode != QFileDialog.ExistingFiles:
        files = files[0]
        # append the extension if not already present
        if fileMode == QFileDialog.AnyFile:
          firstExt = None
          for ext in FileFilter.getFilterExtensions(dialog.selectedNameFilter()):
            if FileFilter.filenameMatchesFilterExt(files, ext):
              firstExt = None
              break

            if firstExt == None:
              firstExt = ext

          if firstExt != None:
            if firstExt.startswith('*'):
              files += firstExt[1:]

    if useEncoding:
      encoding = dialog.encoding()
      # encoding setted yet by QgsEncodingFileDialog
      #setLastUsedEncoding(encoding)
      return (files, encoding)

    return files

  @classmethod
  def getOpenFileNames(self, parent = None, caption = '', filter = '', selectedFilter = None, useEncoding = False):
    return self.getDialog(parent, caption, QFileDialog.AcceptOpen, QFileDialog.ExistingFiles, filter, selectedFilter, useEncoding)

  @classmethod
  def getOpenFileName(self, parent = None, caption = '', filter = '', selectedFilter = None, useEncoding = False):
    return self.getDialog(parent, caption, QFileDialog.AcceptOpen, QFileDialog.ExistingFile, filter, selectedFilter, useEncoding)

  @classmethod
  def getSaveFileName(self, parent = None, caption = '', filter = '', selectedFilter = None, useEncoding = False):
    return self.getDialog(parent, caption, QFileDialog.AcceptSave, QFileDialog.AnyFile, filter, selectedFilter, useEncoding)

  @classmethod
  def getExistingDirectory(self, parent = None, caption = '', useEncoding = False):
    return self.getDialog(parent, caption, QFileDialog.AcceptOpen, QFileDialog.DirectoryOnly, '', None, useEncoding)

class FileFilter:
  @classmethod
  def getFilter(self, typeName):
      settings = QSettings()
      return settings.value( "/GdalTools/" + typeName + "FileFilter", u"", type=unicode )

  @classmethod
  def setFilter(self, typeName, aFilter):
      settings = QSettings()
      settings.setValue( "/GdalTools/" + typeName + "FileFilter", aFilter )

  # stores the supported raster file filter
  rastersFilter = ''

  # Retrieves the filter for supported raster files
  @classmethod
  def allRastersFilter(self):
    if self.rastersFilter == '':
      self.rastersFilter = QgsProviderRegistry.instance().fileRasterFilters()

      # workaround for QGis < 1.5 (see #2376)
      # removed as this is a core plugin QGis >= 1.9

    return self.rastersFilter

  # Retrieves the last used filter for raster files
  # Note: filter string is in a list
  @classmethod
  def lastUsedRasterFilter(self):
     return [self.getFilter("lastRaster")]

  @classmethod
  def setLastUsedRasterFilter(self, aFilter):
     self.setFilter("lastRaster", aFilter[0])

  # stores the supported vectors file filter
  vectorsFilter = ''

  # Retrieves the filter for supported vector files
  @classmethod
  def allVectorsFilter(self):
    if self.vectorsFilter == '':
      self.vectorsFilter = QgsProviderRegistry.instance().fileVectorFilters()
    return self.vectorsFilter

  # Retrieves the last used filter for vector files
  # Note: filter string is in a list
  @classmethod
  def lastUsedVectorFilter(self):
     return [self.getFilter("lastVector")]

  @classmethod
  def setLastUsedVectorFilter(self, aFilter):
     self.setFilter("lastVector", aFilter[0])

  # Retrieves the extensions list from a filter string
  @classmethod
  def getFilterExtensions(self, aFilter):
    extList = []
    # foreach format in filter string
    for f in aFilter.split( ";;" ):
      # gets the list of extensions from the filter
      exts = re.sub('\).*$', '', re.sub('^.*\(', '', f))
      # if there is no extensions or the filter matches all, then return an empty list
      # otherwise return the list of estensions
      if exts != '' and exts != "*" and exts != "*.*":
        extList.extend(exts.split(" "))
    return extList

  @classmethod
  def getFilterName(self, aFilter):
    return string.strip(re.sub('\ \(.*$', '', aFilter))

  @classmethod
  def filenameMatchesFilterExt(self, fileName, ext):
    return re.match( '.'+str(ext), fileName ) is not None

# Retrieves gdal information
class GdalConfig:
  # retrieves and return the installed gdal version
  @classmethod
  def version(self):
      return Version(gdal.VersionInfo("RELEASE_NAME"))

  @classmethod
  def versionNum(self):
      return int(gdal.VersionInfo("VERSION_NUM"))

  # store the supported rasters info
  supportedRasters = None

  # retrieve the supported rasters info
  @classmethod
  def getSupportedRasters(self):
    if self.supportedRasters != None:
      return self.supportedRasters

    # first get the GDAL driver manager
    if gdal.GetDriverCount() == 0:
      gdal.AllRegister()

    self.supportedRasters = dict()
    jp2Driver = None

    # for each loaded GDAL driver
    for i in range(gdal.GetDriverCount()):
      driver = gdal.GetDriver(i)

      if driver == None:
        QgsLogger.warning("unable to get driver " + str(i))
        continue

      # now we need to see if the driver is for something currently
      # supported; if not, we give it a miss for the next driver

      longName = string.strip(re.sub('\(.*$', '', driver.LongName))
      shortName = string.strip(re.sub('\(.*$', '', driver.ShortName))
      extensions = ''

      description = driver.GetDescription()
      glob = []

      metadata = driver.GetMetadata()
      if metadata.has_key(gdal.DMD_EXTENSION):
        extensions = str(metadata[gdal.DMD_EXTENSION])

      if longName != '':
        if extensions != '':
          # XXX add check for SDTS; in that case we want (*CATD.DDF)

          #TODO fix and test
          #glob.append( QString("*." + extensions.replace("/", " *.")).split(" "))
          glob.append(string.split("*." + string.replace(extensions,"/", " *."), sep=(" ")))

          # Add only the first JP2 driver found to the filter list (it's the one GDAL uses)
          if description == "JPEG2000" or description.startswith("JP2"): # JP2ECW, JP2KAK, JP2MrSID
            if jp2Driver != None:
              continue               # skip if already found a JP2 driver
            jp2Driver = driver   # first JP2 driver found
            glob.append("*.j2k")           # add alternate extension
          elif description == "GTiff":
            glob.append("*.tiff")
          elif description == "JPEG":
            glob.append("*.jpeg")
        else:
          # USGS DEMs use "*.dem"
          if description.startswith( "USGSDEM" ):
            glob.append("*.dem")
          elif description.startswith( "DTED" ):
            # DTED use "*.dt0"
            glob.append("*.dt0")
          elif description.startswith( "MrSID" ):
            # MrSID use "*.sid"
            glob.append("*.sid")
          else:
            continue

        self.supportedRasters[longName] = {'EXTENSIONS': glob, 'LONGNAME': longName, 'SHORTNAME': shortName, 'DESCRIPTION': description}

    return self.supportedRasters

  # store the supported vectors info
  supportedVectors = None

  # retrieve the supported vectors info
  @classmethod
  def getSupportedVectors(self):
    if self.supportedVectors != None:
      return self.supportedVectors

    # first get the OGR driver manager
    QgsApplication.registerOgrDrivers()

    self.supportedVectors = dict()

    # for each loaded OGR driver
    for i in range(ogr.GetDriverCount()):
      driver = ogr.GetDriver(i)

      if driver == None:
        QgsLogger.warning("unable to get driver " + str(i))
        continue

      driverName = driver.GetName()
      longName = ''
      glob = []

      if driverName.startswith( "AVCBin" ):
        pass #myDirectoryDrivers += "Arc/Info Binary Coverage,AVCBin"
      elif driverName.startswith( "AVCE00" ):
        longName = "Arc/Info ASCII Coverage"
        glob.append("*.e00")
      elif driverName.startswith( "BNA" ):
        longName = "Atlas BNA"
        glob.append("*.bna")
      elif driverName.startswith( "CSV" ):
        longName = "Comma Separated Value"
        glob.append("*.csv")
      elif driverName.startswith( "DODS" ):
        pass #myProtocolDrivers += "DODS/OPeNDAP,DODS"
      elif driverName.startswith( "PGeo" ):
        pass #myDatabaseDrivers += "ESRI Personal GeoDatabase,PGeo"

        # on Windows add a pair to the dict for this driver
        if platform.system() == "Windows":
          longName = "ESRI Personal GeoDatabase"
          glob.append("*.mdb")
      elif driverName.startswith( "SDE" ):
        pass #myDatabaseDrivers += "ESRI ArcSDE,SDE"
      elif driverName.startswith( "ESRI" ):
        longName = "ESRI Shapefiles"
        glob.append("*.shp")
      elif driverName.startswith( "FMEObjects Gateway" ):
        longName = "FMEObjects Gateway"
        glob.append("*.fdd")
      elif driverName.startswith( "GeoJSON" ):
        pass #myProtocolDrivers += "GeoJSON,GeoJSON"
        longName = "GeoJSON"
        glob.append("*.geojson")
      elif driverName.startswith( "GeoRSS" ):
        longName = "GeoRSS"
        glob.append("*.xml")
      elif driverName.startswith( "GML" ):
        longName = "Geography Markup Language"
        glob.append("*.gml")
      elif driverName.startswith( "GMT" ):
        longName = "GMT"
        glob.append("*.gmt")
      elif driverName.startswith( "GPX" ):
        longName = "GPX"
        glob.append("*.gpx")
      elif driverName.startswith( "GRASS" ):
        pass #myDirectoryDrivers += "Grass Vector,GRASS"
      elif driverName.startswith( "IDB" ):
        pass #myDatabaseDrivers += "Informix DataBlade,IDB"
      elif driverName.startswith( "Interlis 1" ):
        longName = "INTERLIS 1"
        glob.append("*.itf")
        glob.append("*.xml")
        glob.append("*.ili")
      elif driverName.startswith( "Interlis 2" ):
        longName = "INTERLIS 2"
        glob.append("*.itf")
        glob.append("*.xml")
        glob.append("*.ili")
      elif driverName.startswith( "INGRES" ):
        pass #myDatabaseDrivers += "INGRES,INGRES"
      elif driverName.startswith( "KML" ):
        longName = "KML"
        glob.append("*.kml")
      elif driverName.startswith( "MapInfo File" ):
        longName = "Mapinfo File"
        glob.append("*.mif")
        glob.append("*.tab")
      elif driverName.startswith( "DGN" ):
        longName = "Microstation DGN"
        glob.append("*.dgn")
      elif driverName.startswith( "MySQL" ):
        pass #myDatabaseDrivers += "MySQL,MySQL"
      elif driverName.startswith( "OCI" ):
        pass #myDatabaseDrivers += "Oracle Spatial,OCI"
      elif driverName.startswith( "ODBC" ):
        pass #myDatabaseDrivers += "ODBC,ODBC"
      elif driverName.startswith( "OGDI" ):
        pass #myDatabaseDrivers += "OGDI Vectors,OGDI"
      elif driverName.startswith( "PostgreSQL" ):
        pass #myDatabaseDrivers += "PostgreSQL,PostgreSQL"
      elif driverName.startswith( "S57" ):
        longName = "S-57 Base file"
        glob.append("*.000")
      elif driverName.startswith( "SDTS" ):
        longName = "Spatial Data Transfer Standard"
        glob.append("*catd.ddf")
      elif driverName.startswith( "SQLite" ):
        longName = "SQLite"
        glob.append("*.sqlite")
      elif driverName.startswith( "UK .NTF" ):
        pass #myDirectoryDrivers += "UK. NTF,UK. NTF"
      elif driverName.startswith( "TIGER" ):
        pass #myDirectoryDrivers += "U.S. Census TIGER/Line,TIGER"
      elif driverName.startswith( "VRT" ):
        longName = "VRT - Virtual Datasource "
        glob.append("*.vrt")
      elif driverName.startswith( "XPlane" ):
        longName = "X-Plane/Flighgear"
        glob.append("apt.dat")
        glob.append("nav.dat")
        glob.append("fix.dat")
        glob.append("awy.dat")

      longName = string.strip(longName)

      if longName =='':
        continue

      self.supportedVectors[longName] = {'EXTENSIONS': glob, 'LONGNAME': longName, 'SHORTNAME': driverName}

    return self.supportedVectors

  class SupportedRasters:
    dict_long2shortName = dict()

    # retrieve the raster format short name by long format name
    @classmethod
    def long2ShortName(self, longName):
      if longName == '':
        return ''

      if self.dict_long2shortName.has_key(longName):
        return self.dict_long2shortName[longName]

      # first get the GDAL driver manager
      if gdal.GetDriverCount() == 0:
        gdal.AllRegister()

      shortName = ''

      # for each loaded GDAL driver
      for i in range(gdal.GetDriverCount()):
        driver = gdal.GetDriver(i)
        if driver == None:
          continue

        # if this is the driver we searched for then return its short name
        if FileFilter.getFilterName(driver.LongName) == longName:
          shortName = FileFilter.getFilterName(driver.ShortName)
          self.dict_long2shortName[longName] = shortName
          break

      return shortName

    # retrieve the raster format short name by using the file name extension
    @classmethod
    def filename2ShortName(self, fileName):
      if fileName == '':
        return ''

      shortName = ''

      # for each raster format search for the file extension
      formats = FileFilter.allRastersFilter().split( ";;" )
      for f in formats:
        for ext in FileFilter.getFilterExtensions(f):
          if FileFilter.filenameMatchesFilterExt(fileName, ext):
            longName = FileFilter.getFilterName(f)
            shortName = self.long2ShortName(longName)
            break

        if not shortName == '':
          break

      return shortName

# class which allows creating version objects and compare them
class Version:
  def __init__(self, ver):
      self.vers = ('0', '0', '0')

      if isinstance(ver, Version):
        self.vers = ver.vers
      elif isinstance(ver, tuple) or isinstance(ver, list):
        self.vers = map(str, ver)
      elif isinstance(ver, str):
        self.vers = self.string2vers(ver)

  @staticmethod
  def string2vers(string):
      vers = ['0', '0', '0']

      nums = str(string).split(".")

      if len(nums) > 0:
        vers[0] = nums[0]
      if len(nums) > 1:
        vers[1] = nums[1]
      if len(nums) > 2:
        vers[2] = nums[2]

      return (vers[0], vers[1], vers[2])

  def __cmp__(self, other):
      if not isinstance(other, Version):
        other = Version(other)

      if self.vers > other.vers:
        return 1
      if self.vers < other.vers:
        return -1
      return 0

  def __str__(self):
    return ".".join(self.vers)


def setProcessEnvironment(process):
    envvar_list = {
        "PATH" : getGdalBinPath(),
        "PYTHONPATH" : getGdalPymodPath(),
        "GDAL_FILENAME_IS_UTF8" : "NO"
    }

    sep = os.pathsep

    for name, val in envvar_list.iteritems():
      if val == None or val == "":
        continue

      envval = os.getenv(name)
      if envval == None or envval == "":
        envval = str(val)
      elif (platform.system() == "Windows" and val.lower() not in envval.lower().split( sep )) or \
          (platform.system() != "Windows" and val not in envval.split( sep )):
        envval += "%s%s" % (sep, str(val))
      else:
        envval = None

      if envval != None:
        os.putenv( name, envval )

def setMacOSXDefaultEnvironment():
  # fix bug #3170: many GDAL Tools don't work in OS X standalone

  if platform.system() != "Darwin":
    return

  # QgsApplication.prefixPath() contains the path to qgis executable (i.e. .../Qgis.app/MacOS)
  # get the path to Qgis application folder
  qgis_app = u"%s/.." % QgsApplication.prefixPath()
  qgis_app = QDir( qgis_app ).absolutePath()

  qgis_bin = u"%s/bin" % QgsApplication.prefixPath()   # path to QGis bin folder
  qgis_python = u"%s/Resources/python" % qgis_app    # path to QGis python folder

  # path to the GDAL framework within the Qgis application folder (QGis standalone only)
  qgis_standalone_gdal_path = u"%s/Frameworks/GDAL.framework" % qgis_app

  # path to the GDAL framework when installed as external framework
  gdal_versionsplit = str(GdalConfig.version()).split('.')
  gdal_base_path = u"/Library/Frameworks/GDAL.framework/Versions/%s.%s" % (gdal_versionsplit[0], gdal_versionsplit[1])

  if os.path.exists( qgis_standalone_gdal_path ):  # qgis standalone
    # GDAL executables are in the QGis bin folder
    if getGdalBinPath() == '':
      setGdalBinPath( qgis_bin )
    # GDAL pymods are in the QGis python folder
    if getGdalPymodPath() == '':
      setGdalPymodPath( qgis_python )
    # GDAL help is in the framework folder
    if getHelpPath() == '':
      setHelpPath( u"%s/Resources/doc" % qgis_standalone_gdal_path )

  elif os.path.exists( gdal_base_path ):
    # all GDAL parts are in the GDAL framework folder
    if getGdalBinPath() == '':
      setGdalBinPath( u"%s/Programs" % gdal_base_path )
    if getGdalPymodPath() == '':
      setGdalPymodPath( u"%s/Python/%s.%s/site-packages" % (gdal_base_path, sys.version_info[0], sys.version_info[1]) )
    if getHelpPath() == '':
      setHelpPath( u"%s/Resources/doc" % gdal_base_path )


# setup the MacOSX path to both GDAL executables and python modules
if platform.system() == "Darwin":
  setMacOSXDefaultEnvironment()

