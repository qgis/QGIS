# -*- coding: utf-8 -*-

"""
***************************************************************************
    general.py
    ---------------------
    Date                 : April 2013
    Copyright            : (C) 2013 by Victor Olaya
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
from __future__ import print_function
from future import standard_library
standard_library.install_aliases()
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'April 2013'
__copyright__ = '(C) 2013, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
try:
    import configparser
except ImportError:
    import configparser as configparser

from qgis.core import (QgsApplication,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterVectorOutput,
                       QgsProcessingParameterRasterOutput,
                       QgsProcessingOutputLayerDefinition,
                       QgsProject)
from processing.core.Processing import Processing
from processing.core.parameters import ParameterSelection
from processing.gui.Postprocessing import handleAlgorithmResults


def algorithmOptions(id):
    """Prints all algorithm options with their values.
    """
    alg = QgsApplication.processingRegistry().algorithmById(id)
    if alg is not None:
        opts = ''
        for param in alg.parameterDefinitions():
            if isinstance(param, ParameterSelection):
                opts += '{} ({})\n'.format(param.name(), param.description())
                for option in enumerate(param.options):
                    opts += '\t{} - {}\n'.format(option[0], option[1])
        print(opts)
    else:
        print('Algorithm "{}" not found.'.format(id))


def algorithmHelp(id):
    """Prints algorithm parameters with their types. Also
    provides information about options if any.
    """
    alg = QgsApplication.processingRegistry().algorithmById(id)
    if alg is not None:
        print(str(alg))
        algorithmOptions(id)
    else:
        print('Algorithm "{}" not found.'.format(id))


def run(algOrName, parameters, onFinish=None, feedback=None, context=None):
    """Executes given algorithm and returns its outputs as dictionary
    object.
    """
    return Processing.runAlgorithm(algOrName, parameters, onFinish, feedback, context)


def runAndLoadResults(algOrName, parameters, feedback=None, context=None):
    """Executes given algorithm and load its results into QGIS project
    when possible.
    """
    if isinstance(algOrName, QgsProcessingAlgorithm):
        alg = algOrName
    else:
        alg = QgsApplication.processingRegistry().algorithmById(algOrName)

    # output destination parameters to point to current project
    for param in alg.parameterDefinitions():
        if not param.name() in parameters:
            continue

        if isinstance(param, (QgsProcessingParameterFeatureSink, QgsProcessingParameterVectorOutput, QgsProcessingParameterRasterOutput)):
            p = parameters[param.name()]
            if not isinstance(p, QgsProcessingOutputLayerDefinition):
                parameters[param.name()] = QgsProcessingOutputLayerDefinition(p, QgsProject.instance())
            else:
                p.destinationProject = QgsProject.instance()
                parameters[param.name()] = p

    return Processing.runAlgorithm(alg, parameters=parameters, onFinish=handleAlgorithmResults, feedback=feedback, context=context)


def version():
    pluginPath = os.path.split(os.path.dirname(__file__))[0]
    cfg = configparser.ConfigParser()
    cfg.read(os.path.join(pluginPath, 'metadata.txt'))
    ver = cfg.get('general', 'version').split('.')
    return 10000 * int(ver[0]) + 100 * int(ver[1]) + int(ver[2])
