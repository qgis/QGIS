# -*- coding: utf-8 -*-

"""
***************************************************************************
    SaveAsPythonScriptAction.py
    ---------------------
    Date                 : April 2013
    Copyright            : (C) 2013 by Victor Olaya
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
import sys
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput

__author__ = 'Victor Olaya'
__date__ = 'April 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from sextante.gui.ContextAction import ContextAction
from sextante.modeler.ModelerAlgorithm import ModelerAlgorithm,\
    AlgorithmAndParameter
from sextante.script.ScriptUtils import ScriptUtils
from PyQt4.QtCore import *
from PyQt4.QtGui import *

class SaveAsPythonScriptAction(ContextAction):

    def __init__(self):
        self.name="Save as Python script"

    def isEnabled(self):
        return isinstance(self.alg, ModelerAlgorithm)

    def execute(self):
        filename = str(QFileDialog.getSaveFileName(None, "Save Script", ScriptUtils.scriptsFolder(), "Python scripts (*.py)"))

        if filename:
            if not filename.endswith(".py"):
                filename += ".py"
            text = self.translateToPythonCode(self.alg)            
            try:
                fout = open(filename, "w")
                fout.write(text)
                fout.close()
                if filename.replace("\\","/").startswith(ScriptUtils.scriptsFolder().replace("\\","/")):
                    self.toolbox.updateTree()
            except:
                QMessageBox.warning(self,
                                    self.tr("I/O error"),
                                    self.tr("Unable to save edits. Reason:\n %1").arg(unicode(sys.exc_info()[1]))
                                   )         
                
    def translateToPythonCode(self, model):
        s = ["##" + model.name + "=name"]
        for param in model.parameters:
            s.append(str(param.getAsScriptCode().lower()))
        i = 0
        for outs in model.algOutputs:
            for out in outs.keys():
                if outs[out]:
                    s.append("##" + out.lower() + "_alg" + str(i) +"=" + model.getOutputType(i, out))
            i += 1
        i = 0
        iMultiple = 0
        for alg in model.algs:
            multiple= []
            runline = "outputs_" + str(i) + "=Sextante.runalg(\"" + alg.commandLineName() + "\""
            for param in alg.parameters:
                aap = model.algParameters[i][param.name]
                if aap == None:
                    runline += ", None"
                elif isinstance(param, ParameterMultipleInput):
                    value = model.paramValues[aap.param]
                    tokens = value.split(";")
                    layerslist = []
                    for token in tokens:
                        iAlg, paramname = token.split("|")
                        if float(iAlg) == float(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM):
                            if model.ismodelparam(paramname):
                                value = paramname.lower()
                            else:
                                value = model.paramValues[paramname]
                        else:
                            value = "outputs_" + str(iAlg) + "['" + paramname +"']"
                        layerslist.append(str(value))

                    multiple.append("multiple_" + str(iMultiple) +"=[" + ",".join(layerslist) + "]")
                    runline +=", \";\".join(multiple_" + str(iMultiple) + ") "
                else:
                    if float(aap.alg) == float(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM):
                        if model.ismodelparam(aap.param):
                            runline += ", " + aap.param.lower()
                        else:
                            runline += ", " + str(model.paramValues[aap.param])
                    else:
                        runline += ", outputs_" + str(aap.alg) + "['" + aap.param +"']"
            for out in alg.outputs:
                value = model.algOutputs[i][out.name]
                if value:
                    name = out.name.lower() + "_alg" + str(i)
                else:
                    name = str(None)
                runline += ", " + name
            i += 1
            s += multiple
            s.append(str(runline + ")"))
        return "\n".join(s)                 