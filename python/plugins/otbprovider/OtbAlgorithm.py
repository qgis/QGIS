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

import os

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtGui import QIcon

from qgis.core import (Qgis,
                       QgsMessageLog,
                       QgsMapLayer,
                       QgsApplication,
                       QgsProcessingException,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterDefinition,
                       QgsProcessingOutputLayerDefinition,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterString,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterFile,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingParameterVectorDestination,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterBand,
                       QgsProcessingParameterField,
                       QgsProviderRegistry)

from processing.core.parameters import getParameterFromString
from .OtbChoiceWidget import OtbParameterChoice
from .OtbUtils import OtbUtils


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

        self.pixelTypes = ['uint8', 'uint16', 'int16', 'uint32', 'int32',
                           'float', 'double', 'cint16', 'cint32', 'cfloat', 'cdouble']
        self._descriptionfile = descriptionfile
        self.defineCharacteristicsFromFile()

    def icon(self):
        return QgsApplication.getThemeIcon("/providerOtb.svg")

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

    # TODO: show version which is same as OtbAlgorithm rather than always latest.
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
                # self._name = self._name #+ " - " + self.doc
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
                        param = getParameterFromString(line, 'OtbAlgorithm')

                    # if parameter is None, then move to next line and continue
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

                    # 'elev.dem.path', 'elev.dem', 'elev.dem.geoid', 'elev.geoid' are special!
                    # Even though it is not typical for OTB to fix on parameter keys,
                    # we current use below !hack! to set SRTM path and GEOID files
                    # Future releases of OTB must follow this rule keep
                    # compatibility or update this checklist.
                    if name in ["elev.dem.path", "elev.dem"]:
                        param.setDefaultValue(OtbUtils.srtmFolder())
                    if name in ["elev.dem.geoid", "elev.geoid"]:
                        param.setDefaultValue(OtbUtils.geoidFile())

                    # outputpixeltype is a special parameter associated with raster output
                    # reset list of options to 'self.pixelTypes'.
                    if name == 'outputpixeltype':
                        param.setOptions(self.pixelTypes)
                        param.setDefaultValue(self.pixelTypes.index('float'))

                    self.addParameter(param)
                    # parameter is added now and we must move to next line
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
            # If parameterDefinition(k) return None, this is considered a invalid parameter.
            # just continue for loop
            if param is None:
                continue

            # Any other valid parameters have:
            # - empty or no metadata
            # - metadata without a 'group_key'
            # - metadata with 'group_key' and 'group_key' is not in list of parameters. see ParameterGroup in OTB
            # - metadata with 'group_key' and 'group_key' is a valid parameter and it's value == metadata()['group_value']
            if (
                'group_key' in param.metadata()
                and param.metadata()['group_key'] not in parameters
            ):
                valid_params[k] = v
            elif 'group_key' not in param.metadata() or parameters[param.metadata()['group_key']] == param.metadata()[
                    'group_value']:
                valid_params[k] = v

        return valid_params

    def processAlgorithm(self, parameters, context, feedback):
        app_launcher_path = OtbUtils.getExecutableInPath(OtbUtils.otbFolder(), 'otbApplicationLauncherCommandLine')
        command = '"{}" {} {}'.format(app_launcher_path, self.name(), OtbUtils.appFolder())
        outputPixelType = None
        for k, v in parameters.items():
            # if value is None or en empty string for a parameter then we don't want to pass that value to OTB (see https://gitlab.orfeo-toolbox.org/orfeotoolbox/otb/-/issues/2130)
            if v == '' or v is None:
                continue
            # for 'outputpixeltype' parameter we find the pixeltype string from self.pixelTypes
            if k == 'outputpixeltype':
                pixel_type = self.pixelTypes[int(parameters['outputpixeltype'])]
                outputPixelType = None if pixel_type == 'float' else pixel_type
                continue

            param = self.parameterDefinition(k)
            if param.isDestination():
                continue
            if isinstance(param, QgsProcessingParameterEnum) and param.name() == "outputpixeltype":
                value = self.parameterAsEnum(parameters, param.name(), context)
            elif isinstance(param, QgsProcessingParameterEnum):
                value = " ".join(
                    param.options()[i]
                    for i in self.parameterAsEnums(
                        parameters, param.name(), context
                    )
                    if i >= 0 and i < len(param.options())
                )

            elif isinstance(param, QgsProcessingParameterField):
                value = " ".join(self.parameterAsFields(parameters, param.name(), context))
            elif isinstance(param, QgsProcessingParameterBoolean):
                value = self.parameterAsBoolean(parameters, param.name(), context)
            elif isinstance(param, QgsProcessingParameterCrs):
                crsValue = self.parameterAsCrs(parameters, param.name(), context)
                authid = crsValue.authid()
                if authid.startswith('EPSG:'):
                    value = authid.split('EPSG:')[1]
                else:
                    raise QgsProcessingException(
                        self.tr("Incorrect value for parameter '{}'. No EPSG code found in '{}'".format(
                            param.name(),
                            authid)))
            elif isinstance(param, QgsProcessingParameterFile):
                value = self.parameterAsFile(parameters, param.name(), context)
            elif isinstance(param, QgsProcessingParameterMultipleLayers):
                layers = self.parameterAsLayerList(parameters, param.name(), context)
                if layers is None or len(layers) == 0:
                    continue
                value = ' '.join(
                    '"{}"'.format(self.getLayerSource(param.name(), layer))
                    for layer in layers
                )

            elif isinstance(param, QgsProcessingParameterNumber):
                if param.dataType() == QgsProcessingParameterNumber.Integer:
                    value = self.parameterAsInt(parameters, param.name(), context)
                else:
                    value = self.parameterAsDouble(parameters, param.name(), context)
            elif isinstance(param, (QgsProcessingParameterRasterLayer, QgsProcessingParameterVectorLayer)):
                value = '"{}"'.format(self.getLayerSource(param.name(), self.parameterAsLayer(parameters, param.name(), context)))
            elif isinstance(param, QgsProcessingParameterString):
                value = '"{}"'.format(self.parameterAsString(parameters, param.name(), context))
            elif isinstance(param, QgsProcessingParameterBand):
                value = ' '.join(
                    '"Channel{}"'.format(index)
                    for index in self.parameterAsInts(
                        parameters, param.name(), context
                    )
                )

            else:
                # Use whatever is given
                value = '"{}"'.format(parameters[param.name()])

            # Check if value is set in above if elif ladder and update command string
            if value and value is not None:
                command += ' -{} {}'.format(k, value)

        output_files = {}

        for out in self.destinationParameterDefinitions():
            filePath = self.parameterAsOutputLayer(parameters, out.name(), context)
            if filePath:
                output_files[out.name()] = filePath
                if outputPixelType is not None:
                    command += ' -{} "{}" "{}"'.format(out.name(), filePath, outputPixelType)
                else:
                    command += ' -{} "{}"'.format(out.name(), filePath)

        OtbUtils.executeOtb(command, feedback)

        return {
            o.name(): output_files[o.name()]
            for o in self.outputDefinitions()
            if o.name() in output_files
        }

    def getLayerSource(self, name, layer):
        providerName = layer.dataProvider().name()

        # TODO: add other provider support in OTB, eg: memory
        if providerName == 'gdal':
            return layer.source()
        elif providerName == 'ogr':
            # when a file contains several layer we pass only the file path to OTB
            # TODO make OTB able to take a layer index in this case
            uriElements = QgsProviderRegistry.instance().decodeUri("ogr", layer.source())

            if 'path' not in uriElements:
                raise QgsProcessingException(
                    self.tr("Invalid layer source '{}'. Missing valid 'path' element".format(layer.source())))

            return uriElements['path']
        else:
            raise QgsProcessingException(
                self.tr("OTB currently support only gdal and ogr provider. Parameter '{}' uses '{}' provider".format(name, providerName)))
