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
#
# CHANGES SHOULD NOT BE MADE TO THE writeMapFile METHOD UNLESS YOU
# ARE CHANGING THE QgsMapserverExport CLASS AND YOU KNOW WHAT YOU ARE
# DOING
import sys 
import os
from string import *
from xml.dom import minidom, Node
from qgis.core import *

# symbol map
qgisSymbols = {'hard:circle'   : 'CIRCLE',
               'hard:triangle' : 'TRIANGLE'}

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
    self.minimumScale = ''
    self.maximumScale = ''
    self.template = ''
    self.header = ''
    self.footer = ''
    self.symbolQueue = {}
    

  # Set the options collected from the GUI
  def setOptions(self, units, image, mapname, width, height, template, header, footer):
    self.units = units.encode('utf-8')
    self.imageType = image.encode('utf-8')
    self.mapName = mapname.encode('utf-8')
    self.width = width.encode('utf-8')
    self.height = height.encode('utf-8')
    #self.minimumScale = minscale
    #self.maximumScale = maxscale
    self.template = template.encode('utf-8')
    self.header = header.encode('utf-8')
    self.footer = footer.encode('utf-8')
    #print units, image, mapname, width, height, template, header, footer

  ## All real work happens here by calling methods to write the
  ## various sections of the map file
  def writeMapFile(self):
    # open the output file
    print "creating the map file"
    self.outFile = open(self.mapFile, 'w')
    # write the general map and web settings
    print " --- python : map section "
    self.writeMapSection()
    logmsg =  "Wrote map section\n"
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
    self.writeWebSection()
    logmsg += "Wrote web section\n"
    print " --- python : web section done"

    # write the LAYER sections
    print " --- python : layer section "
    self.writeMapLayers()
    logmsg += "Wrote map layers\n"
    print " --- python : layer section done"

    # write the symbol defs section
    # must happen after layers so we can build a symbol queue
    print " --- python : symbol section "
    self.writeSymbolSection()
    logmsg += "Wrote symbol section\n"
    print " --- python : symbol section done"

    # END and close the map file
    self.outFile.write("END")
    self.outFile.close()

    logmsg += "Map file completed for " + self.project + "\n"
    logmsg += "Map file saved as " + self.mapFile + "\n"
    return logmsg

  # Write the general parts of the map section
  def writeMapSection(self):
    self.outFile.write("# Map file created from QGIS project file " + self.project.encode('utf-8') + "\n")
    self.outFile.write("# Edit this file to customize for your map interface\n")
    self.outFile.write("# (Created with PyQgis MapServer Export plugin)\n")
    self.outFile.write("MAP\n")
    self.outFile.write("  NAME " + self.mapName + "\n")
    self.outFile.write("  # Map image size\n")
    self.outFile.write("  SIZE " + self.width + " " + self.height + "\n")
    self.outFile.write("  UNITS %s\n" % (self.units))
    self.outFile.write("\n")
    # extents
    xmin = self.qgs.getElementsByTagName("xmin")
    self.outFile.write("  EXTENT ")
    self.outFile.write(xmin[0].childNodes[0].nodeValue.encode('utf-8'))
    self.outFile.write(" ")
    ymin = self.qgs.getElementsByTagName("ymin")
    self.outFile.write(ymin[0].childNodes[0].nodeValue.encode('utf-8'))
    self.outFile.write(" ")
    xmax = self.qgs.getElementsByTagName("xmax")
    self.outFile.write(xmax[0].childNodes[0].nodeValue.encode('utf-8'))
    self.outFile.write(" ")
    ymax = self.qgs.getElementsByTagName("ymax")
    self.outFile.write(ymax[0].childNodes[0].nodeValue.encode('utf-8'))
    self.outFile.write("\n")

  # Write the OUTPUTFORMAT section
  def writeOutputFormat(self):
    self.outFile.write("  # Background color for the map canvas -- change as desired\n")
    self.outFile.write("  IMAGECOLOR 192 192 192\n")
    self.outFile.write("  IMAGEQUALITY 95\n")
    self.outFile.write("  IMAGETYPE " + self.imageType + "\n")
    self.outFile.write("  OUTPUTFORMAT\n")
    self.outFile.write("    NAME " + self.imageType + "\n")
    self.outFile.write("    DRIVER 'GD/" + self.imageType.upper() + "'\n")
    self.outFile.write("    MIMETYPE 'image/" + lower(self.imageType) + "'\n")
    self.outFile.write("    #IMAGEMODE PC256\n")
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
    
  # Write the symbol definitions
  def writeSymbolSection(self):
    for symbol in self.symbolQueue.keys():
      self.outFile.write( self.symbolQueue[symbol] )
      self.outFile.write( "\n" )

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
    self.outFile.write("      'wms_title'           '\"" + self.mapName + "\"'\n")
    self.outFile.write("      'wms_onlineresource'  'http://my.host.com/cgi-bin/mapserv?map=wms.map&'\n")
    self.outFile.write("      'wms_srs'             'EPSG:4326'\n")
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

  # Write the map layers - we have to defer writing to disk so we
  # can invert the order of the layes, since they are opposite in QGIS 
  # compared to mapserver
  def writeMapLayers(self):
    # get the list of legend nodes so the layers can be written in the
    # proper order
    legend_nodes = self.qgs.getElementsByTagName("legendlayer")
    self.z_order = list()
    for legend_node in legend_nodes:
        self.z_order.append(legend_node.getAttribute("name").encode('utf-8').replace("\"", ""))

    # get the list of maplayer nodes
    maplayers = self.qgs.getElementsByTagName("maplayer")
    print "Processing ", len(maplayers), " layers"
    count = 0
    layer_list = dict()
    for lyr in maplayers:
      count += 1
      print "Processing layer ", count 
      # The attributes of the maplayer tag contain the scale dependent settings,
      # visibility, and layer type
      layer_def = "  LAYER\n"
      # store name of the layer
      layer_name = lyr.getElementsByTagName("layername")[0].childNodes[0].nodeValue.encode('utf-8').replace("\"", "")
      # first check to see if there is a name
      if len(lyr.getElementsByTagName("layername")[0].childNodes) > 0:
        layer_def += "    NAME '" + lyr.getElementsByTagName("layername")[0].childNodes[0].nodeValue.encode('utf-8').replace("\"", "") + "'\n"
      else:
        # if no name for the layer, manufacture one  
        layer_def += "    NAME 'LAYER%s'\n" % count

      if lyr.getAttribute("type").encode('utf-8') == 'vector':  
        layer_def += "    TYPE " + lyr.getAttribute("geometry").encode('utf-8').upper() + "\n"
      elif lyr.getAttribute("type").encode('utf-8') == 'raster':  
        layer_def += "    TYPE " + lyr.getAttribute("type").encode('utf-8').upper() + "\n"
 
      # Set min/max scales
      if lyr.getAttribute('hasScaleBasedVisibilityFlag').encode('utf-8') == 1:
        layer_def += "    MINSCALE " + lyr.getAttribute('minimumScale').encode('utf-8') + "\n"
        layer_def += "    MAXSCALE " + lyr.getAttribute('maximumScale').encode('utf-8') + "\n"

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
        layer_def += "    CONNECTION \"" + uri.connectionInfo() + "\"\n"
        layer_def += "    DATA '\"" + uri.geometryColumn() + "\" FROM " + uri.quotedTablename() + "'\n"
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
        layer_def += "      'wms_name' '" + ','.join(wmsNames) + "'\n"
        layer_def += "      'wms_server_version' '1.1.1'\n"
        try:
          ct = lyr.getElementsByTagName('coordinatetransform')[0]
          srs = ct.getElementsByTagName('sourcesrs')[0].getElementsByTagName('spatialrefsys')[0]
          epsg = srs.getElementsByTagName('epsg')[0].childNodes[0].nodeValue.encode('utf-8')
          layer_def += "      'wms_srs' 'EPSG:4326 EPSG:" + epsg + "'\n"
        except:
	  pass
        layer_def += "      'wms_format' '" + format + "'\n"
        layer_def += "      'wms_style' '" + ','.join(wmsStyles) + "'\n"
        layer_def += "    END\n"

      else: 
        # its a standard ogr, gdal or grass layer
        layer_def += "    DATA '" + dataString + "'\n"
      
      # WMS settings for all layers
      layer_def += "    METADATA\n"
      layer_def += "      'wms_title' '" + lyr.getElementsByTagName("layername")[0].childNodes[0].nodeValue.encode('utf-8').replace("\"", "") + "'\n"
      layer_def += "    END\n"

      layer_def += "    STATUS DEFAULT\n"

      opacity = int ( 100.0 * 
           float(lyr.getElementsByTagName("transparencyLevelInt")[0].childNodes[0].nodeValue.encode('utf-8')) / 255.0 ) 
      layer_def += "    TRANSPARENCY " + str(opacity) + "\n"

      layer_def += "    PROJECTION\n"
      # Get the destination srs for this layer and use it to create
      # the projection section
      destsrs = self.qgs.getElementsByTagName("destinationsrs")[0] 
      proj4Text = destsrs.getElementsByTagName("proj4")[0].childNodes[0].nodeValue.encode('utf-8') 
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
        labelOn    = lyr.getElementsByTagName(     "label")[0].childNodes[0].nodeValue.encode('utf-8')
        labelField = lyr.getElementsByTagName("labelfield")[0].childNodes[0].nodeValue.encode('utf-8')
        if labelField != '' and labelField is not None and labelOn == "1":
          layer_def += "    LABELITEM '" + labelField + "'\n"
      except:
        # no labels
        pass
      
      # write the CLASS section for rendering
      # First see if there is a single symbol renderer
      if lyr.getElementsByTagName("singlesymbol").length > 0:
        symbolNode = lyr.getElementsByTagName("singlesymbol")[0].getElementsByTagName('symbol')[0] 
        layer_def += self.simpleRenderer(lyr, symbolNode)
      elif lyr.getElementsByTagName("graduatedsymbol").length > 0:
        layer_def += self.graduatedRenderer(lyr, lyr.getElementsByTagName("graduatedsymbol")[0].getElementsByTagName('symbol')[0] )
      elif lyr.getElementsByTagName("continuoussymbol").length > 0:
        layer_def += self.continuousRenderer(lyr, lyr.getElementsByTagName("continuoussymbol")[0] )
      elif lyr.getElementsByTagName("uniquevalue").length > 0:
        layer_def += self.uniqueRenderer(lyr, lyr.getElementsByTagName("uniquevalue")[0].getElementsByTagName('symbol')[0] )

      # end of LAYER
      layer_def += "  END\n\n"

      # add the layer to the list
      layer_list[layer_name] = layer_def
    # all layers have been processed, reverse the list and write
    # not necessary since z-order is mapped by the legend list order
    self.z_order.reverse()
    for layer in self.z_order:
      self.outFile.write(layer_list[layer])




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
    class_def += "         SIZE " + symbolNode.getElementsByTagName('pointsize')[0].childNodes[0].nodeValue.encode('utf-8')  + " \n"

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
        class_def += "        SIZE " + cls.getElementsByTagName('pointsize')[0].childNodes[0].nodeValue.encode('utf-8')  + " \n"

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
        class_def += "        SIZE " \
            + cls.getElementsByTagName('pointsize')[0].childNodes[0].nodeValue.encode('utf-8') \
            + " \n"

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
        symbolName = qgisSymbols[symbolNode.getElementsByTagName('pointsymbol')[0].childNodes[0].nodeValue.encode('utf-8')]
      except:
        symbolName = "CIRCLE"
      # make sure it's single quoted
      symbol = "'" + symbolName + "'"

    if symbolName == 'CIRCLE':
      self.symbolQueue['CIRCLE'] = """
      #Circle symbol
      SYMBOL
        NAME 'CIRCLE'
        TYPE ellipse
        FILLED true
        POINTS
          1 1
        END
      END """

    if symbolName == 'TRIANGLE':
      self.symbolQueue['TRIANGLE'] = """
      SYMBOL
        NAME "TRIANGLE"
        TYPE vector
        FILLED true
        POINTS
          0 1
         .5 0
          1 1
          0 1
        END
      END """

    return symbol

  # Label block creation
  # TODO field-based parameters, alignment, truetype fonts, sizes
  def msLabel(self, layerNode):
    # currently a very basic bitmap font
    labelNode = layerNode.getElementsByTagName('labelattributes')[0]
    labelField = labelNode.getElementsByTagName('label')[0].getAttribute('field').encode('utf-8')
    if labelField != '' and labelField is not None:
      labelBlock  = "     LABEL \n"
     
      labelBlock += "      SIZE medium\n"
      labelBlock += "      COLOR 0 0 0 \n"
 
      # Include label angle if specified
      # Note that angles only work for truetype fonts which aren't supported yet
      angle = labelNode.getElementsByTagName('angle')[0].getAttribute('value').encode('utf-8')
      labelBlock += "      ANGLE " + angle + "\n"
     
      # Include label buffer if specified
      # Note that the buffer has different meaning in qgis vs mapserver
      # mapserver just adds blank space around the label while
      # qgis uses a fill color around the label
      # Note that buffer only works for truetype fonts which aren't supported yet
      buffer = labelNode.getElementsByTagName('buffersize')[0].getAttribute('value').encode('utf-8')
      labelBlock += "      BUFFER " + buffer + "\n"

      labelBlock += "     END \n"
      return labelBlock
    else:
      return ''

