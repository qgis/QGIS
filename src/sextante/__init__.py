from sextante.SextanteToolboxPlugin import SextanteToolboxPlugin

def name():
    return "SEXTANTE"
def description():
    return "SEXTANTE Geoprocessing platform for QGIS"
def version():
    return "Version 0.1"
def icon():
    return "icon.png"
def qgisMinimumVersion():
    return "1.0"
def classFactory(iface):
    # load SextantePlugin class from file SextantePlugin
    return SextanteToolboxPlugin(iface)
