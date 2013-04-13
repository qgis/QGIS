# -*- coding: utf-8 -*-

"""
***************************************************************************
    Sextante.py
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

import sextante
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.core.QGisLayers import QGisLayers
from sextante.core.SextanteConfig import SextanteConfig
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.SextanteLog import SextanteLog
from sextante.gui.AlgorithmClassification import AlgorithmDecorator
from sextante.gui.AlgorithmExecutor import AlgorithmExecutor
from sextante.gui.RenderingStyles import RenderingStyles
from sextante.gui.SextantePostprocessing import SextantePostprocessing
from sextante.gui.UnthreadedAlgorithmExecutor import UnthreadedAlgorithmExecutor
from sextante.core.SilentProgress import SilentProgress
from sextante.modeler.Providers import Providers
from sextante.modeler.ModelerAlgorithmProvider import ModelerAlgorithmProvider
from sextante.modeler.ModelerOnlyAlgorithmProvider import ModelerOnlyAlgorithmProvider
from sextante.algs.QGISAlgorithmProvider import QGISAlgorithmProvider
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.grass.GrassAlgorithmProvider import GrassAlgorithmProvider
from sextante.lidar.LidarToolsAlgorithmProvider import LidarToolsAlgorithmProvider
from sextante.gdal.GdalOgrAlgorithmProvider import GdalOgrAlgorithmProvider
from sextante.otb.OTBAlgorithmProvider import OTBAlgorithmProvider
from sextante.r.RAlgorithmProvider import RAlgorithmProvider
from sextante.saga.SagaAlgorithmProvider import SagaAlgorithmProvider
from sextante.script.ScriptAlgorithmProvider import ScriptAlgorithmProvider
from sextante.taudem.TauDEMAlgorithmProvider import TauDEMAlgorithmProvider
from sextante.admintools.AdminToolsAlgorithmProvider import AdminToolsAlgorithmProvider

class Sextante:

    iface = None
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
        Sextante.providers.append(provider)
        SextanteConfig.loadSettings()
        if updateList:
            Sextante.updateAlgsList()

    @staticmethod
    def removeProvider(provider):
        '''Use this method to remove a provider.
        This method should be called when unloading a plugin that contributes a provider to SEXTANTE'''
        try:
            provider.unload()
            Sextante.providers.remove(provider)
            SextanteConfig.loadSettings()
            Sextante.updateAlgsList()
        except:
            pass #This try catch block is here to avoid problems if the plugin with a provider is unloaded
                 #after SEXTANTE itself has been unloaded. It is a quick fix before I found out how to
                 #properly avoid that

    @staticmethod
    def getProviderFromName(name):
        '''returns the provider with the given name'''
        for provider in Sextante.providers:
            if provider.getName() == name:
                return provider
        return Sextante.modeler

    @staticmethod
    def getInterface():
        return Sextante.iface

    @staticmethod
    def setInterface(iface):
        Sextante.iface = iface

    @staticmethod
    def initialize():
        #add the basic providers
        Sextante.addProvider(QGISAlgorithmProvider())
        Sextante.addProvider(ModelerOnlyAlgorithmProvider())
        Sextante.addProvider(GdalOgrAlgorithmProvider())
        Sextante.addProvider(LidarToolsAlgorithmProvider())
        Sextante.addProvider(OTBAlgorithmProvider())
        Sextante.addProvider(RAlgorithmProvider())
        Sextante.addProvider(SagaAlgorithmProvider())
        Sextante.addProvider(GrassAlgorithmProvider())
        Sextante.addProvider(ScriptAlgorithmProvider())
        Sextante.addProvider(TauDEMAlgorithmProvider())
        Sextante.addProvider(AdminToolsAlgorithmProvider())
        Sextante.modeler.initializeSettings();
        #and initialize
        AlgorithmDecorator.loadClassification()
        SextanteLog.startLogging()
        SextanteConfig.initialize()
        SextanteConfig.loadSettings()
        RenderingStyles.loadStyles()
        Sextante.loadFromProviders()

    @staticmethod
    def updateAlgsList():
        '''call this method when there has been any change that requires the list of algorithms
        to be created again from algorithm providers'''
        Sextante.loadFromProviders()
        Sextante.fireAlgsListHasChanged()

    @staticmethod
    def loadFromProviders():
        Sextante.loadAlgorithms()
        Sextante.loadActions()
        Sextante.loadContextMenuActions()

    @staticmethod
    def updateProviders():
        for provider in Sextante.providers:
            provider.loadAlgorithms()

    @staticmethod
    def addAlgListListener(listener):
        '''listener should implement a algsListHasChanged() method. Whenever the list of algorithms changes,
        that method will be called for all registered listeners'''
        Sextante.listeners.append(listener)

    @staticmethod
    def fireAlgsListHasChanged():
        for listener in Sextante.listeners:
            listener.algsListHasChanged()

    @staticmethod
    def loadAlgorithms():
        Sextante.algs={}
        Sextante.updateProviders()
        for provider in Sextante.providers:
            providerAlgs = provider.algs
            algs = {}
            for alg in providerAlgs:
                algs[alg.commandLineName()] = alg
            Sextante.algs[provider.getName()] = algs

        #this is a special provider, since it depends on others
        #TODO Fix circular imports, so this provider can be incorporated
        #as a normal one
        provider = Sextante.modeler
        provider.setAlgsList(Sextante.algs)
        provider.loadAlgorithms()
        providerAlgs = provider.algs
        algs = {}
        for alg in providerAlgs:
            algs[alg.commandLineName()] = alg
        Sextante.algs[provider.getName()] = algs
        #And we do it again, in case there are models containing models
        #TODO: Improve this
        provider.setAlgsList(Sextante.algs)
        provider.loadAlgorithms()
        providerAlgs = provider.algs
        algs = {}
        for alg in providerAlgs:
            algs[alg.commandLineName()] = alg
        Sextante.algs[provider.getName()] = algs
        provs = {}
        for provider in Sextante.providers:
            provs[provider.getName()] = provider
        provs[Sextante.modeler.getName()] = Sextante.modeler
        Providers.providers = provs

    @staticmethod
    def loadActions():
        for provider in Sextante.providers:
            providerActions = provider.actions
            actions = list()
            for action in providerActions:
                actions.append(action)
            Sextante.actions[provider.getName()] = actions

        provider = Sextante.modeler
        actions = list()
        for action in provider.actions:
            actions.append(action)
        Sextante.actions[provider.getName()] = actions

    @staticmethod
    def loadContextMenuActions():
        Sextante.contextMenuActions = []
        for provider in Sextante.providers:
            providerActions = provider.contextMenuActions
            for action in providerActions:
                Sextante.contextMenuActions.append(action)

        provider = Sextante.modeler
        providerActions = provider.contextMenuActions
        for action in providerActions:
            Sextante.contextMenuActions.append(action)

    @staticmethod
    def getAlgorithm(name):
        for provider in Sextante.algs.values():
            if name in provider:
                return provider[name]
        return None

    @staticmethod
    def getObject(uri):
        '''Returns the QGIS object identified by the given URI'''
        return QGisLayers.getObjectFromUri(uri)

    @staticmethod
    def runandload(name, *args):
        Sextante.runAlgorithm(name, SextantePostprocessing.handleAlgorithmResults, *args)

    @staticmethod
    def runAlgorithm(algOrName, onFinish, *args):
        if isinstance(algOrName, GeoAlgorithm):
            alg = algOrName
        else:
            alg = Sextante.getAlgorithm(algOrName)
        if alg == None:
            print("Error: Algorithm not found\n")
            return
        if len(args) != alg.getVisibleParametersCount() + alg.getVisibleOutputsCount():
            print ("Error: Wrong number of parameters")
            sextante.alghelp(algOrName)
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

            settrace()
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

        SextanteLog.addToLog(SextanteLog.LOG_ALGORITHM, alg.getAsCommand())

        # don't set the wait cursor twice, because then when you restore it
        # it will still be a wait cursor
        cursor = QApplication.overrideCursor()
        if cursor == None or cursor == 0:
            QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
        elif cursor.shape() != Qt.WaitCursor:
            QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))

        useThreads = SextanteConfig.getSetting(SextanteConfig.USE_THREADS)

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
                SextanteLog.addToLog(SextanteLog.LOG_ERROR, msg)
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



