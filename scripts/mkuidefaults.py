#!/usr/bin/env python3
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

import sys
import struct

from PyQt5.QtCore import QCoreApplication, QSettings


def chunks(l, n):
    for i in range(0, len(l), n):
        yield l[i:i + n]


QCoreApplication.setOrganizationName("QGIS")
QCoreApplication.setOrganizationDomain("qgis.org")
QCoreApplication.setApplicationName("QGIS3")

if len(sys.argv) == 1:
    print("Usage: ./scripts/mkuidefaults.py \"location_to_ini\"")
    sys.exit(1)

s = QSettings(sys.argv[1], QSettings.IniFormat)

ba = bytes(s.value("/UI/geometry"))

with open("src/app/ui_defaults.h", "w") as f:

    f.write("#ifndef UI_DEFAULTS_H\n#define UI_DEFAULTS_H\n" +
            "\nstatic const unsigned char defaultUIgeometry[] =\n{\n")

    for chunk in chunks(ba, 16):
        f.write('  {},\n'.format(
            ', '.join(map(hex, struct.unpack('B' * len(chunk), chunk)))))

    f.write("};\n\nstatic const unsigned char defaultUIstate[] =\n{\n")

    ba = bytes(s.value("/UI/state"))

    for chunk in chunks(ba, 16):
        f.write('  {},\n'.format(
            ', '.join(map(hex, struct.unpack('B' * len(chunk), chunk)))))

    try:
        ba = bytes(s.value("/app/LayoutDesigner/geometry"))
        f.write("};\n\nstatic const unsigned char " +
                "defaultLayerDesignerUIgeometry[] =\n{\n")

        for chunk in chunks(ba, 16):
            f.write('  {},\n'.format(
                ', '.join(map(hex, struct.unpack('B' * len(chunk), chunk)))))
    except TypeError as ex:
        pass

    try:
        ba = bytes(s.value("/app/LayoutDesigner/state"))
        f.write("};\n\nstatic const unsigned char " +
                "defaultLayerDesignerUIstate[] =\n{\n")

        for chunk in chunks(ba, 16):
            f.write('  {},\n'.format(
                ', '.join(map(hex, struct.unpack('B' * len(chunk), chunk)))))
    except TypeError as ex:
        pass

    f.write("};\n\n#endif // UI_DEFAULTS_H\n")
