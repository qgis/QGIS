from sextante.core.SextanteConfig import SextanteConfig
from PyQt4 import QtCore
class SextanteExternalAppsConfigurer():

    @staticmethod
    def autoConfigure():
        IS_FIRST_USAGE = "IS_FIRST_USAGE"
        settings = QtCore.QSettings()
        if not settings.contains(IS_FIRST_USAGE):
            settings.setValue(IS_FIRST_USAGE, True)




