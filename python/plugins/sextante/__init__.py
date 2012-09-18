def name():
    return "SEXTANTE"

def description():
    return "SEXTANTE Geoprocessing Platform for QGIS"

def version():
    return "1.0.8"

def icon():
    return "images/toolbox.png"

def qgisMinimumVersion():
    return "1.0"

def classFactory(iface):
    from sextante.SextantePlugin import SextantePlugin
    return SextantePlugin(iface)
