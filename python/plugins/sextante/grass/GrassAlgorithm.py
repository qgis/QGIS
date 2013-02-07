# -*- coding: utf-8 -*-

"""
***************************************************************************
    GrassAlgorithm.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import uuid
from sextante.outputs.OutputHTML import OutputHTML
import importlib
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.parameters.ParameterTable import ParameterTable
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.outputs.OutputRaster import OutputRaster
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.outputs.OutputVector import OutputVector
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.SextanteLog import SextanteLog
from sextante.parameters.ParameterFactory import ParameterFactory
from sextante.outputs.OutputFactory import OutputFactory
from sextante.core.SextanteConfig import SextanteConfig
from sextante.core.QGisLayers import QGisLayers
from sextante.grass.GrassUtils import GrassUtils
import time
from sextante.core.SextanteUtils import SextanteUtils
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.core.LayerExporter import LayerExporter
from sextante.core.WrongHelpFileException import WrongHelpFileException
from sextante.outputs.OutputFile import OutputFile
from sextante.parameters.ParameterExtent import ParameterExtent
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterString import ParameterString

class GrassAlgorithm(GeoAlgorithm):

    GRASS_MIN_AREA_PARAMETER = "GRASS_MIN_AREA_PARAMETER"
    GRASS_SNAP_TOLERANCE_PARAMETER = "GRASS_SNAP_TOLERANCE_PARAMETER"
    GRASS_REGION_EXTENT_PARAMETER = "GRASS_REGION_PARAMETER"
    GRASS_REGION_CELLSIZE_PARAMETER = "GRASS_REGION_CELLSIZE_PARAMETER"

    def __init__(self, descriptionfile):
        GeoAlgorithm.__init__(self)
        self.descriptionFile = descriptionfile
        self.defineCharacteristicsFromFile()
        self.numExportedLayers = 0
        #GRASS console output, needed to do postprocessing in case GRASS dumps results to the console
        self.consoleOutput = []

    def getCopy(self):
        newone = GrassAlgorithm(self.descriptionFile)
        newone.provider = self.provider
        return newone

    def getIcon(self):
        return  QIcon(os.path.dirname(__file__) + "/../images/grass.png")

    def helpFile(self):
        folder = GrassUtils.grassHelpPath()
        helpfile = str(folder) + os.sep + self.grassName + ".html"
        if os.path.exists(helpfile):
            return helpfile
        else:
            raise WrongHelpFileException("Grass help folder is not correctly configured.\nPlease configure it")
        #return None

    def getParameterDescriptions(self):
        descs = {}
        try:
            helpfile = self.helpFile()
        except WrongHelpFileException:
            return descs
        if helpfile:
            try:
                infile = open(helpfile)
                lines = infile.readlines()
                for i in range(len(lines)):
                    if lines[i].startswith("<DT><b>"):
                        for param in self. parameters:
                            searchLine = "<b>" + param.name + "</b>"
                            if searchLine in lines[i]:
                                i+=1
                                descs[param.name] = lines[i][4:-6]
                                break

                infile.close()
            except Exception:
                pass
        return descs

    def defineCharacteristicsFromFile(self):
        lines = open(self.descriptionFile)
        line = lines.readline().strip("\n").strip()
        self.grassName = line
        line = lines.readline().strip("\n").strip()
        self.name = line
        line = lines.readline().strip("\n").strip()
        self.group = line
        hasRasterOutput = False
        hasVectorOutput = False
        while line != "":
            try:
                line = line.strip("\n").strip()
                if line.startswith("Parameter"):
                    parameter = ParameterFactory.getFromString(line);
                    self.addParameter(parameter)
                    if isinstance(parameter, ParameterVector):
                       hasVectorOutput = True
                    if isinstance(parameter, ParameterMultipleInput) and parameter.datatype < 3:
                       hasVectorOutput = True
                elif line.startswith("*Parameter"):
                    param = ParameterFactory.getFromString(line[1:])
                    param.isAdvanced = True
                    self.addParameter(param)
                else:
                    output = OutputFactory.getFromString(line)
                    self.addOutput(output);
                    if isinstance(output, OutputRaster):
                        hasRasterOutput = True
                line = lines.readline().strip("\n").strip()
            except Exception,e:
                SextanteLog.addToLog(SextanteLog.LOG_ERROR, "Could not open GRASS algorithm: " + self.descriptionFile + "\n" + line)
                raise e
        lines.close()

        self.addParameter(ParameterExtent(self.GRASS_REGION_EXTENT_PARAMETER, "GRASS region extent"))
        if hasRasterOutput:
            self.addParameter(ParameterNumber(self.GRASS_REGION_CELLSIZE_PARAMETER, "GRASS region cellsize (leave 0 for default)", 0, None, 0.0))
        if hasVectorOutput:
            param = ParameterNumber(self.GRASS_SNAP_TOLERANCE_PARAMETER, "v.in.ogr snap tolerance (-1 = no snap)", -1, None, -1.0)
            param.isAdvanced = True
            self.addParameter(param)
            param = ParameterNumber(self.GRASS_MIN_AREA_PARAMETER, "v.in.ogr min area", 0, None, 0.0001)
            param.isAdvanced = True
            self.addParameter(param)


    def getDefaultCellsize(self):
        cellsize = 0
        for param in self.parameters:
            if param.value:
                if isinstance(param, ParameterRaster):
                    if isinstance(param.value, QgsRasterLayer):
                        layer = param.value
                    else:
                        layer = QGisLayers.getObjectFromUri(param.value)
                    cellsize = max(cellsize, (layer.extent().xMaximum() - layer.extent().xMinimum())/layer.width())

                elif isinstance(param, ParameterMultipleInput):
                    layers = param.value.split(";")
                    for layername in layers:
                        layer = QGisLayers.getObjectFromUri(layername)
                        if isinstance(layer, QgsRasterLayer):
                            cellsize = max(cellsize, (layer.extent().xMaximum() - layer.extent().xMinimum())/layer.width())

        if cellsize == 0:
            cellsize = 1
        return cellsize


    def processAlgorithm(self, progress):
        if SextanteUtils.isWindows():
            path = GrassUtils.grassPath()
            if path == "":
                raise GeoAlgorithmExecutionException("GRASS folder is not configured.\nPlease configure it before running GRASS algorithms.")

        commands = []
        self.exportedLayers = {}
        outputCommands = []

        # if GRASS session has been created outside of this algorithm then get the list of layers loaded in GRASS
        # otherwise start a new session
        existingSession = GrassUtils.sessionRunning
        if existingSession:
            self.exportedLayers = GrassUtils.getSessionLayers()
        else:
            GrassUtils.startGrassSession()


        #1: Export layer to grass mapset
        for param in self.parameters:
            if isinstance(param, ParameterRaster):
                if param.value == None:
                    continue
                value = param.value
                # check if the layer hasn't already been exported in, for example, previous GRASS calls in this session
                if value in self.exportedLayers.keys():
                    continue
                else:
                    self.setSessionProjectionFromLayer(value, commands)
                    commands.append(self.exportRasterLayer(value))
            if isinstance(param, ParameterVector):
                if param.value == None:
                    continue
                value = param.value
                if value in self.exportedLayers.keys():
                    continue
                else:
                    self.setSessionProjectionFromLayer(value, commands)
                    commands.append(self.exportVectorLayer(value))
            if isinstance(param, ParameterTable):
                pass
            if isinstance(param, ParameterMultipleInput):
                if param.value == None:
                    continue
                layers = param.value.split(";")
                if layers == None or len(layers) == 0:
                    continue
                if param.datatype == ParameterMultipleInput.TYPE_RASTER:
                    for layer in layers:
                        if layer in self.exportedLayers.keys():
                            continue
                        else:
                            self.setSessionProjectionFromLayer(layer, commands)
                            commands.append(self.exportRasterLayer(layer))
                elif param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                    for layer in layers:
                        if layer in self.exportedLayers.keys():
                            continue
                        else:
                            self.setSessionProjectionFromLayer(layer, commands)
                            commands.append(self.exportVectorLayer(layer))

        self.setSessionProjectionFromProject(commands)

        region = str(self.getParameterValue(self.GRASS_REGION_EXTENT_PARAMETER))
        regionCoords = region.split(",")
        command = "g.region"
        command += " n=" + str(regionCoords[3])
        command +=" s=" + str(regionCoords[2])
        command +=" e=" + str(regionCoords[1])
        command +=" w=" + str(regionCoords[0])
        cellsize = self.getParameterValue(self.GRASS_REGION_CELLSIZE_PARAMETER)
        if cellsize:
            command +=" res=" + str(cellsize);
        else:
            command +=" res=" + str(self.getDefaultCellsize())

        commands.append(command)

        #2: set parameters and outputs
        command = self.grassName
        for param in self.parameters:
            if param.value == None or param.value == "":
                continue
            if (param.name == self.GRASS_REGION_CELLSIZE_PARAMETER or param.name == self.GRASS_REGION_EXTENT_PARAMETER
                    or param.name == self.GRASS_MIN_AREA_PARAMETER or param.name == self.GRASS_SNAP_TOLERANCE_PARAMETER):
                continue
            if isinstance(param, (ParameterRaster, ParameterVector)):
                value = param.value
                if value in self.exportedLayers.keys():
                    command+=(" " + param.name + "=" + self.exportedLayers[value])
                else:
                    command+=(" " + param.name + "=" + value)
            elif isinstance(param, ParameterMultipleInput):
                s = param.value
                for layer in self.exportedLayers.keys():
                    s = s.replace(layer, self.exportedLayers[layer])
                s = s.replace(";",",")
                command+=(" " + param.name + "=" + s);
            elif isinstance(param, ParameterBoolean):
                if param.value:
                    command += (" " + param.name)
            elif isinstance(param, ParameterSelection):
                idx = int(param.value)
                command+=(" " + param.name + "=" + str(param.options[idx]));
            elif isinstance(param, ParameterString):
                command+=(" " + param.name + "=\"" + str(param.value) + "\"");
            else:
                command+=(" " + param.name + "=" + str(param.value));

        uniqueSufix = str(uuid.uuid4()).replace("-","");
        for out in self.outputs:
            if isinstance(out, OutputFile):
                command+=(" " + out.name + "=\"" + out.value + "\"");
            elif not isinstance(out, OutputHTML): #html files are not generated by grass, only by sextante to decorate grass output
                #an output name to make sure it is unique if the session uses this algorithm several times
                uniqueOutputName = out.name + uniqueSufix
                command += (" " + out.name + "=" + uniqueOutputName)
                # add output file to exported layers, to indicate that they are present in GRASS
                self.exportedLayers[out.value]= uniqueOutputName


        command += " --overwrite"
        commands.append(command)

        #3:Export resulting layers to a format that qgis can read
        for out in self.outputs:
            if isinstance(out, OutputRaster):
                filename = out.value
                #Raster layer output: adjust region to layer before exporting
                commands.append("g.region rast=" + out.name + uniqueSufix)
                outputCommands.append("g.region rast=" + out.name + uniqueSufix)
                command = "r.out.gdal -c createopt=\"TFW=YES,COMPRESS=LZW\""
                command += " input="
                command += out.name + uniqueSufix
                command += " output=\"" + filename + "\""
                commands.append(command)
                outputCommands.append(command)

            if isinstance(out, OutputVector):
                filename = out.value
                command = "v.out.ogr -ce input=" + out.name + uniqueSufix
                command += " dsn=\"" + os.path.dirname(out.value) + "\""
                command += " format=ESRI_Shapefile"
                command += " olayer=" + os.path.basename(out.value)[:-4]
                command += " type=auto"
                commands.append(command)
                outputCommands.append(command)

        #4 Run GRASS
        loglines = []
        loglines.append("GRASS execution commands")
        for line in commands:
            progress.setCommand(line)
            loglines.append(line)
        if SextanteConfig.getSetting(GrassUtils.GRASS_LOG_COMMANDS):
            SextanteLog.addToLog(SextanteLog.LOG_INFO, loglines)
        self.consoleOutput = GrassUtils.executeGrass(commands, progress, outputCommands);
        self.postProcessResults();
        # if the session has been created outside of this algorithm, add the new GRASS layers to it
        # otherwise finish the session
        if existingSession:
            GrassUtils.addSessionLayers(self.exportedLayers)
        else:
            GrassUtils.endGrassSession()

    def postProcessResults(self):
        name = self.commandLineName().replace('.','_')[len('grass:'):]
        try:
            module = importlib.import_module('sextante.grass.ext.' + name)
        except ImportError:
            return
        if hasattr(module, 'postProcessResults'):
            func = getattr(module,'postProcessResults')
            func(self)

    def exportVectorLayer(self, orgFilename):
        #only export to an intermediate shp if the layer is not file-based.
        #We assume that almost all file formats will be supported by ogr
        #We also export if there is a selection
        if not os.path.exists(orgFilename):
            layer = QGisLayers.getObjectFromUri(orgFilename, False)
            if layer:
                filename = LayerExporter.exportVectorLayer(layer)
        else:
            layer = QGisLayers.getObjectFromUri(orgFilename, False)
            if layer:
                useSelection = SextanteConfig.getSetting(SextanteConfig.USE_SELECTED)
                if useSelection and layer.selectedFeatureCount() != 0:
                    filename = LayerExporter.exportVectorLayer(layer)
                else:
                    filename = orgFilename
            else:
                filename = orgFilename
        destFilename = self.getTempFilename()
        self.exportedLayers[orgFilename]= destFilename
        command = "v.in.ogr"
        min_area = self.getParameterValue(self.GRASS_MIN_AREA_PARAMETER);
        command += " min_area=" + str(min_area)
        snap = self.getParameterValue(self.GRASS_SNAP_TOLERANCE_PARAMETER);
        command += " snap=" + str(snap)
        command += " dsn=\"" + os.path.dirname(filename) + "\""
        command += " layer=" + os.path.basename(filename)[:-4]
        command += " output=" + destFilename;
        command += " --overwrite -o"
        return command

    def setSessionProjectionFromProject(self, commands):
        if not GrassUtils.projectionSet:
            from sextante.core.Sextante import Sextante
            qgis = Sextante.getInterface()
            proj4 = qgis.mapCanvas().mapRenderer().destinationCrs().toProj4()
            command = "g.proj"
            command +=" -c"
            command +=" proj4=\""+proj4+"\""
            commands.append(command)
            GrassUtils.projectionSet = True

    def setSessionProjectionFromLayer(self, layer, commands):
        if not GrassUtils.projectionSet:
            qGisLayer = QGisLayers.getObjectFromUri(layer)
            if qGisLayer:
                proj4 = str(qGisLayer.crs().toProj4())
                command = "g.proj"
                command +=" -c"
                command +=" proj4=\""+proj4+"\""
                commands.append(command)
                GrassUtils.projectionSet = True


    def exportRasterLayer(self, layer):
        destFilename = self.getTempFilename()
        self.exportedLayers[layer]= destFilename
        command = "r.external"
        command +=" input=\"" + layer + "\""
        command +=" band=1"
        command +=" output=" + destFilename;
        command +=" --overwrite -o"
        return command


    def getTempFilename(self):
        filename =  "tmp" + str(time.time()).replace(".","") + str(SextanteUtils.getNumExportedLayers())
        return filename

    def commandLineName(self):
        return "grass:" + self.name[:self.name.find(" ")]

    def checkParameterValuesBeforeExecuting(self):
        name = self.commandLineName().replace('.','_')[len('grass:'):]
        try:
            module = importlib.import_module('sextante.grass.ext.' + name)
        except ImportError:
            return
        if hasattr(module, 'checkParameterValuesBeforeExecuting'):
            func = getattr(module,'checkParameterValuesBeforeExecuting')
            return func(self)

