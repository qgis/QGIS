"""
***************************************************************************
    rasterize_over_fixed_value.py
    ---------------------
    Date                 : September 2013
    Copyright            : (C) 2013 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'September 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterString,
                       QgsProcessingParameterBoolean,
                       QgsProcessingOutputRasterLayer)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class rasterize_over_fixed_value(GdalAlgorithm):
    INPUT = 'INPUT'
    INPUT_RASTER = 'INPUT_RASTER'
    ADD = 'ADD'
    EXTRA = 'EXTRA'
    BURN = 'BURN'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input vector layer')))
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT_RASTER,
                                                            self.tr('Input raster layer')))
        self.addParameter(QgsProcessingParameterNumber(self.BURN,
                                                       self.tr('A fixed value to burn'),
                                                       type=QgsProcessingParameterNumber.Type.Double,
                                                       defaultValue=0.0))

        params = [
            QgsProcessingParameterBoolean(self.ADD,
                                          self.tr('Add burn in values to existing raster values'),
                                          defaultValue=False),
            QgsProcessingParameterString(self.EXTRA,
                                         self.tr('Additional command-line parameters'),
                                         defaultValue=None,
                                         optional=True)
        ]
        for p in params:
            p.setFlags(p.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
            self.addParameter(p)

        self.addOutput(QgsProcessingOutputRasterLayer(self.OUTPUT,
                                                      self.tr('Rasterized')))

    def name(self):
        return 'rasterize_over_fixed_value'

    def displayName(self):
        return self.tr('Rasterize (overwrite with fixed value)')

    def group(self):
        return self.tr('Vector conversion')

    def groupId(self):
        return 'vectorconversion'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'rasterize.png'))

    def commandName(self):
        return 'gdal_rasterize'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        input_details = self.getOgrCompatibleSource(self.INPUT, parameters, context, feedback, executing)
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT_RASTER, context)
        if inLayer is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT_RASTER))
        input_raster_details = GdalUtils.gdal_connection_details_from_layer(
            inLayer)

        self.setOutputValue(self.OUTPUT, inLayer.source())

        arguments = [
            '-l',
            input_details.layer_name,
            '-burn',
            str(self.parameterAsDouble(parameters, self.BURN, context)),
        ]

        if self.parameterAsBool(parameters, self.ADD, context):
            arguments.append('-add')

        if input_details.open_options:
            if GdalUtils.version() < 3070000:
                raise QgsProcessingException(self.tr(
                    'Open options are not supported by gdal_rasterize version {} (requires GDAL version 3.7 or later)'.format(
                        GdalUtils.readableVersion())))

            arguments.extend(input_details.open_options_as_arguments())

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ''):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        arguments.append(input_details.connection_string)
        arguments.append(input_raster_details.connection_string)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]

    def postProcessAlgorithm(self, context, feedback):
        fileName = self.output_values.get(self.OUTPUT)
        if not fileName:
            return {}

        if context.project():
            for l in context.project().mapLayers().values():
                if l.source() != fileName:
                    continue

                l.dataProvider().reloadData()
                l.triggerRepaint()

        for l in context.temporaryLayerStore().mapLayers().values():
            if l.source() != fileName:
                continue

            l.dataProvider().reloadData()

        return {}
