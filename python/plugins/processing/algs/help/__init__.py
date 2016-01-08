import os
import yaml
from qgis.core import *
from PyQt4.QtCore import QSettings, QLocale


def loadShortHelp():
    h = {}
    path = os.path.dirname(__file__)
    for f in os.listdir(path):
        if f.endswith("yaml"):
            filename = os.path.join(path, f)
            with open(filename) as stream:
                h.update(yaml.load(stream))
    version = ".".join(QGis.QGIS_VERSION.split(".")[0:2])
    overrideLocale = QSettings().value('locale/overrideFlag', False, bool)
    if not overrideLocale:
        locale = QLocale.system().name()[:2]
    else:
        locale = QSettings().value('locale/userLocale', '')
    locale = locale.split("_")[0]

    def replace(s):
        if s is not None:
            return s.replace("{qgisdocs}", "https://docs.qgis.org/%s/%s/docs" % (version, locale))
        else:
            return None
    h = {k: replace(v) for k, v in h.iteritems()}
    return h


shortHelp = loadShortHelp()
