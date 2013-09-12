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
from processing import interface

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import processing
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from processing.tools import dataobjects
from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.gui.AlgorithmClassification import AlgorithmDecorator
from processing.gui.AlgorithmExecutor import AlgorithmExecutor
from processing.gui.RenderingStyles import RenderingStyles
from processing.gui.Postprocessing import Postprocessing
from processing.gui.UnthreadedAlgorithmExecutor import UnthreadedAlgorithmExecutor
from processing.core.SilentProgress import SilentProgress
from processing.modeler.Providers import Providers
from processing.modeler.ModelerAlgorithmProvider import ModelerAlgorithmProvider
from processing.modeler.ModelerOnlyAlgorithmProvider import ModelerOnlyAlgorithmProvider
from processing.algs.QGISAlgorithmProvider import QGISAlgorithmProvider
from processing.grass.GrassAlgorithmProvider import GrassAlgorithmProvider
from processing.lidar.LidarToolsAlgorithmProvider import LidarToolsAlgorithmProvider
from processing.gdal.GdalOgrAlgorithmProvider import GdalOgrAlgorithmProvider
from processing.otb.OTBAlgorithmProvider import OTBAlgorithmProvider
from processing.r.RAlgorithmProvider import RAlgorithmProvider
from processing.saga.SagaAlgorithmProvider import SagaAlgorithmProvider
from processing.script.ScriptAlgorithmProvider import ScriptAlgorithmProvider
from processing.taudem.TauDEMAlgorithmProvider import TauDEMAlgorithmProvider
from processing.admintools.AdminToolsAlgorithmProvider import AdminToolsAlgorithmProvider

class Processing:

    listeners = []
    providers = []
    #a dictionary of algorithms. Keys are names of providers
    #and values are list with all algorithms from that provider
    algs = {}
    #Same structure as algs
    actions = {}
    #All the registered context menu actions for the toolbox
    contextMenuActions = []

    modeler = ModelerAlgorithmProvider()

    @staticmethod
    def addProvider(provider, updateList = False):
        '''use this method to add algorithms from external providers'''
        '''Adding a new provider automatically initializes it, so there is no need to do it in advance'''
        #Note: this might slow down the initialization process if there are many new providers added.
        #Should think of a different solution
        provider.initializeSettings()
        Processing.providers.append(provider)
        ProcessingConfig.loadSettings()
        if updateList:
            Processing.updateAlgsList()

    @staticmethod
    def removeProvider(provider):
        '''Use this method to remove a provider.
        This method should be called when unloading a plugin that contributes a provider'''
        try:
            provider.unload()
            Processing.providers.remove(provider)
            ProcessingConfig.loadSettings()
            Processing.updateAlgsList()
        except:
            pass #This try catch block is here to avoid problems if the plugin with a provider is unloaded
                 #after the processing framework itself has been unloaded. It is a quick fix before I found out how to
                 #properly avoid that

    @staticmethod
    def getProviderFromName(name):
        '''returns the provider with the given name'''
        for provider in Processing.providers:
            if provider.getName() == name:
                return provider
        return Processing.modeler

    @staticmethod
    def getInterface():
        return interface.iface

    @staticmethod
    def setInterface(iface):
        pass#Processing.iface = iface


    @staticmethod
    def initialize():
        #add the basic providers
        Processing.addProvider(QGISAlgorithmProvider())
        Processing.addProvider(ModelerOnlyAlgorithmProvider())
        Processing.addProvider(GdalOgrAlgorithmProvider())
        Processing.addProvider(LidarToolsAlgorithmProvider())
        Processing.addProvider(OTBAlgorithmProvider())
        Processing.addProvider(RAlgorithmProvider())
        Processing.addProvider(SagaAlgorithmProvider())
        Processing.addProvider(GrassAlgorithmProvider())
        Processing.addProvider(ScriptAlgorithmProvider())
        Processing.addProvider(TauDEMAlgorithmProvider())
        Processing.addProvider(AdminToolsAlgorithmProvider())
        Processing.modeler.initializeSettings();
        #and initialize
        AlgorithmDecorator.loadClassification()
        ProcessingLog.startLogging()
        ProcessingConfig.initialize()
        ProcessingConfig.loadSettings()
        RenderingStyles.loadStyles()
        Processing.loadFromProviders()

    @staticmethod
    def updateAlgsList():
        '''call this method when there has been any change that requires the list of algorithms
        to be created again from algorithm providers'''
        Processing.loadFromProviders()
        Processing.fireAlgsListHasChanged()

    @staticmethod
    def loadFromProviders():
        Processing.loadAlgorithms()
        Processing.loadActions()
        Processing.loadContextMenuActions()

    @staticmethod
    def updateProviders():
        for provider in Processing.providers:
            provider.loadAlgorithms()

    @staticmethod
    def addAlgListListener(listener):
        '''listener should implement a algsListHasChanged() method. Whenever the list of algorithms changes,
        that method will be called for all registered listeners'''
        Processing.listeners.append(listener)

    @staticmethod
    def fireAlgsListHasChanged():
        for listener in Processing.listeners:
            listener.algsListHasChanged()

    @staticmethod
    def loadAlgorithms():
        Processing.algs={}
        Processing.updateProviders()
        for provider in Processing.providers:
            providerAlgs = provider.algs
            algs = {}
            for alg in providerAlgs:
                algs[alg.commandLineName()] = alg
            Processing.algs[provider.getName()] = algs

        #this is a special provider, since it depends on others
        #TODO Fix circular imports, so this provider can be incorporated
        #as a normal one
        provider = Processing.modeler
        provider.setAlgsList(Processing.algs)
        provider.loadAlgorithms()
        providerAlgs = provider.algs
        algs = {}
        for alg in providerAlgs:
            algs[alg.commandLineName()] = alg
        Processing.algs[provider.getName()] = algs
        #And we do it again, in case there are models containing models
        #TODO: Improve this
        provider.setAlgsList(Processing.algs)
        provider.loadAlgorithms()
        providerAlgs = provider.algs
        algs = {}
        for alg in providerAlgs:
            algs[alg.commandLineName()] = alg
        Processing.algs[provider.getName()] = algs
        provs = {}
        for provider in Processing.providers:
            provs[provider.getName()] = provider
        provs[Processing.modeler.getName()] = Processing.modeler
        Providers.providers = provs

    @staticmethod
    def loadActions():
        for provider in Processing.providers:
            providerActions = provider.actions
            actions = list()
            for action in providerActions:
                actions.append(action)
            Processing.actions[provider.getName()] = actions

        provider = Processing.modeler
        actions = list()
        for action in provider.actions:
            actions.append(action)
        Processing.actions[provider.getName()] = actions

    @staticmethod
    def loadContextMenuActions():
        Processing.contextMenuActions = []
        for provider in Processing.providers:
            providerActions = provider.contextMenuActions
            for action in providerActions:
                Processing.contextMenuActions.append(action)

        provider = Processing.modeler
        providerActions = provider.contextMenuActions
        for action in providerActions:
            Processing.contextMenuActions.append(action)

    @staticmethod
    def getAlgorithm(name):
        for provider in Processing.algs.values():
            if name in provider:
                return provider[name]
        return None

    @staticmethod
    def getAlgorithmFromFullName(name):
        for provider in Processing.algs.values():
            for alg in provider.values():
                if alg.name == name:
                    return alg
        return None

    @staticmethod
    def getObject(uri):
        '''Returns the QGIS object identified by the given URI'''
        return dataobjects.getObjectFromUri(uri)

    @staticmethod
    def runandload(name, *args):
        Processing.runAlgorithm(name, Postprocessing.handleAlgorithmResults, *args)

    @staticmethod
    def runAlgorithm(algOrName, onFinish, *args):
        if isinstance(algOrName, GeoAlgorithm):
            alg = algOrName
        else:
            alg = Processing.getAlgorithm(algOrName)
        if alg == None:
            print("Error: Algorithm not found\n")
            return
        if len(args) != alg.getVisibleParametersCount() + alg.getVisibleOutputsCount():
            print ("Error: Wrong number of parameters")
            processing.alghelp(algOrName)
            return

        alg = alg.getCopy()
        if isinstance(args, dict):
            # set params by name
            for name, value in args.items():
                if alg.getParameterFromName(name).setValue(value):
                    continue;
                if alg.getOutputFromName(name).setValue(value):
                    continue;
                print ("Error: Wrong parameter value %s for parameter %s." % (value, name))
                return
        else:
            i = 0
            for param in alg.parameters:
                if not param.hidden:
                    if not param.setValue(args[i]):
                        print ("Error: Wrong parameter value: " + unicode(args[i]))
                        return
                    i = i +1

            for output in alg.outputs:
                if not output.hidden:
                    if not output.setValue(args[i]):
                        print ("Error: Wrong output value: " + unicode(args[i]))
                        return
                    i = i +1

        msg = alg.checkParameterValuesBeforeExecuting()
        if msg:
            print ("Unable to execute algorithm\n" + msg)
            return

        if not alg.checkInputCRS():
            print ("Warning: Not all input layers use the same CRS.\n" +
                   "This can cause unexpected results.")

        ProcessingLog.addToLog(ProcessingLog.LOG_ALGORITHM, alg.getAsCommand())

        # don't set the wait cursor twice, because then when you restore it
        # it will still be a wait cursor
        cursor = QApplication.overrideCursor()
        if cursor == None or cursor == 0:
            QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
        elif cursor.shape() != Qt.WaitCursor:
            QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))

        useThreads = ProcessingConfig.getSetting(ProcessingConfig.USE_THREADS)

        #this is doing strange things, so temporarily the thread execution is disabled from the console
        useThreads = False

        if useThreads:
            algEx = AlgorithmExecutor(alg)
            progress = QProgressDialog()
            progress.setWindowTitle(alg.name)
            progress.setLabelText("Executing %s..." % alg.name)
            def finish():
                QApplication.restoreOverrideCursor()
                if onFinish is not None:
                    onFinish(alg, SilentProgress())
                progress.close()
            def error(msg):
                QApplication.restoreOverrideCursor()
                print msg
                ProcessingLog.addToLog(ProcessingLog.LOG_ERROR, msg)
            def cancel():
                try:
                    algEx.finished.disconnect()
                    algEx.terminate()
                    QApplication.restoreOverrideCursor()
                    progress.close()
                except:
                    pass
            algEx.error.connect(error)
            algEx.finished.connect(finish)
            algEx.start()
            algEx.wait()
        else:
            progress = SilentProgress()
            ret = UnthreadedAlgorithmExecutor.runalg(alg, progress)
            if onFinish is not None and ret:
                onFinish(alg, progress)
            QApplication.restoreOverrideCursor()
        return alg



