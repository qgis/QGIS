# -*- coding: utf-8 -*-

"""
***************************************************************************
    mkuidefaults.py
    ---------------------
    Date                 : June 2013
    Copyright            : (C) 2013 by Juergen E. Fischer
    Email                : jef at norbit dot de
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Juergen E. Fischer'
__date__ = 'June 2013'
__copyright__ = '(C) 2013, Juergen E. Fischer'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import QCoreApplication, QSettings


def chunks(l, n):
    for i in xrange(0, len(l), n):
        yield l[i:i + n]

QCoreApplication.setOrganizationName("QGIS")
QCoreApplication.setOrganizationDomain("qgis.org")
QCoreApplication.setApplicationName("QGIS2")

s = QSettings()

ba = s.value("/UI/geometry").toByteArray()

f = open("src/app/ui_defaults.h", "w")

f.write("#ifndef UI_DEFAULTS_H\n#define UI_DEFAULTS_H\n\nstatic const unsigned char defaultUIgeometry[] =\n{\n")

for chunk in chunks(ba, 16):
    f.write("  %s,\n" % ", ".join(map(lambda x: "0x%02x" % ord(x), chunk)))

f.write("};\n\nstatic const unsigned char defaultUIstate[] =\n{\n")

ba = s.value("/UI/state").toByteArray()

for chunk in chunks(ba, 16):
    f.write("  %s,\n" % ", ".join(map(lambda x: "0x%02x" % ord(x), chunk)))

ba = s.value("/Composer/geometry").toByteArray()

f.write("};\n\nstatic const unsigned char defaultComposerUIgeometry[] =\n{\n")

for chunk in chunks(ba, 16):
    f.write("  %s,\n" % ", ".join(map(lambda x: "0x%02x" % ord(x), chunk)))

f.write("};\n\nstatic const unsigned char defaultComposerUIstate[] =\n{\n")

ba = s.value("/ComposerUI/state").toByteArray()

for chunk in chunks(ba, 16):
    f.write("  %s,\n" % ", ".join(map(lambda x: "0x%02x" % ord(x), chunk)))

f.write("};\n\n#endif // UI_DEFAULTS_H\n")

f.close()
