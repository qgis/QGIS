# -*- coding: utf-8 -*-

"""
***************************************************************************
    createnetwork.py
    ---------------------
    Date                 : February 2017
    Copyright            : (C) 2017 by Alexander Bruy
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
__date__ = 'February 2017'
__copyright__ = '(C) 2017, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from osgeo import gdal, ogr, gnm

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import (ParameterMultipleInput,
                                        ParameterNumber,
                                        ParameterString,
                                        ParameterCrs)
from processing.core.outputs import OutputDirectory
from processing.tools import dataobjects


class CreateNetwork(GeoAlgorithm):

    INPUT_LAYERS = 'INPUT_LAYERS'
    TOLERANCE = 'TOLERANCE'
    NETWORK_FORMAT = 'NETWORK_FORMAT'
    NETWORK_CRS = 'NETWORK_CRS'
    NETWORK_NAME = 'NETWORK_NAME'
    NETWORK_DESCRIPTION = 'NETWORK_DESCRIPTION'
    RULES = 'RULES'
    NETWORK = 'NETWORK'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Create network')
        self.group, self.i18n_group = self.trAlgorithm('Network analysis')

        self.addParameter(ParameterMultipleInput(
            self.INPUT_LAYERS,
            self.tr('Layer to add to the network'),
            dataobjects.TYPE_VECTOR_ANY))
        self.addParameter(ParameterNumber(
            self.TOLERANCE,
            self.tr('Topology tolerance'),
            0.0, 99999999.999999, 0.0))
        self.addParameter(ParameterString(
            self.NETWORK_FORMAT,
            self.tr('Network format'),
            'ESRI Shapefile'))
        self.addParameter(ParameterCrs(
            self.NETWORK_CRS,
            self.tr('Network CRS'),
            'EPSG:4326'))
        self.addParameter(ParameterString(
            self.NETWORK_NAME,
            self.tr('Network name')))
        self.addParameter(ParameterString(
            self.NETWORK_DESCRIPTION,
            self.tr('Network description'),
            optional=True))
        self.addParameter(ParameterString(
            self.RULES,
            self.tr('Network rules'),
            multiline=True,
            optional=True))

        self.addOutput(OutputDirectory(
            self.NETWORK,
            self.tr('Directory for storing network')))

    def processAlgorithm(self, feedback):
        layers = self.getParameterValue(self.INPUT_LAYERS).split(';')
        tolerance = self.getParameterValue(self.TOLERANCE)
        networkFormat = self.getParameterValue(self.NETWORK_FORMAT)
        networkCrs = self.getParameterValue(self.NETWORK_CRS)
        networkName = self.getParameterValue(self.NETWORK_NAME)
        networkDescription = self.getParameterValue(self.NETWORK_DESCRIPTION)
        rules = self.getParameterValue(self.RULES)

        outputPath = self.getOutputValue(self.NETWORK)

        if networkName == '':
            raise GeoAlgorithmExecutionException(
                self.tr('Network name can not be empty.'))

        # hardcoded for now, as only file-based networks implemented
        driver = gdal.GetDriverByName('GNMFile')
        if driver is None:
            raise GeoAlgorithmExecutionException(
                self.tr('Can not initialize GNM driver.'))

        # network metadata
        options = []
        options.append('net_srs={}'.format(networkCrs))
        options.append('net_name={}'.format(networkName))
        options.append('net_description={}'.format(networkDescription))

        # create empty network dataset
        ds = driver.Create(outputPath, 0, 0, 0, gdal.GDT_Unknown, options)
        network = gnm.CastToNetwork(ds)
        if network is None:
            raise GeoAlgorithmExecutionException(
                self.tr('Can not initialize network dataset.'))

        genericNetwork = gnm.CastToGenericNetwork(ds)
        if genericNetwork is None:
            raise GeoAlgorithmExecutionException(
                self.tr('Can not initialize generic network dataset.'))

        # network created, now it is time to add layers to it
        hasPointLayer = False
        hasLineLayer = False
        importedLayers = []
        for path in layers:
            layerDs = gdal.OpenEx(path, gdal.OF_VECTOR)
            if layerDs is None:
                raise GeoAlgorithmExecutionException(
                    self.tr('Can not open dataset {}.'.format(path)))

            # we assume that each dataset has only one layer in it
            layer = layerDs.GetLayerByIndex(0)
            if layer is None:
                raise GeoAlgorithmExecutionException(
                    self.tr('Can not fetch layer 0 from the dataset {}.'.format(path)))

            # import layer into network
            layerName = layer.GetName()
            lay = network.CopyLayer(layer, layerName)
            if lay is None:
                raise GeoAlgorithmExecutionException(
                    self.tr('Could not import layer 0 from the dataset {} into the network.'.format(path)))

            importedLayers.append(lay.GetName())

            geometryType = lay.GetGeomType()
            if geometryType == ogr.wkbPoint:
                hasPointLayer = True
            elif geometryType == ogr.wkbLineString:
                hasLineLayer = True

            layerDs = None

        # add rules
        if rules is not None:
            for r in rules.split('\n'):
                result = genericNetwork.CreateRule(r)
                if result != 0:
                    raise GeoAlgorithmExecutionException(
                        self.tr('Can not create rule "{}".'.format(r)))

        # warn user if some layers are missing
        if not hasPointLayer:
            feedback.pushInfo(
                self.tr('No point layers were imported. We will not be able to build network topology.'))
        elif not hasLineLayer:
            feedback.pushInfo(
                self.tr('No line layers were imported. We will not be able to build network topology.'))

        # create network topology
        result = genericNetwork.ConnectPointsByLines(
            importedLayers, tolerance, 1.0, 1.0, gnm.GNM_EDGE_DIR_BOTH)
        if result != 0:
            raise GeoAlgorithmExecutionException(
                self.tr('Can not build network topology.'))

        # close all datasets
        genericNetwork = None
        network = None
        ds = None
