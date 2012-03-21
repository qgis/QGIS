from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.saga.SagaAlgorithmProvider import SagaAlgorithmProvider
from sextante.script.ScriptAlgorithmProvider import ScriptAlgorithmProvider
import copy
from sextante.core.QGisLayers import QGisLayers
from sextante.gui.AlgorithmExecutor import AlgorithmExecutor, SilentProgress
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.SextanteConfig import SextanteConfig
from sextante.core.SextanteLog import SextanteLog
from sextante.modeler.ModelerAlgorithmProvider import ModelerAlgorithmProvider
from sextante.mmqgis.MMQGISAlgorithmProvider import MMQGISAlgorithmProvider
from sextante.ftools.FToolsAlgorithmProvider import FToolsAlgorithmProvider
from sextante.gui.SextantePostprocessing import SextantePostprocessing
from sextante.modeler.ProviderIcons import ProviderIcons
from sextante.r.RAlgorithmProvider import RAlgorithmProvider
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.grass.GrassAlgorithmProvider import GrassAlgorithmProvider
from sextante.gui.RenderingStyles import RenderingStyles

class Sextante:

    iface = None
    providers = [SagaAlgorithmProvider(), ScriptAlgorithmProvider(),
                 MMQGISAlgorithmProvider(), FToolsAlgorithmProvider(),
                 RAlgorithmProvider(), GrassAlgorithmProvider()]
    algs = {}
    actions = {}
    contextMenuActions = []
    modeler = ModelerAlgorithmProvider()

    @staticmethod
    def setInterface(iface):
        Sextante.iface = iface

    @staticmethod
    def getProviderFromName(name):
        for provider in Sextante.providers:
            if provider.getName() == name:
                return provider
        return Sextante.modeler

    @staticmethod
    def getInterface():
        return Sextante.iface

    @staticmethod
    def initialize():
        SextanteLog.startLogging()
        SextanteConfig.initialize()
        SextanteConfig.loadSettings()
        RenderingStyles.loadStyles()
        Sextante.loadAlgorithms()
        Sextante.loadActions()
        Sextante.loadContextMenuActions()
        #SextanteConfig.loadSettings()


    @staticmethod
    def updateProviders():
        for provider in Sextante.providers:
            provider.loadAlgorithms()


    @staticmethod
    def loadAlgorithms():
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
        icons = {}
        for provider in Sextante.providers:
            icons[provider.getName()] = provider.getIcon()
        icons[Sextante.modeler.getName()] = Sextante.modeler.getIcon()
        ProviderIcons.providerIcons = icons



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

    ##This methods are here to be used from the python console,
    ##making it easy to use SEXTANTE from there

    @staticmethod
    def alglist(text=None):
        s=""
        for provider in Sextante.algs.values():
            sortedlist = sorted(provider.values(), key= lambda alg: alg.name)
            for alg in sortedlist:
                if text == None or text.lower() in alg.name.lower():
                    s+=(alg.name.ljust(50, "-") + "--->" + alg.commandLineName() + "\n")
        print s


    @staticmethod
    def algoptions(name):
        alg = Sextante.getAlgorithm(name)
        if alg != None:
            s =""
            for param in alg.parameters:
                if isinstance(param, ParameterSelection):
                    s+=param.name + "(" + param.description + ")\n"
                    i=0
                    for option in param.options:
                        s+= "\t" + str(i) + " - " + str(option) + "\n"
                        i+=1
            print(s)
        else:
            print "Algorithm not found"

    @staticmethod
    def alghelp(name):
        alg = Sextante.getAlgorithm(name)
        if alg != None:
            print(str(alg))
        else:
            print "Algorithm not found"


    @staticmethod
    def runalg(name, *args):
        alg = Sextante.getAlgorithm(name)
        if alg == None:
            print("Error: Algorithm not found\n")
            return
        if len(args) != len(alg.parameters) + len(alg.outputs):
            print ("Error: Wrong number of parameters")
            Sextante.alghelp(name)
            return

        alg = copy.deepcopy(alg)
        i = 0
        for param in alg.parameters:
            if not param.setValue(args[i]):
                print ("Error: Wrong parameter value: " + args[i])
                return
            i = i +1

        for output in alg.outputs:
            if not output.setValue(args[i]):
                print ("Error: Wrong output value: " + args[i])
                return
            i = i +1

        SextanteLog.addToLog(SextanteLog.LOG_ALGORITHM, alg.getAsCommand())

        try:
            QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
            AlgorithmExecutor.runalg(alg, SilentProgress())
            QApplication.restoreOverrideCursor()
            return alg.getOutputValuesAsDictionary()
        except GeoAlgorithmExecutionException, e:
            print "*****Error executing algorithm*****"
            print e.msg

    @staticmethod
    def load(layer):
        QGisLayers.load(layer)

    @staticmethod
    def loadFromAlg(layersdict):
        QGisLayers.loadFromDict(layersdict)

    @staticmethod
    def getObject(string):
        QGisLayers.getObjectFromUri(string)

    @staticmethod
    def runandload(name, *args):
        #a quick fix to call algorithms from the history dialog
        alg = Sextante.getAlgorithm(name)
        if alg == None:
            #in theory, this could not happen. Maybe we should show a message box?
            QMessageBox.critical(None,"Error", "Error: Algorithm not found\n")
            return
        if len(args) != len(alg.parameters) + len(alg.outputs):
            QMessageBox.critical(None,"Error", "Error: Wrong number of parameters")
            Sextante.alghelp(name)
            return

        alg = copy.deepcopy(alg)
        i = 0
        for param in alg.parameters:
            if not param.setValue(args[i]):
                QMessageBox.critical(None, "Error", "Error: Wrong parameter value: " + args[i])
                return
            i = i +1

        for output in alg.outputs:
            if not output.setValue(args[i]):
                QMessageBox.critical(None, "Error", "Error: Wrong output value: " + args[i])
                return
            i = i +1

        try:
            QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
            AlgorithmExecutor.runalg(alg, SilentProgress())
            QApplication.restoreOverrideCursor()
            SextantePostprocessing.handleAlgorithmResults(alg)
        except GeoAlgorithmExecutionException, e:
            QMessageBox.critical(None, "Error",  e.msg)




