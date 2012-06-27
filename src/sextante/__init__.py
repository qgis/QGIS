def name():
    return "SEXTANTE"
def description():
    return "SEXTANTE Geoprocessing Platform for QGIS"
def version():
    return "Version 1.0.7"
def icon():
    return "icon.png"
def qgisMinimumVersion():
    return "1.0"
def classFactory(iface):
    from sextante.SextantePlugin import SextantePlugin
    return SextantePlugin(iface)
