from sextanteexampleprovider.SextanteExampleProviderPlugin import SextanteExampleProviderPlugin
def name():
    return "SEXTANTE example provider"
def description():
    return "An example plugin that adds algorithm to SEXTANTE"
def version():
    return "Version 1.0"
def icon():
    return "icon.png"
def qgisMinimumVersion():
    return "1.0"
def classFactory(iface):
    return SextanteExampleProviderPlugin()
