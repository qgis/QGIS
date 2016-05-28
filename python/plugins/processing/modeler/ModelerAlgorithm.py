# -*- coding: utf-8 -*-

"""
***************************************************************************
    ModelerAlgorithm.py
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

import os.path
import sys
import copy
import time
import json
import codecs
import traceback
from qgis.PyQt.QtCore import QCoreApplication, QPointF
from qgis.PyQt.QtGui import QIcon
from operator import attrgetter

from qgis.core import QgsRasterLayer, QgsVectorLayer
from qgis.gui import QgsMessageBar
from qgis.utils import iface
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.modeler.WrongModelException import WrongModelException
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import (getParameterFromString,
                                        ParameterRaster,
                                        ParameterVector,
                                        ParameterTable,
                                        ParameterTableField,
                                        ParameterBoolean,
                                        ParameterString,
                                        ParameterNumber,
                                        ParameterExtent,
                                        ParameterDataObject,
                                        ParameterMultipleInput)
from processing.tools import dataobjects
from processing.gui.Help2Html import getHtmlFromDescriptionsDict
from processing.core.alglist import algList

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class ModelerParameter():

    def __init__(self, param=None, pos=None):
        self.param = param
        self.pos = pos

    def todict(self):
        return self.__dict__

    @staticmethod
    def fromdict(d):
        return ModelerParameter(d["param"], d["pos"])


class ModelerOutput():

    def __init__(self, description=""):
        self.description = description
        self.pos = None

    def todict(self):
        return self.__dict__


class Algorithm():

    def __init__(self, consoleName=""):

        self.name = None
        self.description = ""

        # The type of the algorithm, indicated as a string, which corresponds
        # to the string used to refer to it in the python console
        self.consoleName = consoleName

        self._algInstance = None

        # A dict of Input object. keys are param names
        self.params = {}

        # A dict of ModelerOutput with final output descriptions. Keys are output names.
        # Outputs not final are not stored in this dict
        self.outputs = {}

        self.pos = None

        self.dependencies = []

        self.paramsFolded = True
        self.outputsFolded = True
        self.active = True

    def todict(self):
        return {k: v for k, v in self.__dict__.iteritems() if not k.startswith("_")}

    @property
    def algorithm(self):
        if self._algInstance is None:
            self._algInstance = algList.getAlgorithm(self.consoleName).getCopy()
        return self._algInstance

    def setName(self, model):
        if self.name is None:
            i = 1
            name = self.consoleName + "_" + unicode(i)
            while name in model.algs:
                i += 1
                name = self.consoleName + "_" + unicode(i)
            self.name = name

    def getOutputType(self, outputName):
        output = self.algorithm.getOutputFromName(outputName)
        return "output " + output.__class__.__name__.split(".")[-1][6:].lower()

    def toPython(self):
        s = []
        params = []
        for param in self.algorithm.parameters:
            value = self.params[param.name]

            def _toString(v):
                if isinstance(v, (ValueFromInput, ValueFromOutput)):
                    return v.asPythonParameter()
                elif isinstance(v, basestring):
                    return "\\n".join(("'%s'" % v).splitlines())
                elif isinstance(v, list):
                    return "[%s]" % ",".join([_toString(val) for val in v])
                else:
                    return unicode(value)
            params.append(_toString(value))
        for out in self.algorithm.outputs:
            if out.name in self.outputs:
                params.append(safeName(self.outputs[out.name].description).lower())
            else:
                params.append(str(None))
        s.append("outputs_%s=processing.runalg('%s', %s)" % (self.name, self.consoleName, ",".join(params)))
        return s


class ValueFromInput():

    def __init__(self, name=""):
        self.name = name

    def todict(self):
        return self.__dict__

    def __str__(self):
        return self.name

    def __eq__(self, other):
        try:
            return self.name == other.name
        except:
            return False

    def asPythonParameter(self):
        return self.name


class ValueFromOutput():

    def __init__(self, alg="", output=""):
        self.alg = alg
        self.output = output

    def todict(self):
        return self.__dict__

    def __eq__(self, other):
        try:
            return self.alg == other.alg and self.output == other.output
        except:
            return False

    def __str__(self):
        return self.alg + "," + self.output

    def asPythonParameter(self):
        return "outputs_%s['%s']" % (self.alg, self.output)


class ModelerAlgorithm(GeoAlgorithm):

    CANVAS_SIZE = 4000

    def getCopy(self):
        newone = ModelerAlgorithm()
        newone.provider = self.provider
        newone.algs = copy.deepcopy(self.algs)
        newone.inputs = copy.deepcopy(self.inputs)
        newone.defineCharacteristics()
        newone.name = self.name
        newone.group = self.group
        newone.descriptionFile = self.descriptionFile
        newone.helpContent = copy.deepcopy(self.helpContent)
        return newone

    def __init__(self):
        self.name = self.tr('Model', 'ModelerAlgorithm')
        # The dialog where this model is being edited
        self.modelerdialog = None
        self.descriptionFile = None
        self.helpContent = {}

        # Geoalgorithms in this model. A dict of Algorithm objects, with names as keys
        self.algs = {}

        # Input parameters. A dict of Input objects, with names as keys
        self.inputs = {}
        GeoAlgorithm.__init__(self)

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'model.png'))

    def defineCharacteristics(self):
        classes = [ParameterRaster, ParameterVector, ParameterTable, ParameterTableField,
                   ParameterBoolean, ParameterString, ParameterNumber]
        self.parameters = []
        for c in classes:
            for inp in self.inputs.values():
                if isinstance(inp.param, c):
                    self.parameters.append(inp.param)
        for inp in self.inputs.values():
            if inp.param not in self.parameters:
                self.parameters.append(inp.param)
        self.outputs = []
        for alg in self.algs.values():
            if alg.active:
                for out in alg.outputs:
                    modelOutput = copy.deepcopy(alg.algorithm.getOutputFromName(out))
                    modelOutput.name = self.getSafeNameForOutput(alg.name, out)
                    modelOutput.description = alg.outputs[out].description
                    self.outputs.append(modelOutput)
        self.outputs.sort(key=attrgetter("description"))

    def addParameter(self, param):
        self.inputs[param.param.name] = param

    def updateParameter(self, param):
        self.inputs[param.name].param = param

    def addAlgorithm(self, alg):
        name = self.getNameForAlgorithm(alg)
        alg.name = name
        self.algs[name] = alg

    def getNameForAlgorithm(self, alg):
        i = 1
        while alg.consoleName.upper().replace(":", "") + "_" + unicode(i) in self.algs.keys():
            i += 1
        return alg.consoleName.upper().replace(":", "") + "_" + unicode(i)

    def updateAlgorithm(self, alg):
        alg.pos = self.algs[alg.name].pos
        alg.paramsFolded = self.algs[alg.name].paramsFolded
        alg.outputsFolded = self.algs[alg.name].outputsFolded
        self.algs[alg.name] = alg

        from processing.modeler.ModelerGraphicItem import ModelerGraphicItem
        for i, out in enumerate(alg.outputs):
            alg.outputs[out].pos = (alg.outputs[out].pos or
                                    alg.pos + QPointF(
                ModelerGraphicItem.BOX_WIDTH,
                (i + 1.5) * ModelerGraphicItem.BOX_HEIGHT))

    def removeAlgorithm(self, name):
        """Returns True if the algorithm could be removed, False if
        others depend on it and could not be removed.
        """
        if self.hasDependencies(name):
            return False
        del self.algs[name]
        self.modelerdialog.hasChanged = True
        return True

    def removeParameter(self, name):
        """Returns True if the parameter could be removed, False if
        others depend on it and could not be removed.
        """
        if self.hasDependencies(name):
            return False
        del self.inputs[name]
        self.modelerdialog.hasChanged = True
        return True

    def hasDependencies(self, name):
        """This method returns True if some other element depends on
        the passed one.
        """
        for alg in self.algs.values():
            for value in alg.params.values():
                if value is None:
                    continue
                if isinstance(value, list):
                    for v in value:
                        if isinstance(v, ValueFromInput):
                            if v.name == name:
                                return True
                        elif isinstance(v, ValueFromOutput):
                            if v.alg == name:
                                return True
                if isinstance(value, ValueFromInput):
                    if value.name == name:
                        return True
                elif isinstance(value, ValueFromOutput):
                    if value.alg == name:
                        return True
        return False

    def getDependsOnAlgorithms(self, name):
        """This method returns a list with names of algorithms
        a given one depends on.
        """
        alg = self.algs[name]
        algs = set()
        algs.update(set(alg.dependencies))
        for value in alg.params.values():
            if value is None:
                continue
            if isinstance(value, list):
                for v in value:
                    if isinstance(v, ValueFromOutput):
                        algs.add(v.alg)
                        algs.update(self.getDependsOnAlgorithms(v.alg))
            elif isinstance(value, ValueFromOutput):
                algs.add(value.alg)
                algs.update(self.getDependsOnAlgorithms(value.alg))

        return algs

    def getDependentAlgorithms(self, name):
        """This method returns a list with the names of algorithms
        depending on a given one. It includes the algorithm itself
        """
        algs = set()
        algs.add(name)
        for alg in self.algs.values():
            for value in alg.params.values():
                if value is None:
                    continue
                if isinstance(value, list):
                    for v in value:
                        if isinstance(v, ValueFromOutput) and v.alg == name:
                            algs.update(self.getDependentAlgorithms(alg.name))
                elif isinstance(value, ValueFromOutput) and value.alg == name:
                    algs.update(self.getDependentAlgorithms(alg.name))

        return algs

    def setPositions(self, paramPos, algPos, outputsPos):
        for param, pos in paramPos.iteritems():
            self.inputs[param].pos = pos
        for alg, pos in algPos.iteritems():
            self.algs[alg].pos = pos
        for alg, positions in outputsPos.iteritems():
            for output, pos in positions.iteritems():
                self.algs[alg].outputs[output].pos = pos

    def prepareAlgorithm(self, alg):
        algInstance = alg.algorithm
        for param in algInstance.parameters:
            if not param.hidden:
                if param.name in alg.params:
                    value = self.resolveValue(alg.params[param.name])
                else:
                    if iface is not None:
                        iface.messageBar().pushMessage(self.tr("Warning"),
                                                       self.tr("Parameter %s in algorithm %s in the model is run with default value! Edit the model to make sure that this is correct.") % (param.name, alg.name),
                                                       QgsMessageBar.WARNING, 4)
                    value = param.default
                if value is None and isinstance(param, ParameterExtent):
                    value = self.getMinCoveringExtent()
                # We allow unexistent filepaths, since that allows
                # algorithms to skip some conversion routines
                if not param.setValue(value) and not isinstance(param,
                                                                ParameterDataObject):
                    raise GeoAlgorithmExecutionException(
                        self.tr('Wrong value %s for %s %s', 'ModelerAlgorithm')
                        % (value, param.__class__.__name__, param.name))

        for out in algInstance.outputs:
            if not out.hidden:
                if out.name in alg.outputs:
                    name = self.getSafeNameForOutput(alg.name, out.name)
                    modelOut = self.getOutputFromName(name)
                    if modelOut:
                        out.value = modelOut.value
                else:
                    out.value = None

        return algInstance

    def deactivateAlgorithm(self, algName):
        dependent = self.getDependentAlgorithms(algName)
        for alg in dependent:
            self.algs[alg].active = False

    def activateAlgorithm(self, algName):
        parents = self.getDependsOnAlgorithms(algName)
        for alg in parents:
            if not self.algs[alg].active:
                return False
        self.algs[algName].active = True
        return True

    def getSafeNameForOutput(self, algName, outName):
        return outName + '_ALG' + algName

    def resolveValue(self, value):
        if value is None:
            return None
        if isinstance(value, list):
            return ";".join([self.resolveValue(v) for v in value])
        if isinstance(value, ValueFromInput):
            return self.getParameterFromName(value.name).value
        elif isinstance(value, ValueFromOutput):
            return self.algs[value.alg].algorithm.getOutputFromName(value.output).value
        else:
            return value

    def getMinCoveringExtent(self):
        first = True
        found = False
        for param in self.parameters:
            if param.value:
                if isinstance(param, (ParameterRaster, ParameterVector)):
                    found = True
                    if isinstance(param.value, (QgsRasterLayer, QgsVectorLayer)):
                        layer = param.value
                    else:
                        layer = dataobjects.getObjectFromUri(param.value)
                    self.addToRegion(layer, first)
                    first = False
                elif isinstance(param, ParameterMultipleInput):
                    found = True
                    layers = param.value.split(';')
                    for layername in layers:
                        layer = dataobjects.getObjectFromUri(layername)
                        self.addToRegion(layer, first)
                        first = False
        if found:
            return ','.join([unicode(v) for v in [self.xmin, self.xmax, self.ymin, self.ymax]])
        else:
            return None

    def addToRegion(self, layer, first):
        if first:
            self.xmin = layer.extent().xMinimum()
            self.xmax = layer.extent().xMaximum()
            self.ymin = layer.extent().yMinimum()
            self.ymax = layer.extent().yMaximum()
        else:
            self.xmin = min(self.xmin, layer.extent().xMinimum())
            self.xmax = max(self.xmax, layer.extent().xMaximum())
            self.ymin = min(self.ymin, layer.extent().yMinimum())
            self.ymax = max(self.ymax, layer.extent().yMaximum())

    def processAlgorithm(self, progress):
        executed = []
        toExecute = [alg for alg in self.algs.values() if alg.active]
        while len(executed) < len(toExecute):
            for alg in toExecute:
                if alg.name not in executed:
                    canExecute = True
                    required = self.getDependsOnAlgorithms(alg.name)
                    for requiredAlg in required:
                        if requiredAlg != alg.name and requiredAlg not in executed:
                            canExecute = False
                            break
                    if canExecute:
                        try:
                            progress.setDebugInfo(
                                self.tr('Prepare algorithm: %s', 'ModelerAlgorithm') % alg.name)
                            self.prepareAlgorithm(alg)
                            progress.setText(
                                self.tr('Running %s [%i/%i]', 'ModelerAlgorithm') % (alg.description, len(executed) + 1, len(toExecute)))
                            progress.setDebugInfo('Parameters: ' + ', '.join([unicode(p).strip()
                                                                              + '=' + unicode(p.value) for p in alg.algorithm.parameters]))
                            t0 = time.time()
                            alg.algorithm.execute(progress, self)
                            dt = time.time() - t0
                            executed.append(alg.name)
                            progress.setDebugInfo(
                                self.tr('OK. Execution took %0.3f ms (%i outputs).', 'ModelerAlgorithm') % (dt, len(alg.algorithm.outputs)))
                        except GeoAlgorithmExecutionException as e:
                            progress.setDebugInfo(self.tr('Failed', 'ModelerAlgorithm'))
                            raise GeoAlgorithmExecutionException(
                                self.tr('Error executing algorithm %s\n%s', 'ModelerAlgorithm') % (alg.description, e.msg))

        progress.setDebugInfo(
            self.tr('Model processed ok. Executed %i algorithms total', 'ModelerAlgorithm') % len(executed))

    def getAsCommand(self):
        if self.descriptionFile:
            return GeoAlgorithm.getAsCommand(self)
        else:
            return None

    def commandLineName(self):
        if self.descriptionFile is None:
            return ''
        else:
            return 'modeler:' + os.path.basename(self.descriptionFile)[:-6].lower()

    def checkBeforeOpeningParametersDialog(self):
        for alg in self.algs.values():
            algInstance = algList.getAlgorithm(alg.consoleName)
            if algInstance is None:
                return "The model you are trying to run contains an algorithm that is not available: <i>%s</i>" % alg.consoleName

    def setModelerView(self, dialog):
        self.modelerdialog = dialog

    def updateModelerView(self):
        if self.modelerdialog:
            self.modelerdialog.repaintModel()

    def help(self):
        try:
            return True, getHtmlFromDescriptionsDict(self, self.helpContent)
        except:
            return False, None

    def shortHelp(self):
        if 'ALG_DESC' in self.helpContent:
            return self._formatHelp(unicode(self.helpContent['ALG_DESC']))
        return None

    def getParameterDescriptions(self):
        descs = {}
        descriptions = self.helpContent
        for param in self.parameters:
            if param.name in descriptions:
                descs[param.name] = unicode(descriptions[param.name])
        return descs

    def todict(self):
        keys = ["inputs", "group", "name", "algs", "helpContent"]
        return {k: v for k, v in self.__dict__.iteritems() if k in keys}

    def toJson(self):
        def todict(o):
            if isinstance(o, QPointF):
                return {"class": "point", "values": {"x": o.x(), "y": o.y()}}
            try:
                d = o.todict()
                return {"class": o.__class__.__module__ + "." + o.__class__.__name__, "values": d}
            except Exception:
                pass
        return json.dumps(self, default=todict, indent=4)

    @staticmethod
    def fromJson(s):
        def fromdict(d):
            try:
                fullClassName = d["class"]
                tokens = fullClassName.split(".")
                className = tokens[-1]
                moduleName = ".".join(tokens[:-1])
                values = d["values"]
                if className == "point":
                    return QPointF(values["x"], values["y"])

                def _import(name):
                    __import__(name)
                    return sys.modules[name]

                if moduleName.startswith("processing.parameters"):
                    moduleName = "processing.core.parameters"
                module = _import(moduleName)
                clazz = getattr(module, className)
                instance = clazz()
                for k, v in values.iteritems():
                    instance.__dict__[k] = v
                return instance
            except KeyError:
                return d
            except Exception as e:
                raise e
        try:
            model = json.loads(s, object_hook=fromdict)
        except Exception as e:
            raise WrongModelException(e.args[0])
        return model

    @staticmethod
    def fromJsonFile(filename):
        with open(filename) as f:
            s = f.read()
        alg = ModelerAlgorithm.fromJson(s)
        alg.descriptionFile = filename
        return alg

    ############LEGACY METHOD TO SUPPORT OLD FORMAT###########

    LINE_BREAK_STRING = '%%%'

    @staticmethod
    def fromFile(filename):
        try:
            alg = ModelerAlgorithm.fromJsonFile(filename)
            return alg
        except WrongModelException:
            alg = ModelerAlgorithm.fromOldFormatFile(filename)
            return alg

    @staticmethod
    def fromOldFormatFile(filename):
        def _tr(s):
            return QCoreApplication.translate('ModelerAlgorithm', s)
        hardcodedValues = {}
        modelParameters = []
        modelAlgs = []
        model = ModelerAlgorithm()
        model.descriptionFile = filename
        lines = codecs.open(filename, 'r', encoding='utf-8')
        line = lines.readline().strip('\n').strip('\r')
        try:
            while line != '':
                if line.startswith('PARAMETER:'):
                    paramLine = line[len('PARAMETER:'):]
                    param = getParameterFromString(paramLine)
                    if param:
                        pass
                    else:
                        raise WrongModelException(
                            _tr('Error in parameter line: %s', 'ModelerAlgorithm') % line)
                    line = lines.readline().strip('\n')
                    tokens = line.split(',')
                    model.addParameter(ModelerParameter(param,
                                                        QPointF(float(tokens[0]), float(tokens[1]))))
                    modelParameters.append(param.name)
                elif line.startswith('VALUE:'):
                    valueLine = line[len('VALUE:'):]
                    tokens = valueLine.split('===')
                    name = tokens[0]
                    value = tokens[1].replace(ModelerAlgorithm.LINE_BREAK_STRING, '\n')
                    hardcodedValues[name] = value
                elif line.startswith('NAME:'):
                    model.name = line[len('NAME:'):]
                elif line.startswith('GROUP:'):
                    model.group = line[len('GROUP:'):]
                elif line.startswith('ALGORITHM:'):
                    algLine = line[len('ALGORITHM:'):]
                    alg = algList.getAlgorithm(algLine)
                    if alg is not None:
                        modelAlg = Algorithm(alg.commandLineName())
                        modelAlg.description = alg.name
                        posline = lines.readline().strip('\n').strip('\r')
                        tokens = posline.split(',')
                        modelAlg.pos = QPointF(float(tokens[0]), float(tokens[1]))
                        # dependenceline = lines.readline().strip('\n').strip('\r')
                        for param in alg.parameters:
                            if not param.hidden:
                                line = lines.readline().strip('\n').strip('\r')
                                if line == unicode(None):
                                    modelAlg.params[param.name] = None
                                else:
                                    tokens = line.split('|')
                                    try:
                                        algIdx = int(tokens[0])
                                    except:
                                        raise WrongModelException(
                                            _tr('Number of parameters in the '
                                                '{} algorithm does not match '
                                                'current Processing '
                                                'implementation'.format(alg.name)))
                                    if algIdx == -1:
                                        if tokens[1] in modelParameters:
                                            modelAlg.params[param.name] = ValueFromInput(tokens[1])
                                        else:
                                            modelAlg.params[param.name] = hardcodedValues[tokens[1]]
                                    else:
                                        modelAlg.params[param.name] = ValueFromOutput(algIdx, tokens[1])

                        for out in alg.outputs:
                            if not out.hidden:
                                line = lines.readline().strip('\n').strip('\r')
                                if unicode(None) != line:
                                    if '|' in line:
                                        tokens = line.split('|')
                                        name = tokens[0]
                                        tokens = tokens[1].split(',')
                                        pos = QPointF(float(tokens[0]), float(tokens[1]))
                                    else:
                                        name = line
                                        pos = None
                                    modelerOutput = ModelerOutput(name)
                                    modelerOutput.pos = pos
                                    modelAlg.outputs[out.name] = modelerOutput

                        model.addAlgorithm(modelAlg)
                        modelAlgs.append(modelAlg.name)
                    else:
                        raise WrongModelException(
                            _tr('Error in algorithm name: %s',) % algLine)
                line = lines.readline().strip('\n').strip('\r')
            for modelAlg in model.algs.values():
                for name, value in modelAlg.params.iteritems():
                    if isinstance(value, ValueFromOutput):
                        value.alg = modelAlgs[value.alg]
            return model
        except Exception as e:
            if isinstance(e, WrongModelException):
                raise e
            else:
                raise WrongModelException(_tr('Error in model definition line: ') + '%s\n%s' % (line.strip(), traceback.format_exc()))

    def toPython(self):
        s = ['##%s=name' % self.name]
        for param in self.inputs.values():
            s.append(param.param.getAsScriptCode())
        for alg in self.algs.values():
            for name, out in alg.outputs.iteritems():
                s.append('##%s=%s' % (safeName(out.description).lower(), alg.getOutputType(name)))

        executed = []
        toExecute = [alg for alg in self.algs.values() if alg.active]
        while len(executed) < len(toExecute):
            for alg in toExecute:
                if alg.name not in executed:
                    canExecute = True
                    required = self.getDependsOnAlgorithms(alg.name)
                    for requiredAlg in required:
                        if requiredAlg != alg.name and requiredAlg not in executed:
                            canExecute = False
                            break
                    if canExecute:
                        s.extend(alg.toPython())
                        executed.append(alg.name)

        return '\n'.join(s)


def safeName(name):
    validChars = 'abcdefghijklmnopqrstuvwxyz'
    return ''.join(c for c in name.lower() if c in validChars)
