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

import traceback
import copy
import os.path
import codecs
import time
from qgis.core import *
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.parameters.ParameterFactory import ParameterFactory
from processing.modeler.WrongModelException import WrongModelException
from processing.modeler.ModelerUtils import ModelerUtils
from processing.parameters.ParameterRaster import ParameterRaster
from processing.core.QGisLayers import QGisLayers
from processing.parameters.ParameterExtent import ParameterExtent
from PyQt4 import QtCore, QtGui
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.parameters.ParameterMultipleInput import ParameterMultipleInput
from processing.outputs.OutputRaster import OutputRaster
from processing.outputs.OutputHTML import OutputHTML
from processing.outputs.OutputTable import OutputTable
from processing.outputs.OutputVector import OutputVector
from processing.outputs.OutputNumber import OutputNumber
from processing.outputs.OutputString import OutputString
from processing.parameters.Parameter import Parameter
from processing.parameters.ParameterVector import ParameterVector
from processing.parameters.ParameterTableField import ParameterTableField
from processing.gui.Help2Html import Help2Html

class ModelerAlgorithm(GeoAlgorithm):

    CANVAS_SIZE = 4000
    LINE_BREAK_STRING="%%%"

    def getCopy(self):
        newone = ModelerAlgorithm()
        newone.openModel(self.descriptionFile)
        newone.provider = self.provider
        newone.deactivated = self.deactivated
        return newone

    def __init__(self):
        GeoAlgorithm.__init__(self)
        #the dialog where this model is being edited
        self.modelerdialog = None
        self.descriptionFile = None

        #Geoalgorithms in this model
        self.algs = []

        #parameters of Geoalgorithms in self.algs.
        #Each entry is a map with (paramname, paramvalue) values for algs[i].
        #paramvalues are instances of AlgorithmAndParameter
        self.algParameters = []

        #algorithms that each algorithm depends on.
        #This is just a list of dependencies not set by outputs and inputs
        #but explicitly entered instead, meaning that an algorithm must ''wait''
        #for another to finish
        #Each entry is a list with algorithm indexes
        self.dependencies = []

        #outputs of Geoalgorithms in self.algs.
        #Each entry is a map with (output, outputvalue) values for algs[i].
        #outputvalue is the name of the output if final. None if is an intermediate output
        self.algOutputs = []

        #Hardcoded parameter values entered by the user when defining the model. Keys are value names.
        self.paramValues = {}

        #position of items in canvas
        self.algPos = []
        self.paramPos = []
        self.outputPos = [] #same structure as algOutputs

        #deactivated algorithms that should not be executed
        self.deactivated = []


    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/model.png")


    def openModel(self, filename):
        self.algPos = []
        self.paramPos = []
        self.outputOutputs = []
        self.algs = []
        self.algParameters = []
        self.algOutputs = []
        self.paramValues = {}
        self.dependencies = []

        self.descriptionFile = filename
        lines = codecs.open(filename, "r", encoding='utf-8')
        line = lines.readline().strip("\n").strip("\r")
        iAlg = 0
        try:
            while line != "":
                if line.startswith("PARAMETER:"):
                    paramLine = line[len("PARAMETER:"):]
                    param = ParameterFactory.getFromString(paramLine)
                    if param:
                        self.parameters.append(param)
                    else:
                        raise WrongModelException("Error in parameter line: " + line)
                    line = lines.readline().strip("\n")
                    tokens = line.split(",")
                    self.paramPos.append(QtCore.QPointF(float(tokens[0]), float(tokens[1])))
                elif line.startswith("VALUE:"):
                    valueLine = line[len("VALUE:"):]
                    tokens = valueLine.split("===")
                    self.paramValues[tokens[0]] = tokens[1].replace(ModelerAlgorithm.LINE_BREAK_STRING, '\n')
                elif line.startswith("NAME:"):
                    self.name = line[len("NAME:"):]
                elif line.startswith("GROUP:"):
                    self.group = line[len("GROUP:"):]
                elif line.startswith("ALGORITHM:"):
                    algParams={}
                    algOutputs={}
                    algLine = line[len("ALGORITHM:"):]
                    alg = ModelerUtils.getAlgorithm(algLine)
                    if alg is not None:
                        posline = lines.readline().strip("\n").strip("\r")
                        tokens = posline.split(",")
                        self.algPos.append(QtCore.QPointF(float(tokens[0]), float(tokens[1])))
                        self.algs.append(alg)
                        dependenceline = lines.readline().strip("\n").strip("\r")
                        dependencies = [];
                        if dependenceline != str(None):
                            for index in dependenceline.split(","):
                                try:
                                    dependencies.append(int(index))
                                except:
                                    pass #a quick fix while I figure out how to solve problems when parsing this
                        for param in alg.parameters:
                            line = lines.readline().strip("\n").strip("\r")
                            if line==str(None):
                                algParams[param.name] = None
                            else:
                                tokens = line.split("|")
                                algParams[param.name] = AlgorithmAndParameter(int(tokens[0]), tokens[1])
                        outputPos = {}
                        for out in alg.outputs:
                            line = lines.readline().strip("\n").strip("\r")
                            if str(None)!=line:
                                if "|" in line:
                                    tokens = line.split("|")
                                    name = tokens[0]
                                    tokens = tokens[1].split(",")
                                    outputPos[out.name] = QtCore.QPointF(float(tokens[0]), float(tokens[1]))
                                else:
                                    name = line
                                    outputPos[out.name] = None
                                algOutputs[out.name] = name
                                #we add the output to the algorithm, with a name indicating where it comes from
                                #that guarantees that the name is unique
                                output = copy.deepcopy(out)
                                output.description = line
                                output.name = self.getSafeNameForOutput(iAlg, output)
                                self.addOutput(output)
                            else:
                                algOutputs[out.name] = None
                        self.outputPos.append(outputPos)
                        self.algOutputs.append(algOutputs)
                        self.algParameters.append(algParams)
                        self.dependencies.append(dependencies)
                        iAlg += 1
                    else:
                        raise WrongModelException("Error in algorithm name: " + algLine)
                line = lines.readline().strip("\n").strip("\r")
        except Exception, e:
            if isinstance (e, WrongModelException):
                raise e
            else:
                raise WrongModelException("Error in model definition line:"  + line.strip() + " : " + traceback.format_exc())

    def addParameter(self, param):
        self.parameters.append(param)
        self.paramPos.append(self.getPositionForParameterItem())

    def updateParameter(self, paramIndex, param):
        self.parameters[paramIndex] = param
        #self.updateModelerView()

    def addAlgorithm(self, alg, parametersMap, valuesMap, outputsMap, dependencies):
        self.algs.append(alg)
        self.algParameters.append(parametersMap)
        self.algOutputs.append(outputsMap)
        self.dependencies.append(dependencies)
        for value in valuesMap.keys():
            self.paramValues[value] = valuesMap[value]
        algPos = self.getPositionForAlgorithmItem()
        self.algPos.append(algPos)
        pos = {}
        i = 0
        from processing.modeler.ModelerGraphicItem import ModelerGraphicItem
        for out in outputsMap:
            pos[out] = algPos + QtCore.QPointF(ModelerGraphicItem.BOX_WIDTH, i * ModelerGraphicItem.BOX_HEIGHT)
            i+=1
        self.outputPos.append(pos)

    def updateAlgorithm(self, algIndex, parametersMap, valuesMap, outputsMap, dependencies):
        self.algParameters[algIndex] = parametersMap
        self.algOutputs[algIndex] = outputsMap
        self.dependencies[algIndex] =  dependencies
        for value in valuesMap.keys():
            self.paramValues[value] = valuesMap[value]
        self.updateModelerView()
        algPos = self.algPos[algIndex]
        pos = {}
        i = 0
        from processing.modeler.ModelerGraphicItem import ModelerGraphicItem
        for out in outputsMap:
            pos[out] = algPos + QtCore.QPointF(ModelerGraphicItem.BOX_WIDTH, i * ModelerGraphicItem.BOX_HEIGHT)
            i+=1
        self.outputPos[algIndex] = pos


    def removeAlgorithm(self, index):
        '''returns true if the algorithm could be removed, false if others depend on it and could not be removed'''
        if self.hasDependencies(self.algs[index], index):
            return False
        for out in self.algs[index].outputs:
            val = self.algOutputs[index][out.name]
            if val:
                name = self.getSafeNameForOutput(index, out)
                self.removeOutputFromName(name)
        del self.algs[index]
        del self.algParameters[index]
        del self.algOutputs[index]
        del self.algPos[index]
        del self.outputPos[index]

        i = -1
        for paramValues in self.algParameters:
            i += 1
            newValues = {}
            for name, value in paramValues.iteritems():
                if value:
                    if value.alg > index:
                        newValues[name] = AlgorithmAndParameter(value.alg - 1, value.param, value.algName, value.paramName)
                    else:
                        newValues[name] = value
                else:
                    newValues[name] = value
            self.algParameters[i] = newValues
        self.updateModelerView()
        return True

    def removeParameter(self, index):
        '''returns true if the parameter could be removed, false if others depend on it and could not be removed'''
        if self.hasDependencies(self.parameters[index], index):
            return False
        del self.parameters[index]
        del self.paramPos[index]
        self.updateModelerView()
        return True

    def hasDependencies(self, element, elementIndex):
        '''This method returns true if some other element depends on the passed one'''
        if isinstance(element, Parameter):
            for alg in self.algParameters:
                for aap in alg.values():
                    if aap:
                        if aap.alg == AlgorithmAndParameter.PARENT_MODEL_ALGORITHM:
                            if aap.param == element.name:
                                return True
                            elif aap.param in self.paramValues: #check for multiple inputs
                                aap2 = self.paramValues[aap.param]
                                if element.name in aap2:
                                    return True
            if isinstance(element, ParameterVector):
                for param in self.parameters:
                    if isinstance(param, ParameterTableField):
                        if param.parent == element.name:
                            return True
        else:
            for alg in self.algParameters:
                for aap in alg.values():
                    if aap:
                        if aap.alg == elementIndex:
                            return True

        return False

    def deactivateAlgorithm(self, algIndex, update = False):
        if algIndex not in self.deactivated:
            dependent = self.getDependentAlgorithms(algIndex)
            self.deactivated.extend(dependent)
        if update:
            self.updateModelerView()

    def activateAlgorithm(self, algIndex, update = False):
        if algIndex in self.deactivated:
            dependsOn = self.getDependsOnAlgorithms(algIndex)
            for alg in dependsOn:
                if alg in self.deactivated and alg != algIndex:
                    return False
            self.deactivated.remove(algIndex)
            dependent = self.getDependentAlgorithms(algIndex)
            for alg in dependent:
                if alg in self.deactivated:
                    self.deactivated.remove(alg)
        if update:
            self.updateModelerView()
        return True

    def getDependsOnAlgorithms(self, algIndex):
        '''This method returns a list with the indexes of algorithms a given one depends on'''
        algs = []
        algs.extend(self.dependencies[algIndex])
        index = -1
        for aap in self.algParameters[algIndex].values():
            index += 1
            if aap is not None:
                if aap.alg != AlgorithmAndParameter.PARENT_MODEL_ALGORITHM and aap.alg not in algs:
                    algs.append(aap.alg)
                    dep = self.getDependsOnAlgorithms(aap.alg)
                    for alg in dep:
                        if alg not in algs:
                            algs.append(alg)
        return algs

    def getDependentAlgorithms(self, algIndex):
        '''This method returns a list with the indexes of algorithms depending on a given one'''
        dependent = [algIndex]
        index = -1
        for alg in self.algParameters:
            index += 1
            if index in dependent:
                continue
            for aap in alg.values():
                if aap is not None:
                    if aap.alg == algIndex:
                        dep = self.getDependentAlgorithms(index)
                        for alg in dep:
                            if alg not in dependent:
                                dependent.append(alg)
                        break
        index = -1
        for dep in self.dependencies:
            index += 1
            if algIndex in dep:
                dep = self.getDependentAlgorithms(index)
                for alg in dep:
                    if alg not in dependent:
                        dependent.append(alg)

        return dependent

    def getPositionForAlgorithmItem(self):
        MARGIN = 20
        BOX_WIDTH = 200
        BOX_HEIGHT = 80
        if len(self.algPos) != 0:
            maxX = max([pos.x() for pos in self.algPos])
            maxY = max([pos.y() for pos in self.algPos])
            newX = min(MARGIN + BOX_WIDTH + maxX, self.CANVAS_SIZE - BOX_WIDTH)
            newY = min(MARGIN + BOX_HEIGHT + maxY, self.CANVAS_SIZE - BOX_HEIGHT)
        else:
            newX = MARGIN + BOX_WIDTH / 2
            newY = MARGIN * 2 + BOX_HEIGHT + BOX_HEIGHT / 2
        return QtCore.QPointF(newX, newY)

    def getPositionForParameterItem(self):
        MARGIN = 20
        BOX_WIDTH = 200
        BOX_HEIGHT = 80
        if len(self.paramPos) != 0:
            maxX = max([pos.x() for pos in self.paramPos])
            newX = min(MARGIN + BOX_WIDTH + maxX, self.CANVAS_SIZE - BOX_WIDTH)
        else:
            newX = MARGIN + BOX_WIDTH / 2
        return QtCore.QPointF(newX, MARGIN + BOX_HEIGHT / 2)

    def serialize(self):
        s="NAME:" + unicode(self.name) + "\n"
        s +="GROUP:" + unicode(self.group) + "\n"

        i = 0
        for param in self.parameters:
            s += "PARAMETER:" + param.serialize() + "\n"
            pt = self.paramPos[i]
            s +=  str(pt.x()) + "," + str(pt.y()) + "\n"
            i+=1
        for key in self.paramValues.keys():
            s += "VALUE:" + key + "===" + str(self.paramValues[key]).replace('\n', ModelerAlgorithm.LINE_BREAK_STRING) + "\n"
        for i in range(len(self.algs)):
            alg = self.algs[i]
            s+="ALGORITHM:" + alg.commandLineName()+"\n"
            pt = self.algPos[i]
            s +=  str(pt.x()) + "," + str(pt.y()) + "\n"
            if len(self.dependencies[i]) != 0:
                s += ",".join([str(index) for index in self.dependencies[i]]) + "\n"
            else:
                s += str(None) + "\n"
            for param in alg.parameters:
                value = self.algParameters[i][param.name]
                if value:
                    s+=value.serialize() + "\n"
                else:
                    s+=str(None) + "\n"
            for out in alg.outputs:
                value = self.algOutputs[i][out.name]
                s+=unicode(value)
                if value is not None:
                    pt = self.outputPos[i][out.name]
                    s +=  "|" + str(pt.x()) + "," + str(pt.y())
                s += "\n"

        return s


    def setPositions(self, paramPos, algPos, outputPos):
        self.paramPos = paramPos
        self.algPos = algPos
        self.outputPos = outputPos


    def prepareAlgorithm(self, alg, iAlg):
        for param in alg.parameters:
            aap = self.algParameters[iAlg][param.name]
            if aap == None:
                if isinstance(param, ParameterExtent):
                    value = self.getMinCoveringExtent()
                    if not param.setValue(value):
                        raise GeoAlgorithmExecutionException("Wrong value: " + str(value))
                else:
                    param.setValue(None)
                continue
            if isinstance(param, ParameterMultipleInput):
                value = self.getValueFromAlgorithmAndParameter(aap)
                tokens = value.split(";")
                layerslist = []
                for token in tokens:
                    i, paramname = token.split("|")
                    aap = AlgorithmAndParameter(int(i), paramname)
                    value = self.getValueFromAlgorithmAndParameter(aap)
                    layerslist.append(str(value))
                value = ";".join(layerslist)
                if not param.setValue(value):
                    raise GeoAlgorithmExecutionException("Wrong value: " + str(value))

            else:
                value = self.getValueFromAlgorithmAndParameter(aap)
                if not param.setValue(value):
                    raise GeoAlgorithmExecutionException("Wrong value: " + str(value))
        for out in alg.outputs:
            val = self.algOutputs[iAlg][out.name]
            if val:
                name = self.getSafeNameForOutput(iAlg, out)
                out.value = self.getOutputFromName(name).value
            else:
                out.value = None


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
                        layer = QGisLayers.getObjectFromUri(param.value)
                    self.addToRegion(layer, first)
                    first = False
                elif isinstance(param, ParameterMultipleInput):
                    found = True
                    layers = param.value.split(";")
                    for layername in layers:
                        layer = QGisLayers.getObjectFromUri(layername)
                        self.addToRegion(layer, first)
                        first = False
        if found:
            return str(self.xmin) + "," + str(self.xmax) + "," + str(self.ymin) + "," + str(self.ymax)
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


    def getSafeNameForOutput(self, ialg, out):
        return out.name +"_ALG" + str(ialg)


    def getValueFromAlgorithmAndParameter(self, aap):
        if aap is None:
            return None
        if float(aap.alg) == float(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM):
                for key in self.paramValues.keys():
                    if aap.param == key:
                        return self.paramValues[key]
                for param in self.parameters:
                    if aap.param == param.name:
                        return param.value
        else:
            return self.producedOutputs[int(aap.alg)][aap.param]

    def processAlgorithm(self, progress):
        self.producedOutputs = {}
        executed = []
        while len(executed) < len(self.algs) - len(self.deactivated):
            iAlg = 0
            for alg in self.algs:
                if iAlg not in self.deactivated and iAlg not in executed:
                    canExecute = True
                    required = self.getDependsOnAlgorithms(iAlg)
                    for requiredAlg in required:
                        if requiredAlg != iAlg and requiredAlg not in executed:
                            canExecute = False
                            break
                    if canExecute:
                        try:
                            alg = alg.getCopy()
                            progress.setDebugInfo("Prepare algorithm %i: %s" % (iAlg, alg.name))
                            self.prepareAlgorithm(alg, iAlg)
                            progress.setText("Running " + alg.name + " [" + str(iAlg+1) + "/"
                                             + str(len(self.algs) - len(self.deactivated)) +"]")
                            outputs = {}
                            progress.setDebugInfo("Parameters: " +
                                ', '.join([unicode(p).strip() + "=" + unicode(p.value) for p in alg.parameters]))
                            t0 = time.time()
                            alg.execute(progress)
                            dt = time.time() - t0
                            for out in alg.outputs:
                                outputs[out.name] = out.value
                            progress.setDebugInfo("Outputs: " +
                                ', '.join([unicode(out).strip() + "=" + unicode(outputs[out.name]) for out in alg.outputs]))
                            self.producedOutputs[iAlg] = outputs
                            executed.append(iAlg)
                            progress.setDebugInfo("OK. Execution took %0.3f ms (%i outputs)." % (dt, len(outputs)))
                        except GeoAlgorithmExecutionException, e :
                            progress.setDebugInfo("Failed")
                            raise GeoAlgorithmExecutionException("Error executing algorithm " + str(iAlg) + "\n" + e.msg)
                else:
                    pass
                    #progress.setDebugInfo("Algorithm %s deactivated (or already executed)" % alg.name)
                iAlg += 1
        progress.setDebugInfo("Model processed ok. Executed %i algorithms total" % iAlg)

    def getOutputType(self, i, outname):
        for out in self.algs[i].outputs:
            if out.name == outname:
                if isinstance(out, OutputRaster):
                    return "output raster"
                elif isinstance(out, OutputVector):
                    return "output vector"
                elif isinstance(out, OutputTable):
                    return "output table"
                elif isinstance(out, OutputHTML):
                    return "output html"
                elif isinstance(out, OutputNumber):
                    return "output number"
                elif isinstance(out, OutputString):
                    return "output string"


    def getAsPythonCode(self):
        s = []
        for param in self.parameters:
            s.append(str(param.getAsScriptCode().lower()))
        i = 0
        for outs in self.algOutputs:
            for out in outs.keys():
                if outs[out]:
                    s.append("##" + out.lower() + "_alg" + str(i) +"=" + self.getOutputType(i, out))
            i += 1
        i = 0
        iMultiple = 0
        for alg in self.algs:
            multiple= []
            runline = "outputs_" + str(i) + "=Processing.runalg(\"" + alg.commandLineName() + "\""
            for param in alg.parameters:
                aap = self.algParameters[i][param.name]
                if aap == None:
                    runline += ", None"
                elif isinstance(param, ParameterMultipleInput):
                    value = self.paramValues[aap.param]
                    tokens = value.split(";")
                    layerslist = []
                    for token in tokens:
                        iAlg, paramname = token.split("|")
                        if float(iAlg) == float(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM):
                            if self.ismodelparam(paramname):
                                value = paramname.lower()
                            else:
                                value = self.paramValues[paramname]
                        else:
                            value = "outputs_" + str(iAlg) + "['" + paramname +"']"
                        layerslist.append(str(value))

                    multiple.append("multiple_" + str(iMultiple) +"=[" + ",".join(layerslist) + "]")
                    runline +=", \";\".join(multiple_" + str(iMultiple) + ") "
                else:
                    if float(aap.alg) == float(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM):
                        if self.ismodelparam(aap.param):
                            runline += ", " + aap.param.lower()
                        else:
                            runline += ", " + str(self.paramValues[aap.param])
                    else:
                        runline += ", outputs_" + str(aap.alg) + "['" + aap.param +"']"
            for out in alg.outputs:
                value = self.algOutputs[i][out.name]
                if value:
                    name = out.name.lower() + "_alg" + str(i)
                else:
                    name = str(None)
                runline += ", " + name
            i += 1
            s += multiple
            s.append(str(runline + ")"))
        return "\n".join(s)

    def ismodelparam(self, paramname):
        for modelparam in self.parameters:
            if modelparam.name == paramname:
                return True
        return False


    def getAsCommand(self):
        if self.descriptionFile:
            return GeoAlgorithm.getAsCommand(self)
        else:
            return None

    def commandLineName(self):
        return "modeler:" + os.path.basename(self.descriptionFile)[:-6].lower()

    def setModelerView(self, dialog):
        self.modelerdialog = dialog

    def updateModelerView(self):
        if self.modelerdialog:
            self.modelerdialog.repaintModel()

    def helpFile(self):
        helpfile = self.descriptionFile + ".help"
        if os.path.exists(helpfile):
            h2h = Help2Html()
            return h2h.getHtmlFile(self, helpfile)
        else:
            return None

class AlgorithmAndParameter():

    PARENT_MODEL_ALGORITHM = -1

    #alg is the index of the algorithm in the list in ModelerAlgorithm.algs
    #-1 if the value is not taken from the output of an algorithm, but from an input of the model
    #or a hardcoded value
    #names are just used for decoration, and are not needed to create a hardcoded value
    def __init__(self, alg, param, algName="", paramName=""):
        self.alg = alg
        self.param = param
        self.algName = algName
        self.paramName = paramName

    def serialize(self):
        return str(self.alg) + "|" + str(self.param)

    def name(self):
        if self.alg != AlgorithmAndParameter.PARENT_MODEL_ALGORITHM:
            return self.paramName + " from algorithm " + str(self.alg) + "(" + self.algName + ")"
        else:
            return self.paramName

    def __str__(self):
        return str(self.alg) + "|" + str(self.param)
