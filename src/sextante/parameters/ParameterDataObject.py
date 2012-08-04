from sextante.parameters.Parameter import Parameter
from sextante.core.SextanteUtils import SextanteUtils

class ParameterDataObject(Parameter):

    def getValueAsCommandLineParameter(self):
        if self.value == None:
            return str(None)
        else:
            if not SextanteUtils.isWindows():
                return "\"" + unicode(self.value) + "\""
            else:
                return "\"" + unicode(self.value).replace("\\", "\\\\") + "\""