# -*- coding: utf-8 -*-

"""
***************************************************************************
    Processing.py
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
import traceback

from qgis.PyQt.QtCore import Qt, QCoreApplication
from qgis.PyQt.QtWidgets import QApplication
from qgis.PyQt.QtGui import QCursor

from qgis.utils import iface
from qgis.core import (QgsMessageLog,
                       QgsApplication,
                       QgsMapLayer,
                       QgsProcessingProvider,
                       QgsProcessingAlgorithm,
                       QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingOutputVectorLayer,
                       QgsProcessingOutputRasterLayer,
                       QgsProcessingOutputMapLayer,
                       QgsProcessingOutputMultipleLayers,
                       QgsProcessingFeedback)

import processing
from processing.core.ProcessingConfig import ProcessingConfig
from processing.gui.MessageBarProgress import MessageBarProgress
from processing.gui.RenderingStyles import RenderingStyles
from processing.gui.Postprocessing import handleAlgorithmResults
from processing.gui.AlgorithmExecutor import execute
from processing.script import ScriptUtils
from processing.tools import dataobjects

from processing.algs.qgis.QgisAlgorithmProvider import QgisAlgorithmProvider  # NOQA
from processing.algs.grass7.Grass7AlgorithmProvider import Grass7AlgorithmProvider
from processing.algs.gdal.GdalAlgorithmProvider import GdalAlgorithmProvider  # NOQA
from processing.algs.otb.OtbAlgorithmProvider import OtbAlgorithmProvider  # NOQA
from processing.algs.saga.SagaAlgorithmProvider import SagaAlgorithmProvider  # NOQA
from processing.script.ScriptAlgorithmProvider import ScriptAlgorithmProvider  # NOQA
#from processing.preconfigured.PreconfiguredAlgorithmProvider import PreconfiguredAlgorithmProvider  # NOQA

# should be loaded last - ensures that all dependent algorithms are available when loading models
from processing.modeler.ModelerAlgorithmProvider import ModelerAlgorithmProvider  # NOQA
from processing.modeler.ProjectProvider import ProjectProvider # NOQA


class Processing(object):
    BASIC_PROVIDERS = []

    @staticmethod
    def activateProvider(providerOrName, activate=True):
        provider_id = providerOrName.id() if isinstance(providerOrName, QgsProcessingProvider) else providerOrName
        provider = QgsApplication.processingRegistry().providerById(provider_id)
        try:
            provider.setActive(True)
            provider.refreshAlgorithms()
        except:
            # provider could not be activated
            QgsMessageLog.logMessage(Processing.tr('Error: Provider {0} could not be activated\n').format(provider_id),
                                     Processing.tr("Processing"))

    @staticmethod
    def initialize():
        if "model" in [p.id() for p in QgsApplication.processingRegistry().providers()]:
            return
        # Add the basic providers
        for c in [
            QgisAlgorithmProvider,
            Grass7AlgorithmProvider,
            GdalAlgorithmProvider,
            OtbAlgorithmProvider,
            SagaAlgorithmProvider,
            ScriptAlgorithmProvider,
            ModelerAlgorithmProvider,
            ProjectProvider
        ]:
            p = c()
            if QgsApplication.processingRegistry().addProvider(p):
                Processing.BASIC_PROVIDERS.append(p)
        # And initialize
        ProcessingConfig.initialize()
        ProcessingConfig.readSettings()
        RenderingStyles.loadStyles()

    @staticmethod
    def deinitialize():
        for p in Processing.BASIC_PROVIDERS:
            QgsApplication.processingRegistry().removeProvider(p)

        Processing.BASIC_PROVIDERS = []

    @staticmethod
    def runAlgorithm(algOrName, parameters, onFinish=None, feedback=None, context=None):
        if isinstance(algOrName, QgsProcessingAlgorithm):
            alg = algOrName
        else:
            alg = QgsApplication.processingRegistry().createAlgorithmById(algOrName)

        if feedback is None:
            feedback = QgsProcessingFeedback()

        if alg is None:
            msg = Processing.tr('Error: Algorithm {0} not found\n').format(algOrName)
            feedback.reportError(msg)
            raise QgsProcessingException(msg)

        if context is None:
            context = dataobjects.createContext(feedback)

        if context.feedback() is None:
            context.setFeedback(feedback)

        ok, msg = alg.checkParameterValues(parameters, context)
        if not ok:
            msg = Processing.tr('Unable to execute algorithm\n{0}').format(msg)
            feedback.reportError(msg)
            raise QgsProcessingException(msg)

        if not alg.validateInputCrs(parameters, context):
            feedback.pushInfo(
                Processing.tr('Warning: Not all input layers use the same CRS.\nThis can cause unexpected results.'))

        ret, results = execute(alg, parameters, context, feedback)
        if ret:
            feedback.pushInfo(
                Processing.tr('Results: {}').format(results))

            if onFinish is not None:
                onFinish(alg, context, feedback)
            else:
                # auto convert layer references in results to map layers
                for out in alg.outputDefinitions():
                    if out.name() not in results:
                        continue

                    if isinstance(out, (QgsProcessingOutputVectorLayer, QgsProcessingOutputRasterLayer, QgsProcessingOutputMapLayer)):
                        result = results[out.name()]
                        if not isinstance(result, QgsMapLayer):
                            layer = context.takeResultLayer(result) # transfer layer ownership out of context
                            if layer:
                                results[out.name()] = layer # replace layer string ref with actual layer (+ownership)
                    elif isinstance(out, QgsProcessingOutputMultipleLayers):
                        result = results[out.name()]
                        if result:
                            layers_result = []
                            for l in result:
                                if not isinstance(result, QgsMapLayer):
                                    layer = context.takeResultLayer(l) # transfer layer ownership out of context
                                    if layer:
                                        layers_result.append(layer)
                                    else:
                                        layers_result.append(l)
                                else:
                                    layers_result.append(l)

                            results[out.name()] = layers_result # replace layers strings ref with actual layers (+ownership)

        else:
            msg = Processing.tr("There were errors executing the algorithm.")
            feedback.reportError(msg)
            raise QgsProcessingException(msg)

        if isinstance(feedback, MessageBarProgress):
            feedback.close()
        return results

    @staticmethod
    def tr(string, context=''):
        if context == '':
            context = 'Processing'
        return QCoreApplication.translate(context, string)
