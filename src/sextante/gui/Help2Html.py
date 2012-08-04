import pickle
from sextante.core.SextanteUtils import SextanteUtils
import os
class Help2Html():

    ALG_DESC = "ALG_DESC"
    ALG_CREATOR = "ALG_CREATOR"
    ALG_HELP_CREATOR = "ALG_HELP_CREATOR"

    def getHtmlFile(self, alg, helpFile):
        if not os.path.exists(helpFile):
            return None
        self.alg = alg
        f = open(helpFile, "rb")
        self.descriptions = pickle.load(f)
        s = "<h2>Algorithm description</h2>\n"
        s += "<p>" + self.getDescription(self.ALG_DESC) + "</p>\n"
        s += "<h2>Input parameters</h2>\n"
        for param in self.alg.parameters:
            s += "<h3>" + param.description + "</h3>\n"
            s += "<p>" + self.getDescription(param.name) + "</p>\n"
        s += "<h2>Outputs</h2>\n"
        for out in self.alg.outputs:
            s += "<h3>" + out.description + "</h3>\n"
            s += "<p>" + self.getDescription(out.name) + "</p>\n"
        filename = SextanteUtils.tempFolder() + os.sep + "temphelp.html"
        tempHtml = open(filename, "w")
        tempHtml.write(s)

        return filename

    def getDescription(self, name):
        if name in self.descriptions    :
            return self.descriptions[name]
        else:
            return ""