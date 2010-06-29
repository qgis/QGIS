#***************************************************************************
#    ms_export.py
#    --------------------------------------
#   Date                 : Sun Sep 16 12:33:46 AKDT 2007
#   Copyright            : (C) 2008 by Gary E. Sherman
#   Email                : sherman at mrcc dot com
#***************************************************************************
#*                                                                         *
#*   This program is free software; you can redistribute it and/or modify  *
#*   it under the terms of the GNU General Public License as published by  *
#*   the Free Software Foundation; either version 2 of the License, or     *
#*   (at your option) any later version.                                   *
#*                                                                         *
#***************************************************************************/
# This class exports a QGIS project file to a mapserver .map file.
# All the work is done in the writeMapFile method. The msexport binary
# presents a Qt based GUI that collects the needed information for this
# script. 
# Matthew Perry contributed major portions of this work.
# Adapted by Erik van de Pol
#
# CHANGES SHOULD NOT BE MADE TO THE writeMapFile METHOD UNLESS YOU
# ARE CHANGING THE QgsMapserverExport CLASS AND YOU KNOW WHAT YOU ARE
# DOING

from xml.dom import minidom
from string import *
import platform
from qgis.core import QgsDataSourceURI
from qgis.core import QgsMapLayerRegistry
#from qgis.core import QgsProject
from PyQt4.QtCore import QString
from PyQt4.QtCore import QVariant


# symbol map
qgis2map_symbol = {
  "hard:circle"               : "circle",
  "hard:triangle"             : "triangle",
  "hard:equilateral_triangle" : "equilateral-triangle",
  "hard:rectangle"            : "square",
  "hard:regular_star"         : "star",
  "hard:diamond"              : "diamond"
}

# alignment/position map
qgis2map_aligment2position = {
  "center"     :   "cc",
  "above"      :   "uc",
  "right"      :   "cr",
  "below"      :   "lc",
  "left"       :   "cl",
  "aboveright" :   "ur",
  "belowright" :   "lr",
  "belowleft"  :   "ll",
  "aboveleft"  :   "ul"
}

# the keys are fonts that must be available in QGis
# the values in this dictionary must correspond to 
# the fonts in the file denoted by the FONTSET in the mapfile
#
# "MS Shell Dlg 2" is the default label font in QGis.
# The automatic mapping to "Tahoma" is correct for Windows 2000, Windows XP,
# Windows Server 2003, Windows Vista and Windows 7.
# See: http://msdn.microsoft.com/en-us/library/dd374112%28VS.85%29.aspx
qgis2map_fontset = {
  "Arial"           : "arial",
  "Courier"         : "courier",
  "Georgia"         : "georgia",
  "Times New Roman" : "times",
  "Trebuchet MS"    : "trebuchet_ms",
  "Verdana"         : "verdana",
  "Tahoma"          : "tahoma",
  "MS Shell Dlg 2"  : "tahoma"
}
# Note that tahoma-italic and tahoma-bold-italic do not exist.
# Therefore a mapping to the corresponding verdana-variants is made
# in the fonts file pointed to by the fontsPath. Feel free to map to another font there.

bool2str = {True: "true", False: "false"}

# This is a magic number now. Rationale?
symbolSizeMultiplier = 3.5


class Qgis2MapDefaults: pass

defaults = Qgis2MapDefaults()

defaults.fontsPath = "./fonts/fonts.txt"
defaults.symbolsPath = "./symbols/symbols.txt"
if platform.system() == "Windows":
  defaults.mapServerUrl = "http://my.host.com/cgi-bin/mapserv.exe"
else:
  defaults.mapServerUrl = "http://localhost/cgi-bin/mapserv"
defaults.width = "100"
defaults.height = "100"

defaults.dump = True
defaults.force = True
defaults.partials = True
defaults.antialias = True



class Qgis2Map:
  def __init__(self, mapFile):
    self.mapFile = mapFile
    # init the other members that are not set by the constructor
    self.mapServerUrl = defaults.mapServerUrl
    self.fontsPath = defaults.fontsPath
    self.symbolsPath = defaults.symbolsPath
    self.units = ''
    self.imageType = ''
    self.mapName = ''
    self.width = defaults.width
    self.height = defaults.height
    self.minimumScale = ''
    self.maximumScale = ''
    self.template = ''
    self.header = ''
    self.footer = ''
    self.dump = bool2str[defaults.dump]
    self.force = bool2str[defaults.force]
    self.antialias = bool2str[defaults.antialias]
    self.partials = bool2str[defaults.partials]
    self.symbolQueue = {}

  def setQgsProject(self, projectFileName):
    try:
      self.projectFileName = projectFileName 
      # create the DOM
      self.qgs = minidom.parse(unicode(self.projectFileName))
      return True
    except:
      return False

  # Set the options collected from the GUI
  def setOptions(self, msUrl, units, image, mapname, width, height, template, header, footer, dump, force, antialias, partials, exportLayersOnly, fontsPath, symbolsPath):
    if msUrl.encode('utf-8') != "":
      self.mapServerUrl = msUrl.encode('utf-8')

    if fontsPath.encode('utf-8') != "":
      self.fontsPath = fontsPath.encode('utf-8')

    if symbolsPath.encode('utf-8') != "":
      self.symbolsPath = symbolsPath.encode('utf-8')

    if width.encode('utf-8') != "":
      self.width = width.encode('utf-8')

    if height.encode('utf-8') != "":
      self.height = height.encode('utf-8')

    self.units = units.encode('utf-8')
    self.imageType = image.encode('utf-8')
    self.mapName = mapname.encode('utf-8')

    #self.minimumScale = minscale
    #self.maximumScale = maxscale
    # TEMPLATE is needed for getFeatureInfo requests in WMS:
    # always set someting ...
    template = template.encode('utf-8')
    if template == "":
      template = "fooOnlyForWMSGetFeatureInfo"
    self.template = template
    self.header = header.encode('utf-8')
    self.footer = footer.encode('utf-8')
    #print units, image, mapname, width, height, template, header, footer
    self.dump               = bool2str[dump]
    self.force              = bool2str[force]
    self.antialias          = bool2str[antialias]
    self.partials           = bool2str[partials]
    self.exportLayersOnly   = exportLayersOnly

  # method to check the project file for the exitence of postgis layers
  # if so it should be loaded in qgis before exporting, because a connection
  # to the database is needed to determine primary keys etc etc
  def projectHasPostgisLayers(self):
    # get the list of maplayer nodes
    maplayers = self.qgs.getElementsByTagName("maplayer")
    for lyr in maplayers:
      try:
        providerString = lyr.getElementsByTagName("provider")[0].childNodes[0].nodeValue.encode('utf-8')
      except:
        print "ERROR getting provider string from layer"
        # if providerString is null
        providerString = ''
      if providerString == 'postgres':
        #print  "POSTGIS LAYER !!"
        return True
    return False
      

  ## All real work happens here by calling methods to write the
  ## various sections of the map file
  def writeMapFile(self):
    # open the output file
    print "creating the map file"
    self.outFile = open(self.mapFile, 'w')
    logmsg = "Starting\n"
    
    if self.exportLayersOnly == False:
        # write the general map and web settings
        print " --- python : map section "
        self.writeMapSection()
        logmsg +=  "Wrote map section\n"
        print " --- python : map section done"
        # write the projection section
        print " --- python : proj section "
        self.writeProjectionSection()
        logmsg += "Wrote projection section\n"
        print " --- python : proj section done"
        # write the output format section
        print " --- python : outputformat section "
        self.writeOutputFormat()
        logmsg += "Wrote output format section\n"
        print " --- python : outputformat section done"
        # write the legend section
        print " --- python : legend section"
        self.writeLegendSection()
        logmsg += "Wrote legend section\n"
        print " --- python : legend section done"
        # write the WEB section
        print " --- python : web section "
        webMsg = self.writeWebSection()
        logmsg += "Wrote web section\n"
        logmsg += webMsg
        print " --- python : web section done"

    # write the LAYER sections
    print " --- python : layer section "
    layersMsg = self.writeMapLayers()
    logmsg += "Wrote map layers\n"
    logmsg += layersMsg
    print " --- python : layer section done"

    if self.exportLayersOnly == False:
        # we use an external synbol set instead
        # write the symbol defs section
        # must happen after layers so we can build a symbol queue
        #print " --- python : symbol section "
        #self.writeSymbolSection()
        #logmsg += "Wrote symbol section\n"
        #print " --- python : symbol section done"

        # END and close the map file
        self.outFile.write("END")
        self.outFile.close()

    logmsg += "Map file completed for " + self.projectFileName + "\n"
    logmsg += "Map file saved as " + self.mapFile + "\n"
    if self.exportLayersOnly:
      logmsg += "\n> We only saved the LAYER portion of the map file. \nMerge this into an excisting map file to see it working\n"
    else:
      logmsg += "\n> If this mapfile is accessible by your mapserver, you\nshould be able to see the capabilities by firing this url:\n" + self.mapServerUrl + "?MAP="+self.mapFile+"&SERVICE=WMS&VERSION=1.1.1&REQUEST=GetCapabilities\n"
      logmsg += "\n> if this mapfile is accessible by your mapserver, you\nshould be able to see a map by firing this url:\n" + self.mapServerUrl + "?MAP="+self.mapFile+"&SERVICE=WMS&LAYERS=ALL&MODE=MAP\n"
    return logmsg

  # Write the general parts of the map section
  def writeMapSection(self):
    self.outFile.write("# Map file created from QGIS project file " + str(self.projectFileName).encode('utf-8') + "\n")
    self.outFile.write("# Edit this file to customize for your map interface\n")
    self.outFile.write("# (Created with PyQgis MapServer Export plugin)\n")
    self.outFile.write("MAP\n")
    self.outFile.write("  NAME \"" + self.mapName + "\"\n")
    self.outFile.write("  # Map image size\n")
    if self.width == '' or self.height == '':
      self.outFile.write("  SIZE 0 0\n") 
    else:
      self.outFile.write("  SIZE " + str(self.width) + " " + str(self.height) + "\n")
      
    self.outFile.write("  UNITS %s\n" % (self.units))
    self.outFile.write("\n")
    # extents
    self.outFile.write(self.getExtentString())

    self.outFile.write("  FONTSET '" + self.fontsPath + "'\n")
    # use of external symbol set
    self.outFile.write("  SYMBOLSET '" + self.symbolsPath + "'\n")

  def getExtentString(self):
    stringToAddTo = ""

    xmin = self.qgs.getElementsByTagName("xmin")
    stringToAddTo += "  EXTENT "
    stringToAddTo += xmin[0].childNodes[0].nodeValue.encode('utf-8')
    stringToAddTo += " "

    ymin = self.qgs.getElementsByTagName("ymin")
    stringToAddTo += ymin[0].childNodes[0].nodeValue.encode('utf-8')
    stringToAddTo += " "

    xmax = self.qgs.getElementsByTagName("xmax")
    stringToAddTo += xmax[0].childNodes[0].nodeValue.encode('utf-8')
    stringToAddTo += " "

    ymax = self.qgs.getElementsByTagName("ymax")
    stringToAddTo += ymax[0].childNodes[0].nodeValue.encode('utf-8')
    stringToAddTo += "\n"

    return stringToAddTo

  # Write the OUTPUTFORMAT section
  def writeOutputFormat(self):
    self.outFile.write("  # Background color for the map canvas -- change as desired\n")
    self.outFile.write("  IMAGECOLOR 255 255 255\n")
    self.outFile.write("  IMAGEQUALITY 95\n")
    self.outFile.write("  IMAGETYPE " + self.imageType + "\n")
    self.outFile.write("\n")
    self.outFile.write("  OUTPUTFORMAT\n")
    self.outFile.write("    NAME " + self.imageType + "\n")
    if self.imageType == 'agg':
        self.outFile.write("    DRIVER AGG/PNG\n")
        self.outFile.write("    IMAGEMODE RGB\n")
    else:
        self.outFile.write("    DRIVER 'GD/" + self.imageType.upper() + "'\n")
        self.outFile.write("    MIMETYPE 'image/" + lower(self.imageType) + "'\n")
        if self.imageType.lower() != "gif":
          self.outFile.write("    IMAGEMODE RGBA\n")
        self.outFile.write("    EXTENSION '" + lower(self.imageType) + "'\n")
    self.outFile.write("  END\n")
    

  # Write Projection section
  def writeProjectionSection(self):
    # Need to get the destination srs from one of the map layers since
    # the project file doesn't contain the epsg id or proj4 text for 
    # the map apart from that defined in each layer

    self.outFile.write("  PROJECTION\n")

    # Get the proj4 text from the first map layer's destination SRS
    destsrs = self.qgs.getElementsByTagName("destinationsrs")[0] 
    proj4Text = destsrs.getElementsByTagName("proj4")[0].childNodes[0].nodeValue.encode('utf-8') 
    # the proj4 text string needs to be reformatted to make mapserver happy
    self.outFile.write(self.formatProj4(proj4Text))

    self.outFile.write("  END\n\n")

  # Write the LEGEND section
  def writeLegendSection(self):
    self.outFile.write("  # Legend\n")
    self.outFile.write("  LEGEND\n")
    self.outFile.write("      IMAGECOLOR 255 255 255\n")
    self.outFile.write("    STATUS ON\n")
    self.outFile.write("    KEYSIZE 18 12\n")
    self.outFile.write("    LABEL\n")
    self.outFile.write("      TYPE BITMAP\n")
    self.outFile.write("      SIZE MEDIUM\n")
    self.outFile.write("      COLOR 0 0 89\n")
    self.outFile.write("    END\n")
    self.outFile.write("  END\n\n")

    # groups are ignored as of yet
    self.legendlayerfileNodesById = {}
    for legendlayerfileNode in self.qgs.getElementsByTagName("legend")[0].getElementsByTagName("legendlayerfile"):
      key = legendlayerfileNode.getAttribute("layerid").encode("utf-8")
      if (key != ""):
        self.legendlayerfileNodesById[key] = legendlayerfileNode

  # Write the symbol definitions
  def writeSymbolSection(self):
    for symbol in self.symbolQueue.keys():
      self.outFile.write( self.symbolQueue[symbol] )
      self.outFile.write( "\n" )

  # Write the WEB section of the map file
  def writeWebSection(self):
    resultMsg = ""
    self.outFile.write("  # Web interface definition. Only the template parameter\n")
    self.outFile.write("  # is required to display a map. See MapServer documentation\n")
    self.outFile.write("  WEB\n")
    self.outFile.write("    # Set IMAGEPATH to the path where MapServer should\n")
    self.outFile.write("    # write its output.\n")
    self.outFile.write("    IMAGEPATH '/tmp/'\n")
    self.outFile.write("\n")
    self.outFile.write("    # Set IMAGEURL to the url that points to IMAGEPATH\n")
    self.outFile.write("    # as defined in your web server configuration\n")
    self.outFile.write("    IMAGEURL '/tmp/'\n")
    self.outFile.write("\n")

    destsrs = self.qgs.getElementsByTagName("destinationsrs")[0]
    try:
      epsg = destsrs.getElementsByTagName("srid")[0].childNodes[0].nodeValue.encode("utf-8")
    except:
      # default to epsg
      epsg="4326"
    self.outFile.write("    # WMS server settings\n")
    self.outFile.write("    METADATA\n")
    self.outFile.write("      'ows_title'           '" + self.mapName + "'\n")
    # if mapserverurl is still defaults.mapServerUrl, give warning
    if defaults.mapServerUrl==self.mapServerUrl:
      resultMsg += " ! MapServer url still default value: '" +  defaults.mapServerUrl + \
            "'?\n  Be sure there is a valid mapserverurl in the 'ows_onlineresource'.\n"
    self.outFile.write("      'ows_onlineresource'  '" + self.mapServerUrl + "?" + "map" + "=" + self.mapFile + "'\n")
    self.outFile.write("      'ows_srs'             'EPSG:" + epsg + "'\n")
    self.outFile.write("    END\n\n")

    self.outFile.write("    #Scale range at which web interface will operate\n")
    if self.minimumScale != "":
      self.outFile.write("    MINSCALE " + self.minimumScale + "\n") 
    if self.maximumScale != "":
      self.outFile.write("    MAXSCALE " + self.maximumScale + "\n") 

    self.outFile.write("    # Template and header/footer settings\n")
    self.outFile.write("    # Only the template parameter is required to display a map. See MapServer documentation\n")
    
    if self.template != "":
      self.outFile.write("    TEMPLATE '" + self.template + "'\n")
    if self.header != "":
      self.outFile.write("    HEADER '" + self.header + "'\n")
    if self.footer != "":
      self.outFile.write("    FOOTER '" + self.footer + "'\n")
    self.outFile.write("  END\n\n")
    return resultMsg

  # Write the map layers - we have to defer writing to disk so we
  # can invert the order of the layes, since they are opposite in QGIS 
  # compared to mapserver
  def writeMapLayers(self):
    resultMsg = ''
    # get the layers from the legend to be able to determine the order later
    legend_layers = self.qgs.getElementsByTagName("legendlayerfile")
    self.layer_order = list()
    for legend_layer in legend_layers:
        self.layer_order.append(legend_layer.getAttribute("layerid").encode('utf-8'))
    # get the list of maplayer nodes
    maplayers = self.qgs.getElementsByTagName("maplayer")
    print "Processing ", len(maplayers), " layers"
    count = 0
    layer_list = dict()
    layer_names = []
    for lyr in maplayers:
      count += 1
      print "Processing layer ", count
      # The attributes of the maplayer tag contain the scale dependent settings,
      # visibility, and layer type
      layer_def = "  LAYER\n"
      # store name of the layer - replace space with underscore for wms compliance
      layer_name = lyr.getElementsByTagName("layername")[0].childNodes[0].nodeValue.encode('utf-8').replace("\"", "").replace(" ","_")
      # layername is not unique in qgis, store id of layer
      layer_id = lyr.getElementsByTagName("id")[0].childNodes[0].nodeValue.encode('utf-8')
      # first check to see if there is a name
      if len(layer_name) > 0:
        # WMS layernames should be unique, so
        # if the layer_name already excists in our layer_list of names:
        if layer_name in layer_names:
          # we give it the old name plus number
          layer_name = layer_name + str(count)
      else:
        # if no name for the layer, manufacture one
        layer_name = 'layer' + str(count)
      # store the name to be able to check for double names
      layer_names.append(layer_name)
      layer_def += "    NAME '%s'\n" % layer_name

      if lyr.getAttribute("type").encode('utf-8') == 'vector':  
        layer_def += "    TYPE " + lyr.getAttribute("geometry").encode('utf-8').upper() + "\n"
      elif lyr.getAttribute("type").encode('utf-8') == 'raster':  
        layer_def += "    TYPE " + lyr.getAttribute("type").encode('utf-8').upper() + "\n"

      # Use (global) default value from the gui
      layer_def += "    DUMP " + self.dump + "\n"
      # id dump = true: add TEMPLATE to be able to use getFeatureInfoRequests
      if self.dump=="true":
        layer_def += "    TEMPLATE fooOnlyForWMSGetFeatureInfo\n"
        
      # Set min/max scales
      if lyr.getAttribute('hasScaleBasedVisibilityFlag').encode('utf-8') == 1:
        layer_def += "    MINSCALE " + lyr.getAttribute('minimumScale').encode('utf-8') + "\n"
        layer_def += "    MAXSCALE " + lyr.getAttribute('maximumScale').encode('utf-8') + "\n"

      layer_def += self.getExtentString()

      # data
      dataString = lyr.getElementsByTagName("datasource")[0].childNodes[0].nodeValue.encode('utf-8')

      # test if it is a postgis, grass or WMS layer
      # is there a better way to do this? probably.
      try:
        providerString = lyr.getElementsByTagName("provider")[0].childNodes[0].nodeValue.encode('utf-8')
      except:
        # if providerString is null
        providerString = ''

      if providerString == 'postgres':
        # it's a postgis layer
        uri = QgsDataSourceURI(dataString)
        layer_def += "    CONNECTIONTYPE postgis\n"
        connectionInfo = str(uri.connectionInfo())
        # if connectionInfo does NOT contain a password, warn user:
        if connectionInfo.find("password")<0:
          resultMsg += " ! No password in connection string for postgres layer '" +  layer_name + \
            "' \n  Add it, or make sure mapserver can connect to postgres.\n"  
        layer_def += "    CONNECTION \"" + connectionInfo + "\"\n"
        # EvdP: it seems that the uri.geometryColumn() is quoted automatically by PostGIS.
        # To prevent double quoting, we don't quote here.
        # Now we are unable to process uri.geometryColumn()s with special characters (uppercase... etc.)
        #layer_def += "    DATA '\"" + uri.geometryColumn() + "\" FROM " + uri.quotedTablename() + "'\n"
        #layer_def += "    DATA '" + uri.geometryColumn() + " FROM " + uri.quotedTablename() + "'\n"
        layer_id = lyr.getElementsByTagName("id")[0].childNodes[0].nodeValue.encode("utf-8")
        uniqueId = self.getPrimaryKey(layer_id, uri.table())
        # %tablename% is returned when no uniqueId is found: inform user
        if uniqueId.find("%") >= 0:
            resultMsg += " ! No primary key found for postgres layer '" +  layer_name + \
            "' \n  Make sure you edit the mapfile and change the DATA-string \n    containing '" + uniqueId + "'\n"
        epsg = self.getEpsg(lyr)
        
        layer_def += "    DATA '" + uri.geometryColumn() + " FROM " + uri.quotedTablename() + " USING UNIQUE " + uniqueId + " USING srid=" + epsg + "'\n"
        # don't write the filter keyword if there isn't one
        if uri.sql() != "":
          layer_def += "    FILTER ( " + uri.sql() + " )\n"

      elif providerString == 'wms' and lyr.getAttribute("type").encode('utf-8').upper() == 'RASTER':
        # it's a WMS layer 
        layer_def += "    CONNECTIONTYPE WMS\n"
        layer_def += "    CONNECTION '" + dataString + "'\n"
        rasterProp = lyr.getElementsByTagName("rasterproperties")[0]
        # loop thru wmsSubLayers  
        wmsSubLayers = rasterProp.getElementsByTagName('wmsSublayer')
        wmsNames = []
        wmsStyles = []
        for wmsLayer in wmsSubLayers: 
          wmsNames.append( wmsLayer.getElementsByTagName('name')[0].childNodes[0].nodeValue.encode('utf-8').replace("\"", "") )
          try: 
            wmsStyles.append( wmsLayer.getElementsByTagName('style')[0].childNodes[0].nodeValue.encode('utf-8') )
          except:
            wmsStyles.append( '' )
        # Create necesssary wms metadata
        format = rasterProp.getElementsByTagName('wmsFormat')[0].childNodes[0].nodeValue.encode('utf-8')
        layer_def += "    METADATA\n"
        layer_def += "      'ows_name' '" + ','.join(wmsNames) + "'\n"
        layer_def += "      'wms_server_version' '1.1.1'\n"
        try:
          #ct = lyr.getElementsByTagName('coordinatetransform')[0]
          #srs = ct.getElementsByTagName('sourcesrs')[0].getElementsByTagName('spatialrefsys')[0]
          #epsg = srs.getElementsByTagName('epsg')[0].childNodes[0].nodeValue.encode('utf-8')
          #layer_def += "      'wms_srs' 'EPSG:4326 EPSG:" + epsg + "'\n"
          layer_def += "      'ows_srs' 'EPSG:" + self.getEpsg(lyr) + "'\n"
          # TODO: add epsg to all METADATA tags??
        except:
          print "ERROR while trying to write ows_srs METADATA"
	  pass
        layer_def += "      'wms_format' '" + format + "'\n"
        layer_def += "      'wms_style' '" + ','.join(wmsStyles) + "'\n"
        layer_def += "    END\n"

      else: 
        # its a standard ogr, gdal or grass layer
        layer_def += "    DATA '" + dataString + "'\n"
      
      # WMS settings for all layers
      layer_def += "    METADATA\n"
      layer_def += "      'ows_title' '" + layer_name + "'\n"
      layer_def += "    END\n"

      layer_def += "    STATUS OFF\n"

      # turn status in MapServer on or off based on visibility in QGis:
#      layer_id = lyr.getElementsByTagName("id")[0].childNodes[0].nodeValue.encode("utf-8")
#      legendLayerNode = self.legendlayerfileNodesById[layer_id]
#      if legendLayerNode.getAttribute("visible").encode("utf-8") == "1":
#        layer_def += "    STATUS ON\n"
#      else:
#        layer_def += "    STATUS OFF\n"

      opacity = int ( 100.0 * 
           float(lyr.getElementsByTagName("transparencyLevelInt")[0].childNodes[0].nodeValue.encode('utf-8')) / 255.0 ) 
      layer_def += "    TRANSPARENCY " + str(opacity) + "\n"

      layer_def += "    PROJECTION\n"
      
       # Get the destination srs for this layer and use it to create the projection section 
      destsrs = self.qgs.getElementsByTagName("destinationsrs")[0]  
      proj4Text = destsrs.getElementsByTagName("proj4")[0].childNodes[0].nodeValue.encode('utf-8')
      # TODO: you would think: take DATA-srs here, but often projected
      # shapefiles do not contain srs data ... If we want to do this
      # we should take make the map-srs the qgs-project srs first
      # instead of taking the srs of the first layer 
      # Get the data srs for this layer and use it to create the projection section 
      # datasrs = lyr.getElementsByTagName("srs")[0]  
      # proj4Text = datasrs.getElementsByTagName("proj4")[0].childNodes[0].nodeValue.encode('utf-8') 
      
      # the proj4 text string needs to be reformatted to make mapserver happy
      layer_def += self.formatProj4(proj4Text)
      layer_def += "    END\n"
      scaleDependent = lyr.getAttribute("hasScaleBasedVisibilityFlag").encode('utf-8')
      if scaleDependent == '1':
        # get the min and max scale settings
        minscale = lyr.getAttribute("minimumScale").encode('utf-8')
        maxscale = lyr.getAttribute("maximumScale").encode('utf-8')
        if minscale > '':
          layer_def += "    MINSCALE " + minscale + "\n"
        if maxscale > '':
          layer_def += "    MAXSCALE " + maxscale + "\n"
      # Check for label field (ie LABELITEM) and label status
      try:
        labelElements = lyr.getElementsByTagName("label")
        labelOn = '0'
        labelField = None
        # there are actually 3 different label-element in a layer element:
        for element in labelElements:
            labelParent = element.parentNode.localName
            if labelParent == 'maplayer':
                labelOn = element.childNodes[0].nodeValue.encode('utf-8')
            if labelParent == 'labelattributes':
                labelField = element.getAttribute('fieldname').encode('utf-8')
        if labelField != '' and labelField is not None and labelOn == "1" and labelOn is not None:
          layer_def += "    LABELITEM '" + labelField + "'\n"
      except:
        # no labels
        pass
    
      # write the CLASS section for rendering
      # First see if there is a single symbol renderer
      if lyr.getElementsByTagName("singlesymbol").length > 0:
        layer_def += self.simpleRenderer(lyr, lyr.getElementsByTagName("singlesymbol")[0].getElementsByTagName('symbol')[0] )
      elif lyr.getElementsByTagName("graduatedsymbol").length > 0:
        layer_def += self.graduatedRenderer(lyr, lyr.getElementsByTagName("graduatedsymbol")[0].getElementsByTagName('symbol')[0] )
      elif lyr.getElementsByTagName("continuoussymbol").length > 0:
        layer_def += self.continuousRenderer(lyr, lyr.getElementsByTagName("continuoussymbol")[0] )
      elif lyr.getElementsByTagName("uniquevalue").length > 0:
        layer_def += self.uniqueRenderer(lyr, lyr.getElementsByTagName("uniquevalue")[0].getElementsByTagName('symbol')[0] )

      # end of LAYER
      layer_def += "  END\n\n"

      # add the layer to the list with layer_id as key
      layer_list[layer_id] = layer_def
    # all layers have been processed, reverse the layer_order and write
    # output layer_def's in order as they appear in legend (as seen by user)
    self.layer_order.reverse()
    for layerid in self.layer_order:
      self.outFile.write(layer_list[layerid])
    return resultMsg


  def getEpsg(self, lyr):
    try:
      srs = lyr.getElementsByTagName('srs')[0].getElementsByTagName('spatialrefsys')[0]
      return srs.getElementsByTagName('srid')[0].childNodes[0].nodeValue.encode('utf-8')
    except:  
      #Use 4326 as a sensible default if the above fails
      return "4326"


  def getPrimaryKey(self, layerId, tableName):
    """
    Since we have no Python bindings for "src\providers\postgres\qgspostgresprovider.cpp"
    we approximate the primary key by finding an integer type field containing "fid" or "id"
    This is obviously a lousy solution at best.

    This script requires the project you export to be open in QGis!!
    
    This method will return either the primary key of this table, or
    the string %tablename% in case we're not able to find it...
    """

    mlr = QgsMapLayerRegistry.instance()
    layers = mlr.mapLayers()
    if not QString(layerId) in layers:
      # layerId of this postgis layer NOT in the layerlist... 
      # probably the project is not loaded in qgis 
      #raise Exception("ERROR: layer not found in project layers.... \nThis happens with postgis layers in a project which \nis not loaded in QGis.\nDid you load this project into QGis? \nIf not please load project first, and then export it to mapserver.")
      return str("%" + tableName + "_id%")
      
    layer = layers[QString(layerId)]
    dataProvider = layer.dataProvider()
    fields = dataProvider.fields()

    intTypes = [QVariant.Int, QVariant.LongLong, QVariant.UInt, QVariant.ULongLong]
    
    integerFields = []
    for id, field in fields.iteritems():
      if field.type() in intTypes:
        integerFields.append(id)

    # fid end
    fidIntegerFields = []
    for id in integerFields:
      if fields[id].name().endsWith("fid"):
        fidIntegerFields.append(id)

    if len(fidIntegerFields) == 1:
      return str(fields[fidIntegerFields[0]].name())

    # fid start
    fidIntegerFields[:] = []
    for id in integerFields:
      if fields[id].name().startsWith("fid"):
        fidIntegerFields.append(id)

    if len(fidIntegerFields) == 1:
      return str(fields[fidIntegerFields[0]].name())

    # id end
    idIntegerFields = []
    for id in integerFields:
      if fields[id].name().endsWith("id"):
        idIntegerFields.append(id)

    if len(idIntegerFields) == 1:
      return str(fields[idIntegerFields[0]].name())

    # id start
    idIntegerFields[:] = []
    for id in integerFields:
      if fields[id].name().startsWith("id"):
        idIntegerFields.append(id)

    if len(idIntegerFields) == 1:
      return str(fields[idIntegerFields[0]].name())

    # if we arrive here we have ambiguous or no primary keys
    #print "Error: Could not find primary key from field type and field name information.\n"

    # using a mapfile pre-processor, the proper id field can be substituted in the following:
    return str("%" + tableName + "_id%")

  # Simple renderer ouput
  # We need the layer node and symbol node
  def simpleRenderer(self, layerNode, symbolNode):
    # get the layers geometry type
    geometry = layerNode.getAttribute("geometry").encode('utf-8').upper()

    class_def = "    CLASS\n"

    class_def += "       NAME '" + layerNode.getElementsByTagName("layername")[0].childNodes[0].nodeValue.encode('utf-8').replace("\"", "") + "' \n"

    class_def += "       STYLE\n"
    # use the point symbol map to lookup the mapserver symbol type
    symbol = self.msSymbol( geometry, symbolNode )
    class_def += "         SYMBOL " + symbol + " \n"
    class_def += self.getSymbolSizeString(symbolNode)

    # outline color
    outlineNode = symbolNode.getElementsByTagName('outlinecolor')[0]
    class_def += "         OUTLINECOLOR " + outlineNode.getAttribute('red').encode('utf-8') + ' ' + outlineNode.getAttribute('green').encode('utf-8') + ' ' + outlineNode.getAttribute('blue').encode('utf-8') + "\n"
    # color
    colorNode = symbolNode.getElementsByTagName('fillcolor')[0]
    class_def += "         COLOR " + colorNode.getAttribute('red').encode('utf-8') + ' ' + colorNode.getAttribute('green').encode('utf-8') + ' ' + colorNode.getAttribute('blue').encode('utf-8') + "\n"

    class_def += "       END\n"

    class_def += self.msLabel( layerNode ) 

    # end of CLASS  
    class_def += "    END\n"

    return class_def


  def getSymbolSizeString(self, symbolNode):
    size = float(symbolNode.getElementsByTagName('pointsize')[0].childNodes[0].nodeValue.encode('utf-8'))
    symbolSize = size * symbolSizeMultiplier
    return "         SIZE " + str(symbolSize) + " \n"


  # Graduated symbol renderer output
  def graduatedRenderer(self, layerNode, symbolNode):
    # get the layers geometry type
    geometry = layerNode.getAttribute("geometry").encode('utf-8').upper()

    # get the renderer field for building up the classes
    classField = layerNode.getElementsByTagName('classificationattribute')[0].childNodes[0].nodeValue.encode('utf-8')  
    # write the render item
    class_def = "    CLASSITEM '" + classField + "'\n"

    # write the rendering info for each class
    classes = layerNode.getElementsByTagName('symbol')
    for cls in classes:
      class_def += "    CLASS\n"

      lower = cls.getElementsByTagName('lowervalue')[0].childNodes[0].nodeValue.encode('utf-8')
      upper = cls.getElementsByTagName('uppervalue')[0].childNodes[0].nodeValue.encode('utf-8')
    
      # If there's a label use it, otherwise autogenerate one
      try:
        label = cls.getElementsByTagName('label')[0].childNodes[0].nodeValue.encode('utf-8')
        class_def += "      NAME '" + label + "'\n"
      except: 
        class_def += "      NAME '" + lower + " < " + classField + " < " + upper + "'\n"

      class_def += "      EXPRESSION ( ([" + classField + "] >= " + lower + ") AND ([" + classField + "] <= " + upper + ") )\n"

      class_def += "      STYLE\n"
      symbol = self.msSymbol( geometry, symbolNode )
      class_def += "        SYMBOL " + symbol + "\n"

      # Symbol size 
      if geometry == 'POINT' or geometry == 'LINE':
        class_def += self.getSymbolSizeString(cls)

      # outline color
      outlineNode = cls.getElementsByTagName('outlinecolor')[0]
      class_def += "          OUTLINECOLOR " + outlineNode.getAttribute('red').encode('utf-8') + ' ' + outlineNode.getAttribute('green').encode('utf-8') + ' ' + outlineNode.getAttribute('blue').encode('utf-8') + "\n"
      # color
      colorNode = cls.getElementsByTagName('fillcolor')[0]
      class_def += "          COLOR " + colorNode.getAttribute('red').encode('utf-8') + ' ' + colorNode.getAttribute('green').encode('utf-8') + ' ' + colorNode.getAttribute('blue').encode('utf-8') + "\n"

      class_def += "        END\n"

      # label
      class_def += self.msLabel( layerNode ) 

      # end of CLASS  
      class_def += "    END\n"

    return class_def

  # Continuous symbol renderer output
  def continuousRenderer(self, layerNode, symbolNode):
    # get the layers geometry type
    geometry = layerNode.getAttribute("geometry").encode('utf-8').upper()

    # get the renderer field for building up the classes
    classField = layerNode.getElementsByTagName('classificationattribute')[0].childNodes[0].nodeValue.encode('utf-8')  

    # write the rendering info for each class
    class_def = "    CLASS\n"

    # Class name irrelevant for color ramps since mapserver can't render their legend
    #self.outFile.write("      NAME '" + classField + "'\n")

    # color
    lower = symbolNode.getElementsByTagName('lowestsymbol')[0].getElementsByTagName('symbol')[0]
    upper = symbolNode.getElementsByTagName('highestsymbol')[0].getElementsByTagName('symbol')[0]
    lowerColor = lower.getElementsByTagName('fillcolor')[0]
    upperColor = upper.getElementsByTagName('fillcolor')[0]

    # outline color
    outlineNode = lower.getElementsByTagName('outlinecolor')[0]

    class_def += "      STYLE\n"
    
    # The first and last color of the ramp ( r g b r g b )
    class_def += "        COLORRANGE " + lowerColor.getAttribute('red').encode('utf-8') + " " + lowerColor.getAttribute('green').encode('utf-8') + " " + lowerColor.getAttribute('blue').encode('utf-8') + " " + upperColor.getAttribute('red').encode('utf-8') + " " + upperColor.getAttribute('green').encode('utf-8') + " " + upperColor.getAttribute('blue').encode('utf-8') + "\n"

    # The range of values over which to ramp the colors
    class_def += "        DATARANGE " + lower.getElementsByTagName('lowervalue')[0].childNodes[0].nodeValue.encode('utf-8') + ' ' + upper.getElementsByTagName('lowervalue')[0].childNodes[0].nodeValue.encode('utf-8') + '\n'

    class_def += "        RANGEITEM '" + classField + "'\n"
    class_def += "      END\n"

    class_def += "      STYLE\n"
    class_def += "        OUTLINECOLOR " + outlineNode.getAttribute('red').encode('utf-8') + " " + outlineNode.getAttribute('green').encode('utf-8') + " " + outlineNode.getAttribute('blue').encode('utf-8') + "\n"
    class_def += "      END\n"

    # label
    class_def +=  self.msLabel( layerNode )

    # end of CLASS  
    class_def += "    END\n"

    return class_def
    

  # Unique value renderer output
  def uniqueRenderer(self, layerNode, symbolNode):
    # get the renderer field for building up the classes
    classField = layerNode.getElementsByTagName('classificationattribute')[0].childNodes[0].nodeValue.encode('utf-8')  

    # get the layers geometry type
    geometry = layerNode.getAttribute("geometry").encode('utf-8').upper()
    
    # write the render item
    class_def = "    CLASSITEM '" + classField + "'\n"

    # write the rendering info for each class
    classes = layerNode.getElementsByTagName('symbol')
    for cls in classes:
      class_def += "    CLASS\n"

      try:
        lower = cls.getElementsByTagName('lowervalue')[0].childNodes[0].nodeValue.encode('utf-8')
      except IndexError:
        # set to blank in the case where the field used for rendering has no value
        lower = ""

      # If there's a label use it, otherwise autogenerate one
      try:
        label = cls.getElementsByTagName('label')[0].childNodes[0].nodeValue.encode('utf-8')
        class_def += "      NAME '" + label + "'\n"
      except:
        class_def += "      NAME '" + classField + " = " + lower + "' \n"

      class_def += "      EXPRESSION '" + lower + "' \n"

      # Get the symbol name
      symbol = self.msSymbol( geometry, symbolNode )  
      
      class_def += "      STYLE\n"
      class_def += "        SYMBOL " + symbol + "\n"

      # Symbol size 
      if geometry == 'POINT' or geometry == 'LINE':
        class_def += self.getSymbolSizeString(cls)

      # outline color
      outlineNode = cls.getElementsByTagName('outlinecolor')[0]
      class_def += "         OUTLINECOLOR "  \
            + outlineNode.getAttribute('red').encode('utf-8') + ' ' \
            + outlineNode.getAttribute('green').encode('utf-8') + ' ' \
            + outlineNode.getAttribute('blue').encode('utf-8') \
            + "\n"

      # color
      colorNode = cls.getElementsByTagName('fillcolor')[0]
      class_def += "         COLOR "  \
            + colorNode.getAttribute('red').encode('utf-8') + ' ' \
            + colorNode.getAttribute('green').encode('utf-8') + ' ' \
            + colorNode.getAttribute('blue').encode('utf-8') \
            + "\n"
      class_def += "       END\n"

      # label
      class_def +=  self.msLabel( layerNode )
      
      # end of CLASS  
      class_def += "    END\n"

    return class_def
    
  # Utility method to format a proj4 text string into mapserver format
  def formatProj4(self, proj4text):
    parms = proj4text.split(" ")
    ret = ""
    for p in parms:
      p = p.replace("+","")
      ret = ret + "    '" + p + "'\n"
    return ret

  def getProj4(self, proj4text):
    """Returns the proj4 string as a dictionary with key value pairs."""
    parms = proj4text.split(" ")
    ret = {}
    for p in parms:
      p = p.replace("+","")
      keyValue = p.split("=")

      key = keyValue[0]
      
      value = ""
      try:    value = keyValue[1]
      except: value = ""

      if key != "":
        ret[key] = value
    return ret

  # Determines the symbol name and adds it to the symbol queue
  def msSymbol(self, geometry, symbolNode):
    # contains the same markup for a layer regardless of type
    # so we infer a symbol type based on the geometry
    symbolName = ''
    symbol = '0'

    if geometry == 'POLYGON':
      symbol = '0'
    elif geometry == 'LINE':
      symbol = '0'
    elif geometry == 'POINT':
      try:
        symbolName = qgis2map_symbol[symbolNode.getElementsByTagName('pointsymbol')[0].childNodes[0].nodeValue.encode('utf-8')]
      except:
        symbolName = "circle"
      # make sure it's double quoted
      symbol = "\"" + symbolName + "\""

# a symbol set in an external symbol.txt file is used; see comment on top of this file
#    if symbolName == 'CIRCLE':
#      self.symbolQueue['CIRCLE'] = """
#      #Circle symbol
#      SYMBOL
#        NAME 'CIRCLE'
#        TYPE ellipse
#        FILLED true
#        POINTS
#          1 1
#        END
#      END """
#
#    if symbolName == 'TRIANGLE':
#      self.symbolQueue['TRIANGLE'] = """
#      SYMBOL
#        NAME "TRIANGLE"
#        TYPE vector
#        FILLED true
#        POINTS
#          0 1
#         .5 0
#          1 1
#          0 1
#        END
#      END """

    return symbol

  # Label block creation
  def msLabel(self, layerNode):
    # currently a very basic bitmap font
    labelNode = layerNode.getElementsByTagName('labelattributes')[0]
    #labelField = labelNode.getElementsByTagName('label')[0].getAttribute('field').encode('utf-8')
    # why was the attribute 'field' and not 'fieldname'?
    labelField = labelNode.getElementsByTagName('label')[0].getAttribute('fieldname').encode('utf-8')
    if labelField != '' and labelField is not None:
      labelBlock  = "     LABEL \n"

      # see comment at 'qgis2ms_fontset'
      fontQgis = labelNode.getElementsByTagName('family')[0].getAttribute('name').encode('utf-8')
      fontMs = ""
      try:
        fontMs = qgis2map_fontset[fontQgis]
      except:
        # we default to the first font in the fontset, if any are present
        if len(qgis2map_fontset) > 0:
          try:
            fontMs = qgis2map_fontset["MS Shell Dialog 2"]
          except:
            sortedKeys = qgis2map_fontset.keys()
            sortedKeys.sort()
            fontMs = qgis2map_fontset[sortedKeys[0]]
        else:
          fontMs = ""
          
      bold = bool(int(labelNode.getElementsByTagName('bold')[0].getAttribute('on').encode("utf-8")))
      italic = bool(int(labelNode.getElementsByTagName('italic')[0].getAttribute('on').encode("utf-8")))

      # "-bold" and "-italic" must correspond with the fontset file
      # font can be both bold and italic
      labelBlock += "      FONT " + fontMs
      if bold:   labelBlock += "-bold"
      if italic: labelBlock += "-italic" 
      labelBlock += "\n"

      labelBlock += "      TYPE truetype\n"

      size = self.getFieldName(labelNode, 'size')
      if size == "":
        sizeNode = labelNode.getElementsByTagName('size')[0]
        units = sizeNode.getAttribute("units").encode("utf-8")
        sizeValue = int(sizeNode.getAttribute("value").encode("utf-8"))
        # we must convert to px for use in the mapfile
        sizePx = 11 # default
        sizePx = sizeValue
        #if units == "pt":   sizePx = int(sizeValue / 0.75)
        # TODO: find appropriate conversion metric from map units to pixels
        #elif unit == "mu":
        #proj4Elem = labelNode.parentNode.getElementsByTagName("proj4")[0].childNodes[0]
        #proj4str = proj4Elem.nodeValue.encode('utf-8')
        #proj4Dict = self.getProj4(proj4str)
        #for i,j in proj4Dict.iteritems():
          #labelBlock += str(i) + ":: " + str(j) + "\n"
        #
        #sizePx = ?????  proj4Dict["units"] ??
        # non-used unit types:
        #elif unit == "em": sizePx = int(size * 16.0)
        #elif unit == "px": sizePx = size
        size = str(sizePx)
      labelBlock += "      SIZE " + size + "\n"

      color = self.getFieldName(labelNode, 'color')
      if color == "":
        colorNode = labelNode.getElementsByTagName('color')[0]
        r = int(colorNode.getAttribute("red"))
        g = int(colorNode.getAttribute("green"))
        b = int(colorNode.getAttribute("blue"))
        color = str(r) + " " + str(g) + " " + str(b)
      labelBlock += "      COLOR " + color + "\n"
 
      # Include label angle if specified
      # Note that angles only work for truetype fonts
      angle = self.getFieldName(labelNode, 'angle')
      if angle == "":
        angle = labelNode.getElementsByTagName('angle')[0].getAttribute('value').encode('utf-8')
      labelBlock += "      ANGLE " + angle + "\n"
     
      # Include label buffer if specified
      # Note that the buffer has different meaning in qgis vs mapserver
      # mapserver just adds blank space around the label while
      # qgis uses a fill color around the label
      # Note that buffer only works for truetype fonts
      buffer = labelNode.getElementsByTagName('buffersize')[0].getAttribute('value').encode('utf-8')
      labelBlock += "      BUFFER " + buffer + "\n"

      # alignment in QGis corresponds to position in MapServer
      alignment = labelNode.getElementsByTagName('alignment')[0].getAttribute('value').encode('utf-8')
      try:
        labelBlock += "      POSITION " + qgis2map_aligment2position[alignment] + "\n"
      except:
        # default to center if we encounter a nonsensical value
        labelBlock += "      POSITION cc\n"

      #values from the gui:
      labelBlock += "      FORCE " + self.force + "\n"
      labelBlock += "      ANTIALIAS " + self.antialias + "\n"
      labelBlock += "      PARTIALS " + self.partials + "\n"

      labelBlock += "     END \n"
      return labelBlock
    else:
      return ''


  def getFieldName(self, parentNode, nodeName):
    """ Returns the fieldname-attribute-value of a nodeName with a given parentNode
    as a string surrounded by brackets ('[' and ']') or
    an empty string if the fieldname-attribute does not exist."""
    try:
      fieldname = parentNode.getElementsByTagName(nodeName)[0].getAttribute('fieldname').encode('utf-8')
      if fieldname != "":
        return "[" + fieldname + "]"
      else:
        return ""
    except:
      return ""


