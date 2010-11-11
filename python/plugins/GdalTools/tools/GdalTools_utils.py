# -*- coding: utf-8 -*-

# Utility functions
# -------------------------------------------------
# getLastUsedDir()
# setLastUsedDir( QString *file_or_dir path )
# -------------------------------------------------

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *
from osgeo import gdal
from osgeo.gdalconst import *
from osgeo import ogr

# to know the os
import platform

# Escapes arguments and return them joined in a string
def escapeAndJoin(strList):
    joined = QString()
    for s in strList:
      if s.contains(" "):
        escaped = '"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"'
      else:
        escaped = s
      joined += escaped + " "
    return joined.trimmed()

# Retrieves last used dir from persistent settings
def getLastUsedDir():
    settings = QSettings()
    lastProjectDir = settings.value( "/UI/lastProjectDir", QVariant(".") ).toString()
    return settings.value( "/GdalTools/lastUsedDir", QVariant(lastProjectDir) ).toString()

# Stores last used dir in persistent settings
def setLastUsedDir(filePath):
    settings = QSettings()
    fileInfo = QFileInfo(filePath)
    if fileInfo.isDir():
      dirPath = fileInfo.filePath()
    else:
      dirPath = fileInfo.path()
    settings.setValue( "/GdalTools/lastUsedDir", QVariant(dirPath) )

# Retrieves GDAL binaries location
def getGdalPath():
  settings = QSettings()
  return settings.value( "/GdalTools/gdalPath", QVariant( "" ) ).toString()

# Stores GDAL binaries location
def setGdalPath( path ):
  settings = QSettings()
  settings.setValue( "/GdalTools/gdalPath", QVariant( path ) )

# Retrieves GDAL help files location
def getHelpPath():
  settings = QSettings()
  return settings.value( "/GdalTools/helpPath", QVariant( "" ) ).toString()

# Stores GDAL help files location
def setHelpPath( path ):
  settings = QSettings()
  settings.setValue( "/GdalTools/helpPath", QVariant( path ) )

# Retrieves last used encoding from persistent settings
def getLastUsedEncoding():
    settings = QSettings()
    return settings.value( "/UI/encoding", QVariant("System") ).toString()

# Stores last used encoding in persistent settings
def setLastUsedEncoding(encoding):
    settings = QSettings()
    settings.setValue( "/UI/encoding", QVariant(encoding) )

def getRasterExtensions():
  formats = FileFilter.allRastersFilter().split( ";;" )
  extensions = QStringList()
  for f in formats:
    if f.contains( "*.bt" ) or f.contains( "*.mpr" ):   # Binary Terrain or ILWIS
      continue
    extensions << FileFilter.getFilterExtensions( f )
  return extensions

def getVectorExtensions():
  formats = FileFilter.allVectorsFilter().split( ";;" )
  extensions = QStringList()
  for f in formats:
    extensions << FileFilter.getFilterExtensions( f )
  return extensions

def getRasterLayers():
  # mantein the reference between combobox indexes and canvas layers
  layers = dict()
  count = 0
  names = []
  layerMap = QgsMapLayerRegistry.instance().mapLayers()
  for name, layer in layerMap.iteritems():
    # only raster layers, but not WMS ones
    if layer.type() == layer.RasterLayer and ( not layer.usesProvider() ):
      layers[count] = layer
      names.append(layer.name())
      count = count +1
  return (layers, names)

def getVectorLayers():
  # mantein the reference between combobox indexes and canvas layers
  layers = dict()
  count = 0
  names = []
  layerMap = QgsMapLayerRegistry.instance().mapLayers()
  for name, layer in layerMap.iteritems():
    if layer.type() == layer.VectorLayer:
      layers[count] = layer
      names.append(layer.name())
      count = count +1
  return (layers, names)

def fillRasterOutputFormat(aFilter = None, filename = None):
  shortName = QString()

  if aFilter != None:
    supportedRasters = GdalConfig.getSupportedRasters()
    filterName = FileFilter.getFilterName( aFilter ).remove( QRegExp('^.*\] ') )
    if supportedRasters.has_key( filterName ):
      return supportedRasters[ filterName ][ "SHORTNAME" ]

    shortName = GdalConfig.SupportedRasters.long2ShortName(filterName)

  if shortName.isEmpty() and filename != None:
    shortName = GdalConfig.SupportedRasters.filename2ShortName(filename)

  if shortName.isEmpty():
    shortName = "GTiff"

  return shortName

def fillVectorOutputFormat(aFilter = None, filename = None):
  shortName = QString()

  if aFilter != None:
    supportedVectors = GdalConfig.getSupportedVectors()
    filterName = FileFilter.getFilterName( aFilter ).remove( QRegExp('^.*\] ') )
    if supportedVectors.has_key( filterName ):
      return supportedVectors[ filterName ][ "SHORTNAME" ]

    pass #shortName = GdalConfig.SupportedVectors.long2ShortName(filterName)

  if shortName.isEmpty() and filename != None:
    pass #shortName = GdalConfig.SupportedVectors.filename2ShortName(filename)

  if shortName.isEmpty():
    shortName = "ESRI Shapefile"

  return shortName

def getVectorFields(vectorFile):
    hds = ogr.Open( unicode(vectorFile).encode('utf8') )
    if hds == None:
      raise Exception( QCoreApplication.translate( "GdalTools", "The selected file is not a supported OGR format" ) )

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
    processSRS = QProcess( parent )
    processSRS.start( "gdalinfo", QStringList() << fileName, QIODevice.ReadOnly )
    arr = QByteArray()
    if processSRS.waitForFinished():
      arr = processSRS.readAllStandardOutput()
      processSRS.close()

    if not arr.isEmpty():
      info = QString( arr ).split( "\n" ).filter( "AUTHORITY" )
      if info.count() == 0:
        return QString()

    srs = info[ info.count() - 1 ]
    srs = srs.simplified().remove( "AUTHORITY[" )
    srs = srs.remove( QRegExp( "\]{2,4},?" ) ).remove( "\"" )
    info = srs.split( "," )
    srs = info[ 0 ] + ":" + info[ 1 ]
    return srs

# This class is used to replace the QFileDialog class.
# Its static methods are used in place of the respective QFileDialog ones to:
# 1. set the last used directory
# 2. append the selected extension to the file name
# 3. bypass the following bug:
# when you use the 'filter' argument, the dialog is enlarged up to the longest filter length,
# so sometimes the dialog excedes the screen width
class FileDialog:
  @classmethod
  def getDialog(self, parent = None, caption = QString(), acceptMode = QFileDialog.AcceptOpen, fileMode = QFileDialog.ExistingFile, filter = QString(), selectedFilter = None, useEncoding = False):
    if useEncoding:
      dialog = QgsEncodingFileDialog(parent, caption, getLastUsedDir(), filter, getLastUsedEncoding())
    else:
      dialog = QFileDialog(parent, caption, getLastUsedDir(), filter)
    dialog.setFileMode(fileMode)
    dialog.setAcceptMode(acceptMode)

    if selectedFilter != None:
      dialog.selectNameFilter(selectedFilter)

    if not dialog.exec_():
      if useEncoding:
        return (QString(), None)
      return QString()

    # change the selected filter value
    if selectedFilter != None:
      selectedFilter.clear()
      selectedFilter += dialog.selectedNameFilter()

    # save the last used dir and return the selected files
    files = dialog.selectedFiles()
    if not files.isEmpty():
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
            if firstExt.startsWith('*'):
              files.append( firstExt[1:] )

    if useEncoding:
      encoding = dialog.encoding()
      # encoding setted yet by QgsEncodingFileDialog
      #setLastUsedEncoding(encoding)
      return (files, encoding)

    return files

  @classmethod
  def getOpenFileNames(self, parent = None, caption = QString(), filter = QString(), selectedFilter = None, useEncoding = False):
    return self.getDialog(parent, caption, QFileDialog.AcceptOpen, QFileDialog.ExistingFiles, filter, selectedFilter, useEncoding)

  @classmethod
  def getOpenFileName(self, parent = None, caption = QString(), filter = QString(), selectedFilter = None, useEncoding = False):
    return self.getDialog(parent, caption, QFileDialog.AcceptOpen, QFileDialog.ExistingFile, filter, selectedFilter, useEncoding)

  @classmethod
  def getSaveFileName(self, parent = None, caption = QString(), filter = QString(), selectedFilter = None, useEncoding = False):
    return self.getDialog(parent, caption, QFileDialog.AcceptSave, QFileDialog.AnyFile, filter, selectedFilter, useEncoding)

  @classmethod
  def getExistingDirectory(self, parent = None, caption = QString(), useEncoding = False):
    return self.getDialog(parent, caption, QFileDialog.AcceptOpen, QFileDialog.DirectoryOnly, QString(), None, useEncoding)

class FileFilter:
  @classmethod
  def getFilter(self, typeName):
      settings = QSettings()
      return settings.value( "/GdalTools/" + typeName + "FileFilter", QVariant( "" ) ).toString()

  @classmethod
  def setFilter(self, typeName, aFilter):
      settings = QSettings()
      settings.setValue( "/GdalTools/" + typeName + "FileFilter", QVariant( aFilter ) )

  # stores the supported raster file filter
  rastersFilter = QString()

  # Retrieves the filter for supported raster files
  @classmethod
  def allRastersFilter(self):
    if self.rastersFilter.isEmpty():
      QgsRasterLayer.buildSupportedRasterFileFilter(self.rastersFilter)

      # workaround for QGis < 1.5 (see #2376)
      # separates multiple extensions that joined by a slash 
      if QGis.QGIS_VERSION[0:3] < "1.8":
          formats = self.rastersFilter.split( ";;" )
          self.rastersFilter = QString()
          for f in formats:
            oldExts = QString(f).remove( QRegExp('^.*\(') ).remove( QRegExp('\).*$') )
            newExts = QString(oldExts).replace( '/', ' *.' )
            if not self.rastersFilter.isEmpty():
              self.rastersFilter += ';;'
            self.rastersFilter += f.replace( oldExts, newExts )

    return self.rastersFilter

  @classmethod
  def lastUsedRasterFilter(self):
     return self.getFilter("lastRaster")

  @classmethod
  def setLastUsedRasterFilter(self, aFilter):
     self.setFilter("lastRaster", aFilter)

  # stores the supported vectors file filter
  vectorsFilter = QString()

  # Retrieves the filter for supported vector files
  @classmethod
  def allVectorsFilter(self):
    if self.vectorsFilter.isEmpty():
      self.vectorsFilter = QgsProviderRegistry.instance().fileVectorFilters()
    return self.vectorsFilter

  @classmethod
  def lastUsedVectorFilter(self):
     return self.getFilter("lastVector")

  @classmethod
  def setLastUsedVectorFilter(self, aFilter):
     self.setFilter("lastVector", aFilter)

  # Retrieves the extensions list from a filter string
  @classmethod
  def getFilterExtensions(self, aFilter):
    extList = QStringList()
    # foreach format in filter string
    for f in aFilter.split( ";;" ):
      # gets the list of extensions from the filter
      exts = f.remove( QRegExp('^.*\(') ).remove( QRegExp('\).*$') )
      # if there is no extensions or the filter matches all, then return an empty list
      # otherwise return the list of estensions
      if not exts.isEmpty() and exts != "*" and exts != "*.*":
        extList << exts.split(" ")
    return extList

  @classmethod
  def getFilterName(self, aFilter):
    return QString(aFilter).remove( QRegExp('\ \(.*$') ).trimmed()

  @classmethod
  def filenameMatchesFilterExt(self, fileName, ext):
    regex = QRegExp(ext)
    # use the wildcard matching
    regex.setPatternSyntax(QRegExp.Wildcard)

    if regex.exactMatch(fileName):
      return True
    return False

# Retrieves gdal information
class GdalConfig:
  # retrieves and return the installed gdal version
  @classmethod
  def version(self):
      return Version(gdal.VersionInfo("RELEASE_NAME"))

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
        QgsLogger.warning("unable to get driver " + QString.number(i))
        continue

      # now we need to see if the driver is for something currently
      # supported; if not, we give it a miss for the next driver

      longName = QString(driver.LongName).remove( QRegExp( '\(.*$' ) ).trimmed()
      shortName = QString(driver.ShortName).remove( QRegExp( '\(.*$' ) ).trimmed()
      extensions = QString()

      description = QString(driver.GetDescription())
      glob = QStringList()

      metadata = driver.GetMetadata()
      if metadata.has_key(gdal.DMD_EXTENSION):
        extensions = QString(metadata[gdal.DMD_EXTENSION])

      if not longName.isEmpty():
        if not extensions.isEmpty():
          # XXX add check for SDTS; in that case we want (*CATD.DDF)
          glob << QString("*." + extensions.replace("/", " *.")).split(" ")

          # Add only the first JP2 driver found to the filter list (it's the one GDAL uses)
          if description == "JPEG2000" or description.startsWith("JP2"): # JP2ECW, JP2KAK, JP2MrSID
            if jp2Driver != None:
              continue               # skip if already found a JP2 driver
            jp2Driver = driver   # first JP2 driver found
            glob << "*.j2k"           # add alternate extension
          elif description == "GTiff":
            glob << "*.tiff"
          elif description == "JPEG":
            glob << "*.jpeg"
        else:
          # USGS DEMs use "*.dem"
          if description.startsWith( "USGSDEM" ):
            glob << "*.dem"
          elif description.startsWith( "DTED" ):
            # DTED use "*.dt0"
            glob << "*.dt0"
          elif description.startsWith( "MrSID" ):
            # MrSID use "*.sid"
            glob << "*.sid"
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
        QgsLogger.warning("unable to get driver " + QString.number(i))
        continue

      driverName = QString(driver.GetName())
      longName = QString()
      glob = QStringList()

      if driverName.startsWith( "AVCBin" ):
        pass #myDirectoryDrivers += "Arc/Info Binary Coverage,AVCBin"
      elif driverName.startsWith( "AVCE00" ):
        longName = "Arc/Info ASCII Coverage"
        glob << "*.e00"
      elif driverName.startsWith( "BNA" ):
        longName = "Atlas BNA"
        glob << "*.bna"
      elif driverName.startsWith( "CSV" ):
        longName = "Comma Separated Value"
        glob << "*.csv"
      elif driverName.startsWith( "DODS" ):
        pass #myProtocolDrivers += "DODS/OPeNDAP,DODS"
      elif driverName.startsWith( "PGeo" ):
        pass #myDatabaseDrivers += "ESRI Personal GeoDatabase,PGeo"

        # on Windows add a pair to the dict for this driver
        if platform.system() == "Windows":
          longName = "ESRI Personal GeoDatabase"
          glob << "*.mdb"
      elif driverName.startsWith( "SDE" ):
        pass #myDatabaseDrivers += "ESRI ArcSDE,SDE"
      elif driverName.startsWith( "ESRI" ):
        longName = "ESRI Shapefiles"
        glob << "*.shp"
      elif driverName.startsWith( "FMEObjects Gateway" ):
        longName = "FMEObjects Gateway"
        glob << "*.fdd"
      elif driverName.startsWith( "GeoJSON" ):
        pass #myProtocolDrivers += "GeoJSON,GeoJSON"
        longName = "GeoJSON"
        glob << "*.geojson"
      elif driverName.startsWith( "GeoRSS" ):
        longName = "GeoRSS"
        glob << "*.xml"
      elif driverName.startsWith( "GML" ):
        longName = "Geography Markup Language"
        glob << "*.gml"
      elif driverName.startsWith( "GMT" ):
        longName = "GMT"
        glob << "*.gmt"
      elif driverName.startsWith( "GPX" ):
        longName = "GPX"
        glob << "*.gpx"
      elif driverName.startsWith( "GRASS" ):
        pass #myDirectoryDrivers += "Grass Vector,GRASS"
      elif driverName.startsWith( "IDB" ):
        pass #myDatabaseDrivers += "Informix DataBlade,IDB"
      elif driverName.startsWith( "Interlis 1" ):
        longName = "INTERLIS 1"
        glob << "*.itf" << "*.xml" << "*.ili"
      elif driverName.startsWith( "Interlis 2" ):
        longName = "INTERLIS 2"
        glob << "*.itf" << "*.xml" << "*.ili"
      elif driverName.startsWith( "INGRES" ):
        pass #myDatabaseDrivers += "INGRES,INGRES"
      elif driverName.startsWith( "KML" ):
        longName = "KML"
        glob << "*.kml"
      elif driverName.startsWith( "MapInfo File" ):
        longName = "Mapinfo File"
        glob << "*.mif" << "*.tab"
      elif driverName.startsWith( "DGN" ):
        longName = "Microstation DGN"
        glob << "*.dgn"
      elif driverName.startsWith( "MySQL" ):
        pass #myDatabaseDrivers += "MySQL,MySQL"
      elif driverName.startsWith( "OCI" ):
        pass #myDatabaseDrivers += "Oracle Spatial,OCI"
      elif driverName.startsWith( "ODBC" ):
        pass #myDatabaseDrivers += "ODBC,ODBC"
      elif driverName.startsWith( "OGDI" ):
        pass #myDatabaseDrivers += "OGDI Vectors,OGDI"
      elif driverName.startsWith( "PostgreSQL" ):
        pass #myDatabaseDrivers += "PostgreSQL,PostgreSQL"
      elif driverName.startsWith( "S57" ):
        longName = "S-57 Base file"
        glob << "*.000"
      elif driverName.startsWith( "SDTS" ):
        longName = "Spatial Data Transfer Standard"
        glob << "*catd.ddf"
      elif driverName.startsWith( "SQLite" ):
        longName = "SQLite"
        glob << "*.sqlite"
      elif driverName.startsWith( "UK .NTF" ):
        pass #myDirectoryDrivers += "UK. NTF,UK. NTF"
      elif driverName.startsWith( "TIGER" ):
        pass #myDirectoryDrivers += "U.S. Census TIGER/Line,TIGER"
      elif driverName.startsWith( "VRT" ):
        longName = "VRT - Virtual Datasource "
        glob << "*.vrt"
      elif driverName.startsWith( "XPlane" ):
        longName = "X-Plane/Flighgear"
        glob << "apt.dat" << "nav.dat" << "fix.dat" << "awy.dat"

      longName = QString(longName).trimmed()

      if longName.isEmpty():
        continue

      self.supportedVectors[longName] = {'EXTENSIONS': glob, 'LONGNAME': longName, 'SHORTNAME': driverName}

    return self.supportedVectors

  class SupportedRasters:
    dict_long2shortName = dict()

    # retrieve the raster format short name by long format name
    @classmethod
    def long2ShortName(self, longName):
      if longName.isEmpty():
        return QString()

      if self.dict_long2shortName.has_key(longName):
        return self.dict_long2shortName[longName]

      # first get the GDAL driver manager
      if gdal.GetDriverCount() == 0:
        gdal.AllRegister()

      shortName = QString()

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
      if fileName.isEmpty():
        return QString()

      shortName = QString()

      # for each raster format search for the file extension
      formats = FileFilter.allRastersFilter().split( ";;" )
      for f in formats:
        for ext in FileFilter.getFilterExtensions(f):
          if FileFilter.filenameMatchesFilterExt(fileName, ext):
            longName = FileFilter.getFilterName(f)
            shortName = self.long2ShortName(longName)
            break

        if not shortName.isEmpty():
          break

      return shortName

# class which allows to create version objects and compare them
class Version:
  def __init__(self, ver):
      self.vers = ('0', '0', '0')

      if isinstance(ver, Version):
        self.vers = ver.vers
      elif isinstance(ver, tuple) or isinstance(ver, list):
        self.vers = map(str, ver)
      elif isinstance(ver, str) or isinstance(ver, QString):
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
