from sextante.core.SextanteUtils import SextanteUtils

class Output(object):

    def __init__(self, name="", description=""):
        self.name = name
        self.description = description
        self.value = None

    def __str__(self):
        return self.name + " <" + self.__module__.split(".")[-1] +">"

    def getValueAsCommandLineParameter(self):
        if self.value == None:
            return str(None)
        else:
            if not SextanteUtils.isWindows():
                return "\"" + str(self.value) + "\""
            else:
                return "\"" + str(self.value).replace("\\", "\\\\") + "\""

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description

    def setValue(self, value):
        try:
            if value != None:
                value = value.strip()
            self.value = value
            return True
        except:
            return False