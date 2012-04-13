from sextante.SextantePlugin import SextantePlugin

def name():
    return "SEXTANTE"
def description():
    return "SEXTANTE Geoprocessing platform for QGIS"
def version():
    return "Version 1.0.3"
def icon():
    return "icon.png"
def qgisMinimumVersion():
    return "1.0"
def classFactory(iface):
    return SextantePlugin(iface)
