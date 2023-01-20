from qgis.server import QgsServerOgcApiHandler


class Handler(QgsServerOgcApiHandler):
    def __init__(self, iface):
        pass


def serverClassFactory(serverIface):
    return Handler(serverIface)
