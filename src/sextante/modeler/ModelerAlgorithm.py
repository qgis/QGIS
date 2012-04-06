from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.parameters.ParameterFactory import ParameterFactory
from sextante.modeler.WrongModelException import WrongModelException
from sextante.modeler.ModelerUtils import ModelerUtils
import copy
from PyQt4 import QtCore, QtGui
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
import os.path
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput
from sextante.outputs.OutputRaster import OutputRaster
from sextante.outputs.OutputHTML import OutputHTML
from sextante.outputs.OutputTable import OutputTable
from sextante.outputs.OutputVector import OutputVector
from sextante.outputs.OutputNumber import OutputNumber
from sextante.parameters.Parameter import Parameter
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterTableField import ParameterTableField

class ModelerAlgorithm(GeoAlgorithm):

    def __deepcopy__(self,memo):
        newone = ModelerAlgorithm()
        newone.openModel(self.descriptionFile)
        newone.provider = self.provider
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

        #outputs of Geoalgorithms in self.algs.
        #Each entry is a map with (output, outputvalue) values for algs[i].
        #outputvalue is the name of the output if final. None if is an intermediate output
        self.algOutputs = []

        #Hardcoded parameter values entered by the user when defining the model. Keys are value names.
        self.paramValues = {}

        #position of items in canvas
        self.algPos = []
        self.paramPos = []


    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/model.png")


    def openModel(self, filename):
        self.algPos = []
        self.paramPos = []
        self.algs = []
        self.algParameters = []
        self.algOutputs = []
        self.paramValues = {}

        self.descriptionFile = filename
        lines = open(filename)
        line = lines.readline().strip("\n")
        iAlg = 0
        try:
            while line != "":
                if line.startswith("PARAMETER:"):
                    paramLine = line[len("PARAMETER:"):]
                    param = ParameterFactory.getFromString(paramLine)
                    if param:
                        self.parameters.append(param)
                    else:
                        raise WrongModelException("Error in line: " + line)
                    line = lines.readline().strip("\n")
                    tokens = line.split(",")
                    self.paramPos.append(QtCore.QPointF(float(tokens[0]), float(tokens[1])))
                elif line.startswith("VALUE:"):
                    valueLine = line[len("VALUE:"):]
                    tokens = valueLine.split("=")
                    self.paramValues[tokens[0]] = tokens[1]
                elif line.startswith("NAME:"):
                    self.name = line[len("NAME:"):]
                elif line.startswith("GROUP:"):
                    self.group = line[len("GROUP:"):]
                elif line.startswith("ALGORITHM:"):
                    algParams={}
                    algOutputs={}
                    algLine = line[len("ALGORITHM:"):]
                    alg = ModelerUtils.getAlgorithm(algLine)
                    if alg:
                        posline = lines.readline().strip("\n")
                        tokens = posline.split(",")
                        self.algPos.append(QtCore.QPointF(float(tokens[0]), float(tokens[1])))
                        self.algs.append(alg)
                        for param in alg.parameters:
                            line = lines.readline().strip("\n")
                            if line==str(None):
                                algParams[param.name] = None
                            else:
                                tokens = line.split("|")
                                algParams[param.name] = AlgorithmAndParameter(int(tokens[0]), tokens[1])
                        for out in alg.outputs:
                            line = lines.readline().strip("\n")
                            if str(None)!=line:
                                algOutputs[out.name] = line
                                #we add the output to the algorithm, with a name indicating where it comes from
                                #that guarantees that the name is unique
                                output = copy.deepcopy(out)
                                output.description = line
                                output.name = self.getSafeNameForOutput(iAlg, output)
                                self.addOutput(output)
                            else:
                                algOutputs[out.name] = None
                        self.algOutputs.append(algOutputs)
                        self.algParameters.append(algParams)
                        iAlg += 1
                    else:
                        raise WrongModelException("Error in line: " + line)
                line = lines.readline().strip("\n")
        except WrongModelException:
            raise WrongModelException(line)

    def addParameter(self, param):
        self.parameters.append(param)
        self.paramPos.append(self.getPositionForParameterItem())

    def addAlgorithm(self, alg, parametersMap, valuesMap, outputsMap):
        self.algs.append(alg)
        self.algParameters.append(parametersMap)
        self.algOutputs.append(outputsMap)
        for value in valuesMap.keys():
            self.paramValues[value] = valuesMap[value]
        self.algPos.append(self.getPositionForAlgorithmItem())

    def getPositionForAlgorithmItem(self):
        MARGIN = 20
        BOX_WIDTH = 200
        BOX_HEIGHT = 80
        return QtCore.QPointF(MARGIN + BOX_WIDTH / 2 + len(self.algPos) * (BOX_WIDTH + MARGIN), BOX_HEIGHT + 2 * MARGIN + BOX_HEIGHT / 2 +  len(self.algs) * (BOX_HEIGHT + MARGIN))

    def getPositionForParameterItem(self):
        MARGIN = 20
        BOX_WIDTH = 200
        BOX_HEIGHT = 80
        return QtCore.QPointF(MARGIN + BOX_WIDTH / 2 + len(self.paramPos) * (BOX_WIDTH + MARGIN), MARGIN + BOX_HEIGHT / 2)

    def getSafeNameForHarcodedParameter(self, param):
        return "HARDCODEDPARAMVALUE_" + param.name + "_" + str(len(self.algs))

    def serialize(self):
        s="NAME:" + self.name + "\n"
        s +="GROUP:" + self.group + "\n"

        i = 0
        for param in self.parameters:
            s += "PARAMETER:" + param.serialize() + "\n"
            pt = self.paramPos[i]
            s +=  str(pt.x()) + "," + str(pt.y()) + "\n"
            i+=1
        for key in self.paramValues.keys():
            s += "VALUE:" + key + "=" + str(self.paramValues[key]) + "\n"
        for i in range(len(self.algs)):
            alg = self.algs[i]
            s+="ALGORITHM:" + alg.commandLineName()+"\n"
            pt = self.algPos[i]
            s +=  str(pt.x()) + "," + str(pt.y()) + "\n"
            for param in alg.parameters:
                value = self.algParameters[i][param.name]
                if value:
                    s+=value.serialize() + "\n"
                else:
                    s+=str(None) + "\n"
            for out in alg.outputs:
                s+=str(self.algOutputs[i][out.name]) + "\n"
        return s


    def setPositions(self, paramPos,algPos):
        self.paramPos = paramPos
        self.algPos = algPos


    def prepareAlgorithm(self, alg, iAlg):
        for param in alg.parameters:
            aap = self.algParameters[iAlg][param.name]
            if aap == None:
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

    def getSafeNameForOutput(self, ialg, out):
        return out.name +"_ALG" + str(ialg)


    def getValueFromAlgorithmAndParameter(self, aap):
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
        self.producedOutputs = []
        iAlg = 0
        for alg in self.algs:
            try:
                alg = copy.deepcopy(alg)
                self.prepareAlgorithm(alg, iAlg)
                progress.setText("Running " + alg.name + " [" + str(iAlg+1) + "/" + str(len(self.algs)) +"]")
                outputs = {}
                alg.execute(progress)
                for out in alg.outputs:
                    outputs[out.name] = out.value
                self.producedOutputs.append(outputs)
                iAlg += 1
            except GeoAlgorithmExecutionException, e :
                raise GeoAlgorithmExecutionException("Error executing algorithm " + str(iAlg) + "\n" + e.msg)


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
            runline = "outputs_" + str(i) + "=Sextante.runalg(\"" + alg.commandLineName() + "\""
            for param in alg.parameters:
                aap = self.algParameters[i][param.name]
                if aap == None:
                    runline += ", None"

                if isinstance(param, ParameterMultipleInput):
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


    def commandLineName(self):
        return "modeler:" + os.path.basename(self.descriptionFile)[:-5].lower()

    def setModelerView(self, dialog):
        self.modelerdialog = dialog

    def updateModelerView(self):
        if self.modelerdialog:
            self.modelerdialog.repaintModel()

    def removeAlgorithm(self, index):
        if self.hasDependencies(self.algs[index], index):
            return False
        for out in self.algs[index].outputs:
            val = self.algOutputs[index][out.name]
            if val:
                name = self.getSafeNameForOutput(index, out)
                self.removeOutputFromName(name)
        del self.algs[index]
        del self.algParameters[index]
        del self.algPos[index]
        self.updateModelerView()
        return True

    def removeParameter(self, index):
        if self.hasDependencies(self.parameters[index], index):
            return False
        del self.parameters[index]
        del self.paramPos[index]
        self.updateModelerView()
        return True

    def hasDependencies(self, element, elementIndex):
        '''returns true if some other element depends on the passed one'''
        if isinstance(element, Parameter):
            for alg in self.algParameters:
                for aap in alg.values():
                    if aap.alg == AlgorithmAndParameter.PARENT_MODEL_ALGORITHM and aap.param == element.name:
                        return True
            if isinstance(element, ParameterVector):
                for param in self.parameters:
                    if isinstance(param, ParameterTableField):
                        if param.parent == element.name:
                            return True
        else:
            for alg in self.algParameters:
                for aap in alg.values():
                    if aap.alg == elementIndex:
                        return True

        return False


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
