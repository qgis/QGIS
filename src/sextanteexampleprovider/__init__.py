from sextanteexampleprovider.SextanteExampleProviderPlugin import SextanteExampleProviderPlugin
def name():
    return "SEXTANTE example provider"
def description():
    return "An example plugin that adds algorithms to SEXTANTE. Mainly created to guide developers in the process of creating plugins that add new capabilities to SEXTANTE"
def version():
    return "Version 1.0"
def icon():
    return "icon.png"
def qgisMinimumVersion():
    return "1.0"
def classFactory(iface):
    return SextanteExampleProviderPlugin()
