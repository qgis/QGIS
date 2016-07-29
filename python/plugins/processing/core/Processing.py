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

import sys
import os
import traceback

from qgis.PyQt.QtCore import Qt, QCoreApplication, QObject, pyqtSignal
from qgis.PyQt.QtWidgets import QApplication
from qgis.PyQt.QtGui import QCursor

from qgis.utils import iface
from qgis.core import QgsMessageLog

import processing
from processing.core.AlgorithmProvider import AlgorithmProvider
from processing.script.ScriptUtils import ScriptUtils
from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.gui.MessageBarProgress import MessageBarProgress
from processing.gui.RenderingStyles import RenderingStyles
from processing.gui.Postprocessing import handleAlgorithmResults
from processing.gui.AlgorithmExecutor import runalg
from processing.tools import dataobjects
from processing.core.alglist import algList

from processing.modeler.ModelerAlgorithmProvider import ModelerAlgorithmProvider
from processing.modeler.ModelerOnlyAlgorithmProvider import ModelerOnlyAlgorithmProvider
from processing.algs.qgis.QGISAlgorithmProvider import QGISAlgorithmProvider
from processing.algs.grass.GrassAlgorithmProvider import GrassAlgorithmProvider
from processing.algs.grass7.Grass7AlgorithmProvider import Grass7AlgorithmProvider
from processing.algs.lidar.LidarToolsAlgorithmProvider import LidarToolsAlgorithmProvider
from processing.algs.gdal.GdalOgrAlgorithmProvider import GdalOgrAlgorithmProvider
from processing.algs.otb.OTBAlgorithmProvider import OTBAlgorithmProvider
from processing.algs.r.RAlgorithmProvider import RAlgorithmProvider
from processing.algs.saga.SagaAlgorithmProvider import SagaAlgorithmProvider
from processing.script.ScriptAlgorithmProvider import ScriptAlgorithmProvider
from processing.algs.taudem.TauDEMAlgorithmProvider import TauDEMAlgorithmProvider
from processing.preconfigured.PreconfiguredAlgorithmProvider import PreconfiguredAlgorithmProvider


class Processing:

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

        if provider.getName() in [p.getName() for p in algList.providers]:
            return
        try:
            provider.initializeSettings()
            Processing.providers.append(provider)
            ProcessingConfig.readSettings()
            provider.loadAlgorithms()
            Processing.actions[provider.getName()] = provider.actions
            Processing.contextMenuActions.extend(provider.contextMenuActions)
            algList.addProvider(provider)
        except:
            ProcessingLog.addToLog(
                ProcessingLog.LOG_ERROR,
                Processing.tr('Could not load provider: %s\n%s')
                % (provider.getDescription(), traceback.format_exc()))
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
                if p.getName() == provider.getName():
                    Processing.providers.remove(p)
            algList.removeProvider(provider.getName())
            if provider.getName() in Processing.actions:
                del Processing.actions[provider.getName()]
            for act in provider.contextMenuActions:
                Processing.contextMenuActions.remove(act)
        except:
            # This try catch block is here to avoid problems if the
            # plugin with a provider is unloaded after the Processing
            # framework itself has been unloaded. It is a quick fix
            # before I find out how to properly avoid that.
            pass

    @staticmethod
    def getProviderFromName(name):
        """Returns the provider with the given name."""
        return algList.getProviderFromName(name)

    @staticmethod
    def activateProvider(providerOrName, activate=True):
        providerName = providerOrName.getName() if isinstance(providerOrName, AlgorithmProvider) else providerOrName
        name = 'ACTIVATE_' + providerName.upper().replace(' ', '_')
        ProcessingConfig.setSettingValue(name, activate)
        algList.providerUpdated.emit(providerName)

    @staticmethod
    def initialize():
        if "model" in [p.getName() for p in Processing.providers]:
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
        provider = Processing.getProviderFromName("qgis")
        scripts = ScriptUtils.loadFromFolder(folder)
        print scripts
        for script in scripts:
            script.allowEdit = False
            script._icon = provider._icon
            script.provider = provider
        provider.externalAlgs.extend(scripts)
        Processing.reloadProvider("qgis")

    @staticmethod
    def removeScripts(folder):
        provider = Processing.getProviderFromName("qgis")
        for alg in provider.externalAlgs[::-1]:
            path = os.path.dirname(alg.descriptionFile)
            if path == folder:
                provider.externalAlgs.remove(alg)
        Processing.reloadProvider("qgis")

    @staticmethod
    def updateAlgsList():
        """Call this method when there has been any change that
        requires the list of algorithms to be created again from
        algorithm providers.
        """
        QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
        for p in Processing.providers:
            Processing.reloadProvider(p)
        QApplication.restoreOverrideCursor()

    @staticmethod
    def reloadProvider(providerName):
        algList.reloadProvider(providerName)

    @staticmethod
    def getAlgorithm(name):
        return algList.getAlgorithm(name)

    @staticmethod
    def getAlgorithmFromFullName(name):
        return algList.getAlgorithmFromFullName(name)

    @staticmethod
    def getObject(uri):
        """Returns the QGIS object identified by the given URI."""
        return dataobjects.getObjectFromUri(uri)

    @staticmethod
    def runandload(name, *args):
        Processing.runAlgorithm(name, handleAlgorithmResults, *args)

    @staticmethod
    def runAlgorithm(algOrName, onFinish, *args, **kwargs):
        if isinstance(algOrName, GeoAlgorithm):
            alg = algOrName
        else:
            alg = Processing.getAlgorithm(algOrName)
        if alg is None:
            print 'Error: Algorithm not found\n'
            QgsMessageLog.logMessage(Processing.tr('Error: Algorithm {0} not found\n').format(algOrName), Processing.tr("Processing"))
            return
        alg = alg.getCopy()

        if len(args) == 1 and isinstance(args[0], dict):
            # Set params by name and try to run the alg even if not all parameter values are provided,
            # by using the default values instead.
            setParams = []
            for (name, value) in args[0].items():
                param = alg.getParameterFromName(name)
                if param and param.setValue(value):
                    setParams.append(name)
                    continue
                output = alg.getOutputFromName(name)
                if output and output.setValue(value):
                    continue
                print 'Error: Wrong parameter value %s for parameter %s.' % (value, name)
                QgsMessageLog.logMessage(Processing.tr('Error: Wrong parameter value {0} for parameter {1}.').format(value, name), Processing.tr("Processing"))
                ProcessingLog.addToLog(
                    ProcessingLog.LOG_ERROR,
                    Processing.tr('Error in %s. Wrong parameter value %s for parameter %s.') % (
                        alg.name, value, name)
                )
                return
            # fill any missing parameters with default values if allowed
            for param in alg.parameters:
                if param.name not in setParams:
                    if not param.setDefaultValue():
                        print 'Error: Missing parameter value for parameter %s.' % param.name
                        QgsMessageLog.logMessage(Processing.tr('Error: Missing parameter value for parameter {0}.').format(param.name), Processing.tr("Processing"))
                        ProcessingLog.addToLog(
                            ProcessingLog.LOG_ERROR,
                            Processing.tr('Error in %s. Missing parameter value for parameter %s.') % (
                                alg.name, param.name)
                        )
                        return
        else:
            if len(args) != alg.getVisibleParametersCount() + alg.getVisibleOutputsCount():
                print 'Error: Wrong number of parameters'
                QgsMessageLog.logMessage(Processing.tr('Error: Wrong number of parameters'), Processing.tr("Processing"))
                processing.alghelp(algOrName)
                return
            i = 0
            for param in alg.parameters:
                if not param.hidden:
                    if not param.setValue(args[i]):
                        print'Error: Wrong parameter value: ' + unicode(args[i])
                        QgsMessageLog.logMessage(Processing.tr('Error: Wrong parameter value: ') + unicode(args[i]), Processing.tr("Processing"))
                        return
                    i = i + 1

            for output in alg.outputs:
                if not output.hidden:
                    if not output.setValue(args[i]):
                        print 'Error: Wrong output value: ' + unicode(args[i])
                        QgsMessageLog.logMessage(Processing.tr('Error: Wrong output value: ') + unicode(args[i]), Processing.tr("Processing"))
                        return
                    i = i + 1

        msg = alg._checkParameterValuesBeforeExecuting()
        if msg:
            print 'Unable to execute algorithm\n' + unicode(msg)
            QgsMessageLog.logMessage(Processing.tr('Unable to execute algorithm\n{0}').format(msg), Processing.tr("Processing"))
            return

        if not alg.checkInputCRS():
            print('Warning: Not all input layers use the same CRS.\n'
                  + 'This can cause unexpected results.')
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

        progress = None
        if kwargs is not None and "progress" in kwargs.keys():
            progress = kwargs["progress"]
        elif iface is not None:
            progress = MessageBarProgress(alg.name)

        ret = runalg(alg, progress)
        if ret:
            if onFinish is not None:
                onFinish(alg, progress)
        else:
            QgsMessageLog.logMessage(Processing.tr("There were errors executing the algorithm."), Processing.tr("Processing"))

        if overrideCursor:
            QApplication.restoreOverrideCursor()
        if isinstance(progress, MessageBarProgress):
            progress.close()
        return alg

    @staticmethod
    def tr(string, context=''):
        if context == '':
            context = 'Processing'
        return QCoreApplication.translate(context, string)
