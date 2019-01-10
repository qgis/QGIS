# -*- coding: utf-8 -*-
"""
***************************************************************************
    OtbAlgorithm.py
    ---------------
    Date                 : June 2017
    Copyright            : (C) 2017 by CS Systemes d'Information (CS SI)
                         : (C) 2018 by Centre National d'Etudes et spatiales (CNES)
    Email                : rashad dot kanavath at c-s fr, otb at c-s dot fr (CS SI)

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Rashad Kanavath'
__date__ = 'June 2017'
__copyright__ = "(C) 2017,2018 by CS Systemes d'information (CS SI), Centre National d'Etudes et spatiales (CNES)"

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtGui import QIcon

from qgis.core import (Qgis,
                       QgsMessageLog,
                       QgsRasterLayer,
                       QgsMapLayer,
                       QgsApplication,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterDefinition,
                       QgsProcessingOutputLayerDefinition,
                       QgsProcessingParameterString,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterEnum)

from processing.core.parameters import getParameterFromString
from processing.algs.otb.OtbChoiceWidget import OtbParameterChoice
from processing.algs.otb import OtbUtils


class OtbAlgorithm(QgsProcessingAlgorithm):
    def __init__(self, group, name, descriptionfile, display_name='', groupId=''):
        super().__init__()
        self._name = name
        self._group = group
        self._display_name = display_name

        self._groupId = ''
        validChars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:'
        if not groupId:
            self._groupId = ''.join(c for c in self._group if c in validChars)

        self.pixelTypes = ['uint8', 'int', 'float', 'double']
        self._descriptionfile = descriptionfile
        self.defineCharacteristicsFromFile()

    def icon(self):
        return QgsApplication.getThemeIcon("/providerOtb.png")

    def createInstance(self):
        return self.__class__(self._group, self._name, self._descriptionfile)

    def tr(self, string):
        return QCoreApplication.translate("OtbAlgorithm", string)

    def name(self):
        return self._name

    def displayName(self):
        return self._display_name

    def group(self):
        return self._group

    def groupId(self):
        return self._groupId

    def descriptionfile(self):
        return self._descriptionfile

    def initAlgorithm(self, config=None):
        pass

    #TODO: show version which is same as OtbAlgorithm rather than always latest.
    def helpUrl(self):
        return "https://www.orfeo-toolbox.org/CookBook/Applications/app_" + self.name() + ".html"

    def defineCharacteristicsFromFile(self):
        line = None
        try:
            with open(self._descriptionfile) as lines:
                line = lines.readline().strip('\n').strip()
                self._name = line.split('|')[0]
                self.appkey = self._name
                line = lines.readline().strip('\n').strip()
                self.doc = line
                self.i18n_doc = QCoreApplication.translate("OtbAlgorithm", self.doc)
                #self._name = self._name #+ " - " + self.doc
                self._display_name = self.tr(self._name)
                self.i18n_name = QCoreApplication.translate("OtbAlgorithm", self._name)

                line = lines.readline().strip('\n').strip()
                self._group = line
                self.i18n_group = QCoreApplication.translate("OtbAlgorithm", self._group)
                line = lines.readline().strip('\n').strip()
                while line != '':
                    line = line.strip('\n').strip()
                    if line.startswith('#'):
                        line = lines.readline().strip('\n').strip()
                        continue
                    param = None
                    if 'OTBParameterChoice' in line:
                        tokens = line.split("|")
                        params = [t if str(t) != str(None) else None for t in tokens[1:]]
                        options = params[2].split(';')
                        param = OtbParameterChoice(params[0], params[1], options, params[3], params[4])
                    else:
                        param = getParameterFromString(line)

                    #if parameter is None, then move to next line and continue
                    if param is None:
                        line = lines.readline().strip('\n').strip()
                        continue

                    name = param.name()
                    if '.' in name and len(name.split('.')) > 2:
                        p = name.split('.')[:-2]
                        group_key = '.'.join(p)
                        group_value = name.split('.')[-2]
                        metadata = param.metadata()
                        metadata['group_key'] = group_key
                        metadata['group_value'] = group_value
                        param.setMetadata(metadata)

                    #'elev.dem.path', 'elev.dem', 'elev.dem.geoid', 'elev.geoid' are special!
                    #Even though it is not typical for OTB to fix on parameter keys,
                    #we current use below !hack! to set SRTM path and GEOID files
                    #Future releases of OTB must follow this rule keep
                    #compatibility or update this checklist.
                    if name in ["elev.dem.path", "elev.dem"]:
                        param.setDefaultValue(OtbUtils.srtmFolder())
                    if name in ["elev.dem.geoid", "elev.geoid"]:
                        param.setDefaultValue(OtbUtils.geoidFile())

                    self.addParameter(param)
                    #parameter is added now and we must move to next line
                    line = lines.readline().strip('\n').strip()

        except BaseException as e:
            import traceback
            errmsg = "Could not open OTB algorithm from file: \n" + self._descriptionfile + "\nline=" + line + "\nError:\n" + traceback.format_exc()
            QgsMessageLog.logMessage(self.tr(errmsg), self.tr('Processing'), Qgis.Critical)
            raise e

    def preprocessParameters(self, parameters):
        valid_params = {}
        for k, v in parameters.items():
            param = self.parameterDefinition(k)
            #If parameterDefinition(k) return None, this is considered a invalid parameter.
            #just continue for loop
            if param is None:
                continue

            #if name of parameter is 'pixtype',
            #it is considered valid if it has value other than float
            if k == 'outputpixeltype' and self.pixelTypes[int(v)] == 'float':
                continue
            # Any other valid parameters have:
            #- empty or no metadata
            #- metadata without a 'group_key'
            #- metadata with 'group_key' and 'group_key' is not in list of parameters. see ParameterGroup in OTB
            #- metadata with 'group_key' and 'group_key' is a valid parameter and it's value == metadata()['group_value']
            if 'group_key' in param.metadata() and not param.metadata()['group_key'] in parameters:
                valid_params[k] = v
            elif not 'group_key' in param.metadata() or parameters[param.metadata()['group_key']] == param.metadata()['group_value']:
                valid_params[k] = v

        return valid_params

    def otbParameterValue(self, v):
        if isinstance(v, QgsMapLayer):
            return v.source()
        elif isinstance(v, QgsProcessingOutputLayerDefinition):
            return v.sink.staticValue()
        else:
            return str(v)

    def outputParameterName(self):
        with open(self._descriptionfile) as df:
            first_line = df.readline().strip()
            tokens = first_line.split("|")
            #params = [t if str(t) != str(None) else None for t in tokens[1:]]
            if len(tokens) == 2:
                return tokens[1]
            else:
                return ''

    def processAlgorithm(self, parameters, context, feedback):
        output_key = self.outputParameterName()
        otb_cli_file = OtbUtils.cliPath()
        command = '"{}" {} {}'.format(otb_cli_file, self.name(), OtbUtils.appFolder())
        for k, v in parameters.items():
            if k == 'outputpixeltype' or not v:
                continue

            if 'epsg' in k and v.startswith('EPSG:'):
                v = v.split('EPSG:')[1]

            if isinstance(v, str) and '\n' in v:
                v = v.split('\n')
            if isinstance(v, list):
                value = ''
                for i in list(filter(None, v)):
                    value += '"{}" '.format(self.otbParameterValue(i))
            else:
                value = '"{}"'.format(self.otbParameterValue(v))

            if k == output_key and 'outputpixeltype' in parameters:
                output_pixel_type = self.pixelTypes[int(parameters['outputpixeltype'])]
                value = '"{}" "{}"'.format(value, output_pixel_type)

            command += ' -{} {}'.format(k, value)

        QgsMessageLog.logMessage(self.tr('cmd={}'.format(command)), self.tr('Processing'), Qgis.Info)
        if not os.path.exists(otb_cli_file) or not os.path.isfile(otb_cli_file):
            import errno
            raise FileNotFoundError(errno.ENOENT, os.strerror(errno.ENOENT), otb_cli_file)

        OtbUtils.executeOtb(command, feedback)

        result = {}
        for out in self.destinationParameterDefinitions():
            filePath = self.parameterAsOutputLayer(parameters, out.name(), context)
            result[out.name()] = filePath
        return result
