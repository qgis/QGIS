from sextante.core.SextanteUtils import SextanteUtils
import os.path

class SextanteConfig():

    OUTPUT_FOLDER = "OUTPUT_FOLDER"

    settings = {}

    @staticmethod
    def initialize():
        SextanteConfig.addSetting(Setting("General", SextanteConfig.OUTPUT_FOLDER,
                                           "Output folder", os.path.join(SextanteUtils.userFolder(),"outputs" )))


    @staticmethod
    def addSetting(setting):
        SextanteConfig.settings[setting.name] = setting

    @staticmethod
    def getSettings():
        settings={}
        for setting in SextanteConfig.settings.values():
            if not setting.group in settings:
                group = []
                settings[setting.group] = group
            else:
                group = settings[setting.group]
            group.append(setting)
        return settings

    @staticmethod
    def configFile():
        return os.path.join(SextanteUtils.userFolder(), "sextante_qgis.conf")

    @staticmethod
    def loadSettings():
        lines = open(SextanteConfig.configFile())
        line = lines.readline().strip("\n")
        while line != "":
            tokens = line.split("=")
            if tokens[0] in SextanteConfig.settings.keys():
                setting = SextanteConfig.settings[tokens[0]]
                if isinstance(setting.value, bool):
                    setting.value = bool(tokens[1])
                else:
                    setting.value = tokens[1]
                SextanteConfig.addSetting(setting)
            line = lines.readline().strip("\n")
        lines.close()

    @staticmethod
    def saveSettings():
        fout = open(SextanteConfig.configFile(), "w")
        for setting in SextanteConfig.settings.values():
            fout.write(str(setting) + "\n")
        fout.close()

    @staticmethod
    def getSetting(name):
        if name in SextanteConfig.settings.keys():
            return SextanteConfig.settings[name].value
        else:
            return None



class Setting():

    def __init__(self, group, name, description, default):
        self.group=group
        self.name = name
        self.description = description
        self.default = default
        self.value = default

    def __str__(self):
        return self.name + "=" + str(self.value)