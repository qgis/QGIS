import os
from sextante.core.SextanteUtils import SextanteUtils, mkdir
from sextante.core.SextanteConfig import SextanteConfig

class ModelerUtils:

    MODELS_FOLDER = "MODELS_FOLDER"
    ACTIVATE_MODELS = "ACTIVATE_MODELS"

    @staticmethod
    def modelsFolder():
        folder = SextanteConfig.getSetting(ModelerUtils.MODELS_FOLDER)
        if folder == None:
            folder = SextanteUtils.userFolder() + os.sep + "models"
        mkdir(folder)

        return folder

    @staticmethod
    def getAlgorithm(name):
        for provider in ModelerUtils.allAlgs.values():
            if name in provider:
                return provider[name]
        return None


    @staticmethod
    def getAlgorithms():
        return ModelerUtils.allAlgs





