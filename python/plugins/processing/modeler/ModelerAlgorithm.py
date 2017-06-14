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
from builtins import str
from builtins import object

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
from qgis.PyQt.QtCore import QPointF
from operator import attrgetter

from qgis.core import (QgsApplication,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterMapLayer,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterPoint,
                       QgsProcessingParameterFile,
                       QgsProcessingParameterMatrix,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterRange,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterString,
                       QgsProcessingParameterExpression,
                       QgsProcessingParameterTable,
                       QgsProcessingParameterTableField,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingModelAlgorithm)
from qgis.gui import QgsMessageBar
from qgis.utils import iface
from processing.modeler.WrongModelException import WrongModelException
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

from processing.gui.Help2Html import getHtmlFromDescriptionsDict

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class Algorithm(QgsProcessingModelAlgorithm.ChildAlgorithm):

    def __init__(self, consoleName=None):
        super().__init__(consoleName)

    def todict(self):
        return {k: v for k, v in list(self.__dict__.items()) if not k.startswith("_")}

    def getOutputType(self, outputName):
        output = self.algorithm().outputDefinition(outputName)
        return "output " + output.__class__.__name__.split(".")[-1][6:].lower()

    def toPython(self):
        s = []
        params = []
        if not self.algorithm():
            return None

        for param in self.algorithm().parameterDefinitions():
            value = self.parameterSources()[param.name()]

            def _toString(v):
                if isinstance(v, (ValueFromInput, ValueFromOutput)):
                    return v.asPythonParameter()
                elif isinstance(v, str):
                    return "\\n".join(("'%s'" % v).splitlines())
                elif isinstance(v, list):
                    return "[%s]" % ",".join([_toString(val) for val in v])
                else:
                    return str(value)
            params.append(_toString(value))
        for out in self.algorithm().outputs:
            if not out.flags() & QgsProcessingParameterDefinition.FlagHidden:
                if out.name() in self.outputs:
                    params.append(safeName(self.outputs[out.name()].description()).lower())
                else:
                    params.append(str(None))
        s.append("outputs_%s=processing.run('%s', %s)" % (self.childId(), self.algorithmId(), ",".join(params)))
        return s


class ValueFromInput(object):

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


class ValueFromOutput(object):

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
        return self.alg + ":" + self.output

    def asPythonParameter(self):
        return "outputs_%s['%s']" % (self.alg, self.output)


class CompoundValue(object):

    def __init__(self, values=[], definition=""):
        self.values = values
        self.definition = definition

    def todict(self):
        return self.__dict__

    def __eq__(self, other):
        try:
            return self.values == other.values and self.definition == other.definition
        except:
            return False

    def __str__(self):
        return self.definition

    def asPythonParameter(self):
        return ""  # TODO


class ModelerAlgorithm(QgsProcessingModelAlgorithm):

    CANVAS_SIZE = 4000

    def __init__(self):
        super().__init__()

        self.descriptionFile = None
        self.helpContent = {}

        # Geoalgorithms in this model. A dict of Algorithm objects, with names as keys
        self.algs = {}

    def updateAlgorithm(self, alg):
        alg.setPosition(self.childAlgorithm(alg.childId()).position())
        alg.setParametersCollapsed(self.childAlgorithm(alg.childId()).parametersCollapsed())
        alg.setOutputsCollapsed(self.childAlgorithm(alg.childId()).outputsCollapsed())
        self.setChildAlgorithm(alg)

        from processing.modeler.ModelerGraphicItem import ModelerGraphicItem
        for i, out in enumerate(alg.modelOutputs().keys()):
            alg.modelOutput(out).setPosition(alg.modelOutput(out).position() or
                                             alg.position() + QPointF(
                ModelerGraphicItem.BOX_WIDTH,
                (i + 1.5) * ModelerGraphicItem.BOX_HEIGHT))

    def prepareAlgorithm(self, alg):
        algInstance = alg.algorithm()
        for param in algInstance.parameterDefinitions():
            if not param.flags() & QgsProcessingParameterDefinition.FlagHidden:
                if param.name() in alg.params:
                    value = self.resolveValue(alg.params[param.name()], param)
                else:
                    if iface is not None:
                        iface.messageBar().pushMessage(self.tr("Warning"),
                                                       self.tr("Parameter {0} in algorithm {1} in the model is run with default value! Edit the model to make sure that this is correct.").format(param.name(), alg.displayName()),
                                                       QgsMessageBar.WARNING, 4)
                    value = param.defaultValue()
                # We allow unexistent filepaths, since that allows
                # algorithms to skip some conversion routines

                # TODO
                #if not param.checkValueIsAcceptable(value) and not isinstance(param,
                #                                                              ParameterDataObject):
                #    raise GeoAlgorithmExecutionException(
                #        self.tr('Wrong value {0} for {1} {2}', 'ModelerAlgorithm').format(
                #            value, param.__class__.__name__, param.name()
                #        )
                #    )

        # note to self - these are parameters, not outputs
        for out in algInstance.outputDefinitions():
            if not out.flags() & QgsProcessingParameterDefinition.FlagHidden:
                if out.name() in alg.outputs:
                    name = self.getSafeNameForOutput(alg.childId(), out.name())
                    modelOut = self.getOutputFromName(name)
                    if modelOut:
                        out.value = modelOut.value
                else:
                    out.value = None

        return algInstance

    def deactivateAlgorithm(self, algName):
        dependent = self.dependentChildAlgorithms(algName)
        for alg in dependent:
            self.algs[alg].setActive(False)
        self.algs[algName].setActive(False)

    def activateAlgorithm(self, algName):
        parents = self.dependsOnChildAlgorithms(algName)
        for alg in parents:
            if not self.childAlgorithm(alg).isActive():
                return False
        self.childAlgorithm(algName).setActive(True)
        return True

    def getSafeNameForOutput(self, algName, outName):
        return outName + '_ALG' + algName

    def resolveValue(self, value, param):
        if value is None:
            v = None
        if isinstance(value, list):
            v = ";".join([self.resolveValue(v, param) for v in value])
        elif isinstance(value, CompoundValue):
            v = self.resolveValue(value.definition, param)
        elif isinstance(value, ValueFromInput):
            v = self.getParameterFromName(value.name).value
        elif isinstance(value, ValueFromOutput):
            v = self.algs[value.alg].algorithm().outputDefinition(value.output).value
        else:
            v = value
        return param.evaluateForModeler(v, self)

    def processAlgorithm(self, parameters, context, feedback):
        executed = []
        toExecute = [alg for alg in list(self.algs.values()) if alg.isActive()]
        while len(executed) < len(toExecute):
            for alg in toExecute:
                if alg.childId() not in executed:
                    canExecute = True
                    required = self.dependsOnChildAlgorithms(alg.childId())
                    for requiredAlg in required:
                        if requiredAlg != alg.childId() and requiredAlg not in executed:
                            canExecute = False
                            break
                    if canExecute:
                        try:
                            feedback.pushDebugInfo(
                                self.tr('Prepare algorithm: {0}', 'ModelerAlgorithm').format(alg.childId()))
                            self.prepareAlgorithm(alg)
                            feedback.setProgressText(
                                self.tr('Running {0} [{1}/{2}]', 'ModelerAlgorithm').format(alg.description, len(executed) + 1, len(toExecute)))
                            feedback.pushDebugInfo('Parameters: ' + ', '.join([str(p).strip() +
                                                                               '=' + str(p.value) for p in alg.algorithm.parameters]))
                            t0 = time.time()
                            alg.algorithm().execute(parameters, context, feedback)
                            dt = time.time() - t0

                            # copy algorithm output value(s) back to model in case the algorithm modified those
                            for out in alg.algorithm().outputs:
                                if not out.flags() & QgsProcessingParameterDefinition.FlagHidden:
                                    if out.name() in alg.modelOutputs():
                                        modelOut = self.getOutputFromName(self.getSafeNameForOutput(alg.childId(), out.name()))
                                        if modelOut:
                                            modelOut.value = out.value

                            executed.append(alg.childId())
                            feedback.pushDebugInfo(
                                self.tr('OK. Execution took %{0:.3f} ms ({1} outputs).', 'ModelerAlgorithm').format(dt, len(alg.algorithm.modelOutputs())))
                        except GeoAlgorithmExecutionException as e:
                            feedback.pushDebugInfo(self.tr('Failed', 'ModelerAlgorithm'))
                            raise GeoAlgorithmExecutionException(
                                self.tr('Error executing algorithm {0}\n{1}', 'ModelerAlgorithm').format(alg.description, e.msg))

        feedback.pushDebugInfo(
            self.tr('Model processed ok. Executed {0} algorithms total', 'ModelerAlgorithm').format(len(executed)))

    def asPythonCommand(self, parameters, context):
        if self.descriptionFile:
            return QgsProcessingAlgorithm.asPythonCommand(self, parameters, context)
        else:
            return None

    def helpUrl(self):
        try:
            return getHtmlFromDescriptionsDict(self, self.helpContent)
        except:
            return None

    def shortHelpString(self):
        if 'ALG_DESC' in self.helpContent:
            return str(self.helpContent['ALG_DESC'])
        return None

    def getParameterDescriptions(self):
        descs = {}
        descriptions = self.helpContent
        for param in self.parameters:
            if param.name in descriptions:
                descs[param.name] = str(descriptions[param.name])
        return descs

    def todict(self):
        keys = ["inputs", "_group", "_name", "algs", "helpContent"]
        return {k: v for k, v in list(self.__dict__.items()) if k in keys}

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
                if isinstance(fullClassName, str):
                    tokens = fullClassName.split(".")
                else:
                    tokens = fullClassName.__class__.__name__.split(".")
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
                for k, v in list(values.items()):
                    # upgrade old model files
                    if k == 'group':
                        k = '_group'
                    elif k == 'name':
                        instance.__dict__['_name'] = v
                        k = 'modeler_name'
                        if not issubclass(clazz, GeoAlgorithm):
                            instance.__dict__['name'] = v
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

        if hasattr(model, "modeler_name"):
            model._name = model.modeler_name
        return model

    @staticmethod
    def fromFile(filename):
        with open(filename) as f:
            s = f.read()
        alg = ModelerAlgorithm.fromJson(s)
        if alg:
            alg.descriptionFile = filename
        return alg

    def toPython(self):
        s = ['##%s=name' % self.name()]
        for param in list(self.parameterComponents().values()):
            s.append(param.param.getAsScriptCode())
        for alg in list(self.algs.values()):
            for name, out in list(alg.modelOutputs().items()):
                s.append('##%s=%s' % (safeName(out.description()).lower(), alg.getOutputType(name)))

        executed = []
        toExecute = [alg for alg in list(self.algs.values()) if alg.isActive()]
        while len(executed) < len(toExecute):
            for alg in toExecute:
                if alg.childId() not in executed:
                    canExecute = True
                    required = self.dependsOnChildAlgorithms(alg.childId())
                    for requiredAlg in required:
                        if requiredAlg != alg.childId() and requiredAlg not in executed:
                            canExecute = False
                            break
                    if canExecute:
                        s.extend(alg.toPython())
                        executed.append(alg.childId())

        return '\n'.join(s)


def safeName(name):
    validChars = 'abcdefghijklmnopqrstuvwxyz'
    return ''.join(c for c in name.lower() if c in validChars)
