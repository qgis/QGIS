# -*- coding: utf-8 -*-

"""
***************************************************************************
    self.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Alexander Bruy
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
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Alexander Bruy'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsProcessingException,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterCrs,
                       QgsProcessingOutputRasterLayer,
                       QgsProcessingContext)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools.system import isWindows

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class AssignProjection(GdalAlgorithm):
    INPUT = 'INPUT'
    CRS = 'CRS'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT,
                                                            self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterCrs(self.CRS,
                                                    self.tr('Desired CRS')))

        self.addOutput(QgsProcessingOutputRasterLayer(self.OUTPUT,
                                                      self.tr('Layer with projection')))

    def name(self):
        return 'assignprojection'

    def displayName(self):
        return self.tr('Assign projection')

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'projection-add.png'))

    def tags(self):
        tags = self.tr('assign,set,transform,reproject,crs,srs').split(',')
        tags.extend(super().tags())
        return tags

    def group(self):
        return self.tr('Raster projections')

    def groupId(self):
        return 'rasterprojections'

    def commandName(self):
        return 'gdal_edit'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT))

        fileName = inLayer.source()

        crs = self.parameterAsCrs(parameters, self.CRS, context)

        arguments = [
            '-a_srs',
            GdalUtils.gdal_crs_string(crs),

            fileName
        ]

        self.setOutputValue(self.OUTPUT, fileName)

        return [self.commandName() + ('.bat' if isWindows() else '.py'), GdalUtils.escapeAndJoin(arguments)]

    def postProcessAlgorithm(self, context, feedback):
        # get output value
        fileName = self.output_values.get(self.OUTPUT)
        if not fileName:
            return {}

        # search in context project's layers
        if context.project():

            for l in context.project().mapLayers().values():

                # check the source
                if l.source() != fileName:
                    continue

                # reload provider's data
                l.dataProvider().reloadData()
                l.setCrs(l.dataProvider().crs())
                l.triggerRepaint()

        # search in context temporary layer store
        for l in context.temporaryLayerStore().mapLayers().values():

            # check the source
            if l.source() != fileName:
                continue

            # reload provider's data
            l.dataProvider().reloadData()
            l.setCrs(l.dataProvider().crs())
            context.temporaryLayerStore().addMapLayer(l)

        return {}
