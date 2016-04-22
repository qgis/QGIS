# -*- coding: utf-8 -*-

"""
***************************************************************************
    __init__.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import yaml
from qgis.core import QGis
from qgis.PyQt.QtCore import QSettings, QLocale


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
