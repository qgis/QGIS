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
from __future__ import print_function
from builtins import str
from builtins import object


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
                       QgsApplication)

import processing
from processing.core.AlgorithmProvider import AlgorithmProvider
from processing.script.ScriptUtils import ScriptUtils
from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.gui.MessageBarProgress import MessageBarProgress
from processing.gui.RenderingStyles import RenderingStyles
from processing.gui.Postprocessing import handleAlgorithmResults
from processing.gui.AlgorithmExecutor import execute
from processing.tools import dataobjects
from processing.core.alglist import algList

from processing.modeler.ModelerAlgorithmProvider import ModelerAlgorithmProvider  # NOQA
from processing.algs.qgis.QGISAlgorithmProvider import QGISAlgorithmProvider  # NOQA
from processing.algs.grass7.Grass7AlgorithmProvider import Grass7AlgorithmProvider  # NOQA
from processing.algs.gdal.GdalAlgorithmProvider import GdalAlgorithmProvider  # NOQA
from processing.algs.r.RAlgorithmProvider import RAlgorithmProvider  # NOQA
from processing.algs.saga.SagaAlgorithmProvider import SagaAlgorithmProvider  # NOQA
from processing.script.ScriptAlgorithmProvider import ScriptAlgorithmProvider  # NOQA
from processing.preconfigured.PreconfiguredAlgorithmProvider import PreconfiguredAlgorithmProvider  # NOQA


class Processing(object):

    providers = []

    # Same structure as algs in algList
    actions = {}

    # All the registered context menu actions for the toolbox
    contextMenuActions = []

    @staticmethod
    def algs():
        """Use this method to get algorithms for wps4server.
        """
        return algList.algs

    @staticmethod
    def addProvider(provider, updateList=True):
        """Use this method to add algorithms from external providers.
        """

        if provider.id() in [p.id() for p in QgsApplication.processingRegistry().providers()]:
            return
        try:
            provider.initializeSettings()
            Processing.providers.append(provider)
            ProcessingConfig.readSettings()
            provider.loadAlgorithms()
            Processing.actions[provider.id()] = provider.actions
            Processing.contextMenuActions.extend(provider.contextMenuActions)
            algList.addProvider(provider)
        except:
            ProcessingLog.addToLog(
                ProcessingLog.LOG_ERROR,
                Processing.tr('Could not load provider: {0}\n{0}').format(
                    provider.name(), traceback.format_exc()
                )
            )
            Processing.removeProvider(provider)

    @staticmethod
    def removeProvider(provider):
        """Use this method to remove a provider.

        This method should be called when unloading a plugin that
        contributes a provider.
        """
        try:
            provider.unload()
            for p in Processing.providers:
                if p.id() == provider.id():
                    Processing.providers.remove(p)
            algList.removeProvider(provider.id())
            if provider.id() in Processing.actions:
                del Processing.actions[provider.id()]
            for act in provider.contextMenuActions:
                Processing.contextMenuActions.remove(act)
        except:
            # This try catch block is here to avoid problems if the
            # plugin with a provider is unloaded after the Processing
            # framework itself has been unloaded. It is a quick fix
            # before I find out how to properly avoid that.
            pass

    @staticmethod
    def activateProvider(providerOrName, activate=True):
        provider_id = providerOrName.id() if isinstance(providerOrName, AlgorithmProvider) else providerOrName
        name = 'ACTIVATE_' + provider_id.upper().replace(' ', '_')
        ProcessingConfig.setSettingValue(name, activate)
        algList.providerUpdated.emit(provider_id)

    @staticmethod
    def initialize():
        if "model" in [p.id() for p in Processing.providers]:
            return
        # Add the basic providers
        for c in AlgorithmProvider.__subclasses__():
            Processing.addProvider(c())
        # And initialize
        ProcessingConfig.initialize()
        ProcessingConfig.readSettings()
        RenderingStyles.loadStyles()

    @staticmethod
    def addScripts(folder):
        Processing.initialize()
        provider = QgsApplication.processingRegistry().providerById("qgis")
        scripts = ScriptUtils.loadFromFolder(folder)
        # fix_print_with_import
        print(scripts)
        for script in scripts:
            script.allowEdit = False
            script._icon = provider._icon
            script.provider = provider
        provider.externalAlgs.extend(scripts)
        Processing.reloadProvider("qgis")

    @staticmethod
    def removeScripts(folder):
        provider = QgsApplication.processingRegistry().providerById("qgis")
        for alg in provider.externalAlgs[::-1]:
            path = os.path.dirname(alg.descriptionFile)
            if path == folder:
                provider.externalAlgs.remove(alg)
        Processing.reloadProvider("qgis")

    @staticmethod
    def updateAlgsList():
        """Call this method when there has been any change that
        requires the list of algorithms to be created again from
        algorithm providers. Use reloadProvider() for a more fine-grained
        update.
        """
        QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
        for p in Processing.providers:
            Processing.reloadProvider(p.id())
        QApplication.restoreOverrideCursor()

    @staticmethod
    def reloadProvider(provider_id):
        algList.reloadProvider(provider_id)

    @staticmethod
    def getAlgorithm(name):
        return algList.getAlgorithm(name)

    @staticmethod
    def getObject(uri):
        """Returns the QGIS object identified by the given URI."""
        return dataobjects.getObjectFromUri(uri)

    @staticmethod
    def runAlgorithm(algOrName, onFinish, *args, **kwargs):
        if isinstance(algOrName, GeoAlgorithm):
            alg = algOrName
        else:
            alg = Processing.getAlgorithm(algOrName)
        if alg is None:
            # fix_print_with_import
            print('Error: Algorithm not found\n')
            QgsMessageLog.logMessage(Processing.tr('Error: Algorithm {0} not found\n').format(algOrName), Processing.tr("Processing"))
            return
        alg = alg.getCopy()

        if len(args) == 1 and isinstance(args[0], dict):
            # Set params by name and try to run the alg even if not all parameter values are provided,
            # by using the default values instead.
            setParams = []
            for (name, value) in list(args[0].items()):
                param = alg.getParameterFromName(name)
                if param and param.setValue(value):
                    setParams.append(name)
                    continue
                output = alg.getOutputFromName(name)
                if output and output.setValue(value):
                    continue
                # fix_print_with_import
                print('Error: Wrong parameter value %s for parameter %s.' % (value, name))
                QgsMessageLog.logMessage(Processing.tr('Error: Wrong parameter value {0} for parameter {1}.').format(value, name), Processing.tr("Processing"))
                ProcessingLog.addToLog(
                    ProcessingLog.LOG_ERROR,
                    Processing.tr('Error in {0}. Wrong parameter value {1} for parameter {2}.').format(
                        alg.name, value, name
                    )
                )
                return
            # fill any missing parameters with default values if allowed
            for param in alg.parameters:
                if param.name not in setParams:
                    if not param.setDefaultValue():
                        # fix_print_with_import
                        print('Error: Missing parameter value for parameter %s.' % param.name)
                        QgsMessageLog.logMessage(Processing.tr('Error: Missing parameter value for parameter {0}.').format(param.name), Processing.tr("Processing"))
                        ProcessingLog.addToLog(
                            ProcessingLog.LOG_ERROR,
                            Processing.tr('Error in {0}. Missing parameter value for parameter {1}.').format(
                                alg.name, param.name)
                        )
                        return
        else:
            if len(args) != alg.getVisibleParametersCount() + alg.getVisibleOutputsCount():
                # fix_print_with_import
                print('Error: Wrong number of parameters')
                QgsMessageLog.logMessage(Processing.tr('Error: Wrong number of parameters'), Processing.tr("Processing"))
                processing.algorithmHelp(algOrName)
                return
            i = 0
            for param in alg.parameters:
                if not param.hidden:
                    if not param.setValue(args[i]):
                        # fix_print_with_import
                        print('Error: Wrong parameter value: ' + str(args[i]))
                        QgsMessageLog.logMessage(Processing.tr('Error: Wrong parameter value: ') + str(args[i]), Processing.tr("Processing"))
                        return
                    i = i + 1

            for output in alg.outputs:
                if not output.hidden:
                    if not output.setValue(args[i]):
                        # fix_print_with_import
                        print('Error: Wrong output value: ' + str(args[i]))
                        QgsMessageLog.logMessage(Processing.tr('Error: Wrong output value: ') + str(args[i]), Processing.tr("Processing"))
                        return
                    i = i + 1

        msg = alg._checkParameterValuesBeforeExecuting()
        if msg:
            # fix_print_with_import
            print('Unable to execute algorithm\n' + str(msg))
            QgsMessageLog.logMessage(Processing.tr('Unable to execute algorithm\n{0}').format(msg), Processing.tr("Processing"))
            return

        if not alg.checkInputCRS():
            print('Warning: Not all input layers use the same CRS.\n' +
                  'This can cause unexpected results.')
            QgsMessageLog.logMessage(Processing.tr('Warning: Not all input layers use the same CRS.\nThis can cause unexpected results.'), Processing.tr("Processing"))

        # Don't set the wait cursor twice, because then when you
        # restore it, it will still be a wait cursor.
        overrideCursor = False
        if iface is not None:
            cursor = QApplication.overrideCursor()
            if cursor is None or cursor == 0:
                QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
                overrideCursor = True
            elif cursor.shape() != Qt.WaitCursor:
                QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
                overrideCursor = True

        feedback = None
        if kwargs is not None and "feedback" in list(kwargs.keys()):
            feedback = kwargs["feedback"]
        elif iface is not None:
            feedback = MessageBarProgress(alg.name)

        ret = execute(alg, feedback)
        if ret:
            if onFinish is not None:
                onFinish(alg, feedback)
        else:
            QgsMessageLog.logMessage(Processing.tr("There were errors executing the algorithm."), Processing.tr("Processing"))

        if overrideCursor:
            QApplication.restoreOverrideCursor()
        if isinstance(feedback, MessageBarProgress):
            feedback.close()
        return alg

    @staticmethod
    def tr(string, context=''):
        if context == '':
            context = 'Processing'
        return QCoreApplication.translate(context, string)
