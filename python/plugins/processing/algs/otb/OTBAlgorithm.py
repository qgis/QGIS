# -*- coding: utf-8 -*-

"""
***************************************************************************
    OTBAlgorithm.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
                           (C) 2013 by CS Systemes d'information (CS SI)
    Email                : volayaf at gmail dot com
                           otb at c-s dot fr (CS SI)
    Contributors         : Victor Olaya
                           Julien Malik (CS SI)  - Changing the way to load algorithms first version
                           Oscar Picas (CS SI)   - Changing the way to load algorithms
                           Alexia Mondot (CS SI) - Add hdf5 support
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
import re
from PyQt4.QtGui import QIcon
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterMultipleInput
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterSelection
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.ProcessingLog import ProcessingLog
from processing.core.parameters import getParameterFromString
from processing.core.outputs import getOutputFromString
from OTBUtils import OTBUtils
from processing.core.parameters import ParameterExtent
from processing.tools.system import getTempFilename
import xml.etree.ElementTree as ET
import traceback

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))


class OTBAlgorithm(GeoAlgorithm):

    REGION_OF_INTEREST = "ROI"

    def __init__(self, descriptionfile):
        GeoAlgorithm.__init__(self)
        self.roiFile = None
        self.descriptionFile = descriptionfile
        self.defineCharacteristicsFromFile()
        self.numExportedLayers = 0
        self.hasROI = None


    def __str__(self):
        return( "Algo : " + self.name + " from app : " + self.cliName + " in : " + self.group )

    def getCopy(self):
        newone = OTBAlgorithm(self.descriptionFile)
        newone.provider = self.provider
        return newone

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'otb.png'))

    def help(self):
        folder = os.path.join( OTBUtils.otbDescriptionPath(), 'doc' )
        helpfile = os.path.join( str(folder), self.appkey + ".html")
        if os.path.exists(helpfile):
            return False, helpfile
        else:
            raise (False, None)


    def adapt_list_to_string(self, c_list):
        a_list = c_list[1:]
        if a_list[0] in ["ParameterVector", "ParameterMultipleInput"]:
            if c_list[0] == "ParameterType_InputImageList":
                a_list[3] = 3
            elif c_list[0] == "ParameterType_InputFilenameList":
                a_list[3] = 4
            else:
                a_list[3] = -1

        a_list[1] = "-%s" % a_list[1]

        def mystr(par):
            if isinstance(par, list):
                return ";".join(par)
            return str(par)

        b_list = map(mystr, a_list)
        res = "|".join(b_list)
        return res

    def get_list_from_node(self, myet):
        all_params = []
        for parameter in myet.iter('parameter'):
            rebuild = []
            par_type = parameter.find('parameter_type').text
            key = parameter.find('key').text
            name = parameter.find('name').text
            source_par_type = parameter.find('parameter_type').attrib['source_parameter_type']
            rebuild.append(source_par_type)
            rebuild.append(par_type)
            rebuild.append(key)
            rebuild.append(name)
            for each in parameter[4:]:
                if each.tag not in ["hidden"]:
                    if len(list(each)) == 0:
                        rebuild.append(each.text)
                    else:
                        rebuild.append([item.text for item in each.iter('choice')])
            all_params.append(rebuild)
        return all_params


    def defineCharacteristicsFromFile(self):
        content = open(self.descriptionFile).read()
        dom_model = ET.fromstring(content)

        self.appkey = dom_model.find('key').text
        self.cliName = dom_model.find('exec').text
        self.name = dom_model.find('longname').text
        self.group = dom_model.find('group').text

        rebu = None
        the_result = None

        try:
            rebu = self.get_list_from_node(dom_model)
            the_result = map(self.adapt_list_to_string,rebu)
        except Exception, e:
            ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                self.tr('Could not open OTB algorithm: %s\n%s' % (self.descriptionFile, traceback.format_exc())))
            raise e

        for line in the_result:
            try:
                if line.startswith("Parameter") or line.startswith("*Parameter"):
                    if line.startswith("*Parameter"):
                        param = getParameterFromString(line[1:])
                        param.isAdvanced = True
                    else:
                        param = getParameterFromString(line)
                    # Hack for initializing the elevation parameters from Processing configuration
                    if param.name == "-elev.dem.path" or param.name == "-elev.dem" or "elev.dem" in param.name:
                        param.default = OTBUtils.otbSRTMPath()
                    elif param.name == "-elev.dem.geoid" or param.name == "-elev.geoid" or "elev.geoid" in param.name:
                        param.default = OTBUtils.otbGeoidPath()
                    self.addParameter(param)
                elif line.startswith("Extent"):
                    self.addParameter(ParameterExtent(self.REGION_OF_INTEREST, "Region of interest", "0,1,0,1"))
                    self.hasROI = True
                else:
                    self.addOutput(getOutputFromString(line))
            except Exception,e:
                ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                    self.tr('Could not open OTB algorithm: %s\n%s' % (self.descriptionFile, line)))
                raise e

    def processAlgorithm(self, progress):
        currentOs = os.name

        path = OTBUtils.otbPath()
        libpath = OTBUtils.otbLibPath()
        if path == "" or libpath == "":
            raise GeoAlgorithmExecutionException(
                self.tr('OTB folder is not configured. Please configure it '
                        'before running OTB algorithms.'))

        commands = []
        commands.append(path + os.sep + self.cliName)

        self.roiVectors = {}
        self.roiRasters = {}
        for param in self.parameters:
            # get the given input(s)
            if param.name in ["-il", "-in"] :
                newparams = ""
                listeParameters = param.value.split(";")
                for inputParameter in listeParameters :
                    # if HDF5 file
                    if "HDF5" in inputParameter :
                        if currentOs == "posix" :
                            data = inputParameter[6:]
                        else :
                            data = inputParameter[5:]
                        dataset = data

                        #on windows, there isn't "
                        #if data[-1] == '"':
                        if currentOs == "posix" :
                            data = data[:data.index('"')]
                        else :
                            data = data[:data.index('://')]
                        #try :
                        if currentOs == "posix" :
                            dataset.index('"')
                            dataset = os.path.basename( data ) + dataset[dataset.index('"'):]
                        #except ValueError :
                        else :
                            #dataset = os.path.basename( data ) + '"' + dataset[dataset.index('://'):]
                            dataset = dataset[dataset.index('://'):]

                        #get index of the subdataset with gdal
                        if currentOs == "posix" :
                            commandgdal = "gdalinfo " + data + " | grep '" + dataset + "$'"
                        else :
                            commandgdal = "gdalinfo " + data + " | findstr \"" + dataset + "$\""
                        resultGDAL = os.popen( commandgdal ).readlines()
                        indexSubdataset = -1
                        if resultGDAL :
                            indexSubdatasetString = re.search("SUBDATASET_(\d+)_", resultGDAL[0])
                            if indexSubdatasetString :
                                #match between ()
                                indexSubdataset = indexSubdatasetString.group(1)
                            else :
                                indexSubdataset = -1
                        else :
                            #print "Error : no match of ", dataset, "$ in gdalinfo " + data
                            indexSubdataset = -1


                        if not indexSubdataset == -1 :
                            indexSubdataset = int(indexSubdataset) -1
                            newParam = "\'" + data + "?&sdataidx=" + str(indexSubdataset) + "\'"

                        else :
                            newParam = inputParameter

                        newparams += newParam
                    # no hdf5
                    else :
                        newparams += inputParameter
                    newparams += ";"
                if newparams[-1] == ";":
                    newparams = newparams[:-1]
                param.value = newparams

            if param.value is None or param.value == "":
                continue
            if isinstance(param, ParameterVector):
                commands.append(param.name)
                if self.hasROI:
                    roiFile = getTempFilename('shp')
                    commands.append(roiFile)
                    self.roiVectors[param.value] = roiFile
                else:
                    commands.append("\"" + param.value+ "\"")
            elif isinstance(param, ParameterRaster):
                commands.append(param.name)
                if self.hasROI:
                    roiFile = getTempFilename('tif')
                    commands.append(roiFile)
                    self.roiRasters[param.value] = roiFile
                else:
                    commands.append("\"" + param.value+ "\"")
            elif isinstance(param, ParameterMultipleInput):
                commands.append(param.name)
                files = str(param.value).split(";")
                paramvalue = " ".join(["\"" + f + " \"" for f in files])
                commands.append(paramvalue)
            elif isinstance(param, ParameterSelection):
                commands.append(param.name)
                idx = int(param.value)
                commands.append(str(param.options[idx]))
            elif isinstance(param, ParameterBoolean):
                if param.value:
                    commands.append(param.name)
                    commands.append(str(param.value).lower())
            elif isinstance(param, ParameterExtent):
                self.roiValues = param.value.split(",")
            else:
                commands.append(param.name)
                commands.append(str(param.value))

        for out in self.outputs:
            commands.append(out.name)
            commands.append('"' + out.value + '"')
        for roiInput, roiFile in self.roiRasters.items():
            startX, startY = float(self.roiValues[0]), float(self.roiValues[1])
            sizeX = float(self.roiValues[2]) - startX
            sizeY = float(self.roiValues[3]) - startY
            helperCommands = [
                "otbcli_ExtractROI",
                "-in",       roiInput,
                "-out",      roiFile,
                "-startx",   str(startX),
                "-starty",   str(startY),
                "-sizex",    str(sizeX),
                "-sizey",    str(sizeY)
            ]
            ProcessingLog.addToLog(ProcessingLog.LOG_INFO, helperCommands)
            progress.setCommand(helperCommands)
            OTBUtils.executeOtb(helperCommands, progress)

        if self.roiRasters:
            supportRaster = self.roiRasters.itervalues().next()
            for roiInput, roiFile in self.roiVectors.items():
                helperCommands = [
                    "otbcli_VectorDataExtractROIApplication",
                    "-vd.in",           roiInput,
                    "-io.in",           supportRaster,
                    "-io.out",          roiFile,
                    "-elev.dem.path",   OTBUtils.otbSRTMPath()]
                ProcessingLog.addToLog(ProcessingLog.LOG_INFO, helperCommands)
                progress.setCommand(helperCommands)
                OTBUtils.executeOtb(helperCommands, progress)

        loglines = []
        loglines.append(self.tr('OTB execution command'))
        for line in commands:
            loglines.append(line)
            progress.setCommand(line)

        ProcessingLog.addToLog(ProcessingLog.LOG_INFO, loglines)
        import processing.algs.otb.OTBSpecific_XMLLoading
        module = processing.algs.otb.OTBSpecific_XMLLoading

        found = False
        if 'adapt%s' % self.appkey in dir(module):
            found = True
            commands = getattr(module, 'adapt%s' % self.appkey)(commands)
        else:
            the_key = 'adapt%s' % self.appkey
            if '-' in the_key:
                base_key = the_key.split("-")[0]
                if base_key in dir(module):
                    found = True
                    commands = getattr(module, base_key)(commands)

        if not found:
            ProcessingLog.addToLog(ProcessingLog.LOG_INFO,
                self.tr("Adapter for %s not found" % the_key))

        #frames = inspect.getouterframes(inspect.currentframe())[1:]
        #for a_frame in frames:
        #    frame,filename,line_number,function_name,lines,index = a_frame
        #    ProcessingLog.addToLog(ProcessingLog.LOG_INFO, "%s %s %s %s %s %s" % (frame,filename,line_number,function_name,lines,index))

        OTBUtils.executeOtb(commands, progress)
