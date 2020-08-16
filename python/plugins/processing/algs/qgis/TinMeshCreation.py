# -*- coding: utf-8 -*-

"""
***************************************************************************
    TinInterpolation.py
    ---------------------
    Date                 : August 2020
    Copyright            : (C) 2020 by Vincent Cloarec
    Email                : vcloarec at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Vincent Cloarec'
__date__ = 'August 2020'
__copyright__ = '(C) 2020, Vincent Cloarec'

import os

from qgis.utils import iface

from qgis.core import (QgsProcessingUtils,
                       QgsProcessingContext,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterFileDestination,
                       QgsProcessingException,
                       QgsProviderRegistry,
                       QgsMeshDriverMetadata,
                       QgsMeshLayer)
from qgis.analysis import (QgsMeshTriangulation,
                           QgsMeshZValueDatasetGroup)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.algs.qgis.ui.TinMeshWidgets import ParameterTinMeshData

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class TinMeshCreation(QgisAlgorithm):
    SOURCE_DATA = 'SOURCE_DATA'
    EXTENT = 'EXTENT'
    MESH_FORMAT = 'MESH_FORMAT'
    CRS = 'CRS_OUTPUT'
    OUTPUT_MESH = 'OUTPUT_MESH'

    def group(self):
        return self.tr('Mesh')

    def groupId(self):
        return 'mesh'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(ParameterTinMeshData(self.SOURCE_DATA, self.tr('Input layer(s)')))

        self.FORMATS = []
        self.providerMetaData = QgsProviderRegistry.instance().providerMetadata("mdal")
        meshDriverMetaList = self.providerMetaData.meshDriversMetadata()
        meshDriverAvailable = []
        for driver in meshDriverMetaList:
            if bool(driver.capabilities() & QgsMeshDriverMetadata.CanWriteMeshData):
                meshDriverAvailable.append(driver)
                self.FORMATS.append(driver.name())

        self.addParameter(QgsProcessingParameterEnum(self.MESH_FORMAT,
                                                     self.tr('Output format'),
                                                     options=self.FORMATS,
                                                     defaultValue=0))

        self.addParameter(QgsProcessingParameterCrs(self.CRS,
                                                    self.tr('Output coordinate system'),
                                                    optional=True))

        self.addParameter(QgsProcessingParameterFileDestination(self.OUTPUT_MESH,
                                                                self.tr('Output file'),
                                                                optional=False))

    def name(self):
        return 'tinmeshcreation'

    def displayName(self):
        return self.tr('TIN mesh creation')

    def processAlgorithm(self, parameters, context, feedback):
        sourceData = ParameterTinMeshData.parseValue(parameters[self.SOURCE_DATA])

        if sourceData is None:
            raise QgsProcessingException(
                self.tr('You need to specify at least one input layer.'))

        crs = self.parameterAsCrs(parameters, self.CRS, context)
        if not crs.isValid():
            crs = iface.mapCanvas().mapSettings().destinationCrs()

        meshTriangulation = QgsMeshTriangulation()

        for i, row in enumerate(sourceData.split('::|::')):
            v = row.split('::~::')
            layer = QgsProcessingUtils.mapLayerFromString(v[0], context)
            if not crs.isValid():
                crs = layer.sourceCrs()

            valueAttribute = int(v[1])

            if v[2] == '0':  # points
                meshTriangulation.addVertices(layer, valueAttribute, context.transformContext(), feedback)
            else:  # lines
                meshTriangulation.addBreakLines(layer, valueAttribute, context.transformContext(), feedback)

        fileName = self.parameterAsFile(parameters, self.OUTPUT_MESH, context)
        driverIndex = self.parameterAsEnum(parameters, self.MESH_FORMAT, context)
        mesh = meshTriangulation.triangulatedMesh()
        self.providerMetaData.createMeshData(mesh, fileName, self.FORMATS[driverIndex], crs)

        # SELAFIN format doesn't support saving Z value on mesh vertices, so create a specific dataset group
        if self.FORMATS[driverIndex] == "SELAFIN":
            self.addZValueDataset(fileName, mesh)

        context.addLayerToLoadOnCompletion(fileName, QgsProcessingContext.LayerDetails('TIN Mesh',
                                                                                       context.project(),
                                                                                       'TIN', QgsProcessingUtils.LayerHint.Mesh))

        return {self.OUTPUT_MESH: fileName}

    def addZValueDataset(self, fileName, mesh):

        tempLayer = QgsMeshLayer(fileName, "temp", "mdal")

        zValueDatasetGroup = QgsMeshZValueDatasetGroup(self.tr("Terrain Elevation"), mesh)
        tempLayer.addDatasets(zValueDatasetGroup)
        datasetGroupIndex = tempLayer.datasetGroupCount() - 1
        tempLayer.saveDataset(fileName, datasetGroupIndex, "SELAFIN")
