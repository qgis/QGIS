# This class exports a QGIS project file to a mapserver .map file.
# All the work is done in the writeMapFile method. The msexport binary
# presents a Qt based GUI that collects the needed information for this
# script. 
#
# CHANGES SHOULD NOT BE MADE TO THE writeMapFile METHOD UNLESS YOU
# ARE CHANGING THE QgsMapserverExport CLASS AND YOU KNOW WHAT YOU ARE
# DOING
import sys, string
from xml.dom import minidom, Node

# symbol map
qgisSymbols = {'hard:circle': 'CIRCLE'}
class Qgis2Map:
  def __init__(self, projectFile, mapFile):
    self.project = projectFile
    self.mapFile = mapFile
    # create the DOM 
    self.qgs = minidom.parse(projectFile)
    # init the other members that are not set by the constructor
    self.units = ''
    self.imageType = ''
    self.mapName = ''
    self.width = ''
    self.height = ''
    self.minScale = ''
    self.maxScale = ''
    self.template = ''
    self.header = ''
    self.footer = ''


# Set the options collected from the GUI
  def setOptions(self, units, image, mapname, width, height, minscale, maxscale, template, header, footer):
    self.units = units
    self.imageType = image
    self.mapName = mapname
    self.width = width
    self.height = height
    self.minScale = minscale
    self.maxScale = maxscale
    self.template = template
    self.header = header
    self.footer = footer
#
## All real work happens here by calling methods to write the
## various sections of the map file
  def writeMapFile(self):
    # open the output file
    self.outFile = open(self.mapFile, 'w')
    # write the general map and web settings
    self.writeMapSection()
    # write the projection section
    self.writeProjectionSection()
    # write the output format section
    self.writeOutputFormat()
    # write the legend section
    self.writeLegendSection()
    # write the symbol defs section
    self.writeSymbolSection()

    # write the WEB section
    self.writeWebSection()

    # write the LAYER sections
    self.writeMapLayers()

    # END and close the map file
    self.outFile.write("END")
    self.outFile.close()

    ret = "Writing the map file using " + self.project + " " + self.mapFile
    return ret
 #, self.mapFile, self.project

# Write the general parts of the map section
  def writeMapSection(self):
    print "Writing header\n"
    self.outFile.write("# Map file created from QGIS project file " + self.project + "\n")
    self.outFile.write("# Edit this file to customize for your map interface\n")
    self.outFile.write("MAP\n")
    self.outFile.write("  NAME " + self.mapName + "\n")
    self.outFile.write("  # Map image size\n")
    self.outFile.write("  SIZE " + self.width + " " + self.height + "\n")
    self.outFile.write("  UNITS " + self.units.lower() + "\n")
    self.outFile.write("\n")
    # extents
    xmin = self.qgs.getElementsByTagName("xmin")
    self.outFile.write("  EXTENT ")
    self.outFile.write(xmin[0].childNodes[0].nodeValue.encode())
    self.outFile.write(" ")
    ymin = self.qgs.getElementsByTagName("ymin")
    self.outFile.write(ymin[0].childNodes[0].nodeValue.encode())
    self.outFile.write(" ")
    xmax = self.qgs.getElementsByTagName("xmax")
    self.outFile.write(xmax[0].childNodes[0].nodeValue.encode())
    self.outFile.write(" ")
    ymax = self.qgs.getElementsByTagName("ymax")
    self.outFile.write(ymax[0].childNodes[0].nodeValue.encode())
    self.outFile.write("\n")

# Write the OUTPUTFORMAT section
  def writeOutputFormat(self):
    self.outFile.write("  # Background color for the map canvas -- change as desired\n")
    self.outFile.write("  IMAGECOLOR 192 192 192\n")
    self.outFile.write("  IMAGEQUALITY 95\n")
    self.outFile.write("  IMAGETYPE " + self.imageType + "\n")
    #self.outFile.write("  OUTPUTFORMAT\n")
    #self.outFile.write("    NAME " + self.imageType + "\n")
    #self.outFile.write("    DRIVER 'GD/" + self.imageType.upper() + "'\n")
    #self.outFile.write("    MIMETYPE 'image/" + self.imageType.lower() + "'\n")
    #self.outFile.write("    IMAGEMODE PC256\n")
    #self.outFile.write("    EXTENSION '" + self.imageType.lower() + "'\n")
    #self.outFile.write("  END\n")
    

# Write Projection section
  def writeProjectionSection(self):
    # Need to get the destination srs from one of the map layers since
    # the project file doesn't contain the epsg id or proj4 text for 
    # the map apart from that defined in each layer

    self.outFile.write("  PROJECTION\n")

    # Get the proj4 text from the first map layer's destination SRS
    proj4Text = self.qgs.getElementsByTagName("destinationsrs")[0].getElementsByTagName("proj4")[0].childNodes[0].nodeValue.encode() 
    # the proj4 text string needs to be reformatted to make mapserver happy
    self.outFile.write(self.formatProj4(proj4Text))

    self.outFile.write("  END\n")

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
    self.outFile.write("    END\n\n")
    
# Write the symbol definitions
  def writeSymbolSection(self):
    self.outFile.write("  #Circle symbol\n");
    self.outFile.write("  SYMBOL\n");
    self.outFile.write("   NAME 'CIRCLE'\n");
    self.outFile.write("   TYPE ellipse\n");
    self.outFile.write("   FILLED true\n");
    self.outFile.write("   POINTS\n");
    self.outFile.write("    1 1\n");
    self.outFile.write("   END\n");
    self.outFile.write("  END\n\n");


# Write the WEB section of the map file
  def writeWebSection(self):
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

    # TODO allow user to configure this
    self.outFile.write("    # WMS server settings\n")
    self.outFile.write("    METADATA\n")
    self.outFile.write("      'wms_title'           '" + self.mapName + "'\n")
    self.outFile.write("      'wms_onlineresource'  'http://my.host.com/cgi-bin/mapserv?map=wms.map&'\n")
    self.outFile.write("      'wms_srs'             'EPSG:4326'\n")
    self.outFile.write("    END\n")

    self.outFile.write("    # Template and header/footer settings\n")
    self.outFile.write("    # Only the template parameter is required to display a map. See MapServer documentation\n")
    
    if self.template != "":
      self.outFile.write("    TEMPLATE '" + self.template + "'\n")
    if self.header != "":
      self.outFile.write("    HEADER '" + self.header + "'\n")
    if self.footer != "":
      self.outFile.write("    FOOTER '" + self.footer + "'\n")
    self.outFile.write("  END\n")

  def parsePostgisConnection( self, dataString ):
    pg = {}
    pg['host'] = 'localhost'
    pg['dbname'] = 'gisdata'
    pg['user'] = ''
    pg['password'] = ''
    pg['table'] = ''
    pg['geom'] = 'the_geom'
    
    cmp = dataString.split(" ")
    for c in cmp:
      if c[:1] == "(":
        pg['geom'] = c[1:][:-1]
      else:
        kvp = c.split("=")
        pg[kvp[0]] =  kvp[1]

    return pg

       
# Write the map layers
  def writeMapLayers(self):
    # get the list of maplayer nodes
    maplayers = self.qgs.getElementsByTagName("maplayer")
    print "Processing ", len(maplayers), " layers"
    count = 0
    for lyr in maplayers:
      count += 1
      print "Processing layer ", count 
      # The attributes of the maplayer tag contain the scale dependent settings,
      # visibility, and layer type

      self.outFile.write("  LAYER\n")
      # write the name of the layer
      self.outFile.write("    NAME '" + lyr.getElementsByTagName("layername")[0].childNodes[0].nodeValue.encode() + "'\n")
      if lyr.getAttribute("type").encode() == 'vector':  
        self.outFile.write("    TYPE " + lyr.getAttribute("geometry").encode().upper() + "\n")
      elif lyr.getAttribute("type").encode() == 'raster':  
        self.outFile.write("    TYPE " + lyr.getAttribute("type").encode().upper() + "\n")

      # data
      dataString = lyr.getElementsByTagName("datasource")[0].childNodes[0].nodeValue.encode()

      # test if it is a postgis, grass or WMS layer
      # is there a better way to do this? probably.
      try:
        providerString = lyr.getElementsByTagName("provider")[0].childNodes[0].nodeValue.encode()
      except:
        # if providerString is null
        providerString = ''

      if providerString == 'postgres':
        # it's a postgis layer
        pg = self.parsePostgisConnection(dataString)
        self.outFile.write("    CONNECTIONTYPE postgis\n")
        self.outFile.write("    CONNECTION 'host=" + pg['host'] + " dbname=" + pg['dbname'] + " password=" + pg['password'] + " user=" + pg['user'] + "'\n")
        self.outFile.write("    DATA '" + pg['geom'] + " FROM " + pg['table'] + "'\n")
      elif providerString == 'wms' and lyr.getAttribute("type").encode().upper() == 'RASTER':
        # it's a WMS layer 
        self.outFile.write("    CONNECTIONTYPE WMS\n")
        self.outFile.write("    CONNECTION '" + dataString + "'\n")
        rasterProp = lyr.getElementsByTagName("rasterproperties")[0]

        # TODO loop thru wmsSubLayers  
        # wmsSubLayers = rasterProp.getElementsByTagName('wmsSublayer')
        layername = rasterProp.getElementsByTagName('wmsSublayer')[0].getElementsByTagName('name')[0].childNodes[0].nodeValue.encode()
        try: 
          style = rasterProp.getElementsByTagName('wmsSublayer')[0].getElementsByTagName('style')[0].childNodes[0].nodeValue.encode()
        except:
          style = ''

        # Create necesssary wms metadata
        format = rasterProp.getElementsByTagName('wmsFormat')[0].childNodes[0].nodeValue.encode()
        srs = lyr.getElementsByTagName('coordinatetransform')[0].getElementsByTagName('sourcesrs')[0].getElementsByTagName('spatialrefsys')[0]
        epsg = srs.getElementsByTagName('epsg')[0].childNodes[0].nodeValue.encode()
        self.outFile.write("    METADATA\n")
        self.outFile.write("      'wms_name' '" + layername + "'\n")
        self.outFile.write("      'wms_server_version' '1.1.1'\n")
        self.outFile.write("      'wms_srs' 'EPSG:" + epsg + "'\n")
        self.outFile.write("      'wms_format' '" + format + "'\n")
        self.outFile.write("      'wms_style' '" + style + "'\n")
        self.outFile.write("    END\n")
      else: 
        # its a standard ogr, gdal or grass layer
        self.outFile.write("    DATA '" + dataString + "'\n")
      
      # WMS settings for all layers
      self.outFile.write("    METADATA\n")
      self.outFile.write("      'wms_title' '" + lyr.getElementsByTagName("layername")[0].childNodes[0].nodeValue.encode() + "'\n")
      self.outFile.write("    END\n")

      self.outFile.write("    STATUS DEFAULT\n")
      opacity = int ( 100.0 * float(lyr.getElementsByTagName("transparencyLevelInt")[0].childNodes[0].nodeValue.encode()) / 255.0 ) 
      self.outFile.write("    TRANSPARENCY " + str(opacity) + "\n")
      self.outFile.write("    PROJECTION\n")
      proj4Text = lyr.getElementsByTagName("proj4")[0].childNodes[0].nodeValue.encode() 
      self.outFile.write(self.formatProj4(proj4Text))
      self.outFile.write("    END\n")
      scaleDependent = lyr.getAttribute("scaleBasedVisibilityFlag").encode()
      if scaleDependent == '1':
        # get the min and max scale settings
        minscale = lyr.getAttribute("minScale").encode()
        maxscale = lyr.getAttribute("maxScale").encode()
        if minscale > '':
          self.outFile.write("    MINSCALE " + minscale + "\n")
        if maxscale > '':
          self.outFile.write("    MAXSCALE " + maxscale + "\n")

      
      # Check for label field (ie LABELITEM) and label status
      try:
        labelOn = lyr.getElementsByTagName("label")[0].childNodes[0].nodeValue.encode()
        labelNode = lyr.getElementsByTagName('labelattributes')[0]
        labelField = labelNode.getElementsByTagName('label')[0].getAttribute('field').encode()
        if labelField != '' and labelField is not None and labelOn == "1":
          self.outFile.write("    LABELITEM '" + labelField + "'\n");
      except:
        # no labels
        pass
      
      # write the CLASS section for rendering
      # First see if there is a single symbol renderer
      if lyr.getElementsByTagName("singlesymbol").length > 0:
        symbolNode = lyr.getElementsByTagName("singlesymbol")[0].getElementsByTagName('symbol')[0] 
        self.simpleRenderer(lyr, symbolNode)
      elif lyr.getElementsByTagName("graduatedsymbol").length > 0:
        self.graduatedRenderer(lyr, lyr.getElementsByTagName("graduatedsymbol")[0].getElementsByTagName('symbol')[0] )
      elif lyr.getElementsByTagName("continuoussymbol").length > 0:
        self.continuousRenderer(lyr, lyr.getElementsByTagName("continuoussymbol")[0] )
      elif lyr.getElementsByTagName("uniquevalue").length > 0:
        self.uniqueRenderer(lyr, lyr.getElementsByTagName("uniquevalue")[0].getElementsByTagName('symbol')[0] )

      # end of LAYER
      self.outFile.write("  END\n\n")

  # Simple renderer ouput
  # We need the layer node and symbol node
  def simpleRenderer(self, layerNode, symbolNode):
    # symbology depends on the feature type and the .qgs file
        # contains the same markup for a layer regardless of type
        # so we infer a symbol type based on the geometry
        geometry = layerNode.getAttribute("geometry").encode().upper()
        if geometry == 'POLYGON':
          symbol = '0'
        elif geometry == 'LINE':
          symbol = '0'
        elif geometry == 'POINT':
          symbolName = qgisSymbols[symbolNode.getElementsByTagName('pointsymbol')[0].childNodes[0].nodeValue.encode()]
          # make sure it's single quoted
          symbol = "'" + symbolName + "'"
      
        self.outFile.write("    CLASS\n")
        # use the point symbol map to lookup the mapserver symbol type
        self.outFile.write("       SYMBOL " + symbol + " \n")
        self.outFile.write("       SIZE " 
            + symbolNode.getElementsByTagName('pointsize')[0].childNodes[0].nodeValue.encode()  
           + " \n")
        # outline color
        outlineNode = symbolNode.getElementsByTagName('outlinecolor')[0]
        self.outFile.write("       OUTLINECOLOR " 
            + outlineNode.getAttribute('red') + ' '
            + outlineNode.getAttribute('green') + ' '
            + outlineNode.getAttribute('blue')
            + "\n")
        # color
        colorNode = symbolNode.getElementsByTagName('fillcolor')[0]
        self.outFile.write("       COLOR " 
            + colorNode.getAttribute('red') + ' '
            + colorNode.getAttribute('green') + ' '
            + colorNode.getAttribute('blue')
            + "\n")

        # label
        # currently a very basic bitmap font
        # need to extend for font, size, color, buffer, alignment, etc
        labelNode = layerNode.getElementsByTagName('labelattributes')[0]
        labelField = labelNode.getElementsByTagName('label')[0].getAttribute('field').encode()
        if labelField != '' and labelField is not None:
          self.outFile.write("     LABEL \n");
          self.outFile.write("      SIZE medium\n");
          self.outFile.write("      COLOR 0 0 0 \n");
          self.outFile.write("     END \n");

        # end of CLASS  
        self.outFile.write("    END\n")
        

# Graduated symbol renderer output
  def graduatedRenderer(self, layerNode, symbolNode):
    # get the renderer field for building up the classes
    classField = layerNode.getElementsByTagName('classificationattribute')[0].childNodes[0].nodeValue.encode()  
    # write the render item
    self.outFile.write("    CLASSITEM '" + classField + "'\n")

    # write the rendering info for each class
    classes = layerNode.getElementsByTagName('symbol')
    for cls in classes:
      self.outFile.write("    CLASS\n")
      lower = cls.getElementsByTagName('lowervalue')[0].childNodes[0].nodeValue.encode()
      upper = cls.getElementsByTagName('uppervalue')[0].childNodes[0].nodeValue.encode()
      self.outFile.write("      EXPRESSION ( ([" + classField + "] >= " + lower + ") AND ([" + classField + "] <= " + upper + ") )\n") 

      # contains the same markup for a layer regardless of type
      # so we infer a symbol type based on the geometry
      geometry = layerNode.getAttribute("geometry").encode().upper()
      if geometry == 'POLYGON':
        symbol = '0'
      elif geometry == 'LINE':
        symbol = '0'
      elif geometry == 'POINT':
        symbolName = qgisSymbols[symbolNode.getElementsByTagName('pointsymbol')[0].childNodes[0].nodeValue.encode()]
        # make sure it's single quoted
        symbol = "'" + symbolName + "'"
      self.outFile.write("      SYMBOL " + symbol + "\n")

      # Symbol size 
      if geometry == 'POINT' or geometry == 'LINE':
        self.outFile.write("      SIZE " 
            + cls.getElementsByTagName('pointsize')[0].childNodes[0].nodeValue.encode()  
            + " \n")

      # outline color
      outlineNode = cls.getElementsByTagName('outlinecolor')[0]
      self.outFile.write("       OUTLINECOLOR " 
            + outlineNode.getAttribute('red') + ' '
            + outlineNode.getAttribute('green') + ' '
            + outlineNode.getAttribute('blue')
            + "\n")
      # color
      colorNode = cls.getElementsByTagName('fillcolor')[0]
      self.outFile.write("       COLOR " 
            + colorNode.getAttribute('red') + ' '
            + colorNode.getAttribute('green') + ' '
            + colorNode.getAttribute('blue')
            + "\n")

      # label
      # currently a very basic bitmap font
      # need to extend for font, size, color, buffer, alignment, etc
      labelNode = layerNode.getElementsByTagName('labelattributes')[0]
      labelField = labelNode.getElementsByTagName('label')[0].getAttribute('field').encode()
      if labelField != '' and labelField is not None:
        self.outFile.write("     LABEL \n");
        self.outFile.write("      SIZE medium\n");
        self.outFile.write("      COLOR 0 0 0 \n");
        self.outFile.write("     END \n");

      # end of CLASS  
      self.outFile.write("    END\n")

# Continuous symbol renderer output
  def continuousRenderer(self, layerNode, symbolNode):
    # get the renderer field for building up the classes
    classField = layerNode.getElementsByTagName('classificationattribute')[0].childNodes[0].nodeValue.encode()  

    # write the rendering info for each class
    self.outFile.write("    CLASS\n")
    
    lower = symbolNode.getElementsByTagName('lowestsymbol')[0].getElementsByTagName('symbol')[0]
    upper = symbolNode.getElementsByTagName('highestsymbol')[0].getElementsByTagName('symbol')[0]

    # color
    lowerColor = lower.getElementsByTagName('fillcolor')[0]
    upperColor = upper.getElementsByTagName('fillcolor')[0]

    # outline color
    outlineNode = lower.getElementsByTagName('outlinecolor')[0]

    self.outFile.write("      STYLE\n")
    self.outFile.write("        COLORRANGE " 
          + lowerColor.getAttribute('red') + " " 
          + lowerColor.getAttribute('green') + " " 
          + lowerColor.getAttribute('blue') + " " 
          + upperColor.getAttribute('red') + " " 
          + upperColor.getAttribute('green') + " " 
          + upperColor.getAttribute('blue') + "\n")
    # QGIS project file is a bit unclear...
    # lower.lowervalue <->  upper.lowervalue
    # upper.LOWER value .. hmm ...
    self.outFile.write("        DATARANGE "
         + lower.getElementsByTagName('lowervalue')[0].childNodes[0].nodeValue.encode() + ' '
         + upper.getElementsByTagName('lowervalue')[0].childNodes[0].nodeValue.encode() + '\n')
    self.outFile.write("        RANGEITEM '" + classField + "'\n")                                        
    self.outFile.write("      END\n")

    self.outFile.write("      STYLE\n")
    self.outFile.write("        OUTLINECOLOR "
          + outlineNode.getAttribute('red') + " " 
          + outlineNode.getAttribute('green') + " " 
          + outlineNode.getAttribute('blue') + "\n") 
    self.outFile.write("      END\n")

    # label
    # currently a very basic bitmap font
    # need to extend for font, size, color, buffer, alignment, etc
    labelNode = layerNode.getElementsByTagName('labelattributes')[0]
    labelField = labelNode.getElementsByTagName('label')[0].getAttribute('field').encode()
    if labelField != '' and labelField is not None:
      self.outFile.write("     LABEL \n");
      self.outFile.write("      SIZE medium\n");
      self.outFile.write("      COLOR 0 0 0 \n");
      self.outFile.write("     END \n");

    # end of CLASS  
    self.outFile.write("    END\n")
    

# Unique value renderer output
  def uniqueRenderer(self, layerNode, symbolNode):
    # get the renderer field for building up the classes
    classField = layerNode.getElementsByTagName('classificationattribute')[0].childNodes[0].nodeValue.encode()  
    # write the render item
    self.outFile.write("    CLASSITEM '" + classField + "'\n")

    # write the rendering info for each class
    classes = layerNode.getElementsByTagName('symbol')
    for cls in classes:
      self.outFile.write("    CLASS\n")

      lower = cls.getElementsByTagName('lowervalue')[0].childNodes[0].nodeValue.encode()
      self.outFile.write("      EXPRESSION '" + lower + "' \n") 

      # contains the same markup for a layer regardless of type
      # so we infer a symbol type based on the geometry
      geometry = layerNode.getAttribute("geometry").encode().upper()
      if geometry == 'POLYGON':
        symbol = '0'
      elif geometry == 'LINE':
        symbol = '0'
      elif geometry == 'POINT':
        symbolName = qgisSymbols[symbolNode.getElementsByTagName('pointsymbol')[0].childNodes[0].nodeValue.encode()]
        # make sure it's single quoted
        symbol = "'" + symbolName + "'"
      self.outFile.write("      SYMBOL " + symbol + "\n")

      # Symbol size 
      if geometry == 'POINT' or geometry == 'LINE':
        self.outFile.write("      SIZE " 
            + cls.getElementsByTagName('pointsize')[0].childNodes[0].nodeValue.encode()  
            + " \n")

      # outline color
      outlineNode = cls.getElementsByTagName('outlinecolor')[0]
      self.outFile.write("       OUTLINECOLOR " 
            + outlineNode.getAttribute('red') + ' '
            + outlineNode.getAttribute('green') + ' '
            + outlineNode.getAttribute('blue')
            + "\n")
      # color
      colorNode = cls.getElementsByTagName('fillcolor')[0]
      self.outFile.write("       COLOR " 
            + colorNode.getAttribute('red') + ' '
            + colorNode.getAttribute('green') + ' '
            + colorNode.getAttribute('blue')
            + "\n")

      # label
      # currently a very basic bitmap font
      # need to extend for font, size, color, buffer, alignment, etc
      labelNode = layerNode.getElementsByTagName('labelattributes')[0]
      labelField = labelNode.getElementsByTagName('label')[0].getAttribute('field').encode()
      if labelField != '' and labelField is not None:
        self.outFile.write("     LABEL \n");
        self.outFile.write("      SIZE medium\n");
        self.outFile.write("      COLOR 0 0 0 \n");
        self.outFile.write("     END \n");

      # end of CLASS  
      self.outFile.write("    END\n")
    
# Utility method to format a proj4 text string into mapserver format
  def formatProj4(self, proj4text):
    parms = proj4text.split(" ")
    ret = ""
    for p in parms:
      p = p.replace("+","")
      ret = ret + "    '" + p + "'\n"
    return ret

