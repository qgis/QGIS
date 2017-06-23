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
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterField,
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

    def __init__(self):
        super().__init__()

        # Geoalgorithms in this model. A dict of Algorithm objects, with names as keys
        self.algs = {}

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

    def toPython(self):
        s = ['##%s=name' % self.name()]
        for param in list(self.parameterComponents().values()):
            s.append(param.param.asScriptCode())
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
