#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
/***************************************************************************
                               appinfo2cpp.py
                              -------------------
    begin                : 2018-09-24
    copyright            : (C) 2018 by Jürgen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

import sys
from xml.etree import ElementTree as et

strings = {}

d = et.parse('linux/org.qgis.qgis.appdata.xml.in')

r = d.getroot()
for elem in ['name', 'summary', 'description']:
    for c in r.iter(elem):
        if not c.attrib:
            l = list(c)
            t = c.text if not l else "".join([et.tostring(x).decode("utf-8") for x in l])
            strings[t] = 1

f = open("linux/org.qgis.qgis.desktop.in", "r")

for r in f.readlines():
    r = r.strip()
    if r.startswith("Name="):
        strings[r[5:]] = 1
    elif r.startswith("GenericName="):
        strings[r[12:]] = 1

f.close()

print("""
/*
 This is NOT a proper c++ source code. This file is only designed to be caught
 by qmake and included in lupdate. It contains all translateable strings collected
 by pylupdate5.
*/
""")

for k in strings:
    k = k.replace('"', '\\"')
    k = k.replace('\n', '\\n')
    print("translate( \"appinfo\", \"{}\" )".format(k))
