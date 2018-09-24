#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
/***************************************************************************
                               ts2appinfo.py
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
import os
from glob import glob
from xml.etree import ElementTree as et
from PyQt5.QtCore import QCoreApplication, QTranslator

strings = {}

d = et.parse('linux/org.qgis.qgis.appdata.xml.in')

r = d.getroot()
for elem in ['name', 'summary', 'description']:
    for c in r.iter(elem):
        if not c.attrib:
            l = list(c)
            t = c.text if not l else "".join([et.tostring(x).decode("utf-8") for x in l])
            strings[t] = {}
        else:
            r.remove(c)

f = open("linux/org.qgis.qgis.desktop.in", "r")
lines = f.readlines()

for line in lines:
    line = line.strip()
    for prefix in ['Name', 'GenericName']:
        if line.startswith(prefix + "="):
            strings[line[len(prefix) + 1:]] = {}

f.close()

try:
    argvb = list(map(os.fsencode, sys.argv))
except AttributeError:
    argvb = sys.argv

app = QCoreApplication(argvb)

for qm in glob(sys.argv[1] + "/output/i18n/qgis_*.qm"):
    translator = QTranslator()
    translator.load(qm)

    lang = qm[len(sys.argv[1] + "/output/i18n/qgis_"):-3]

    for s in strings:
        translation = translator.translate("appinfo", s, "")
        if translation in [s, '']:
            continue
        strings[s][lang] = translation

for elem in ['name', 'summary', 'description']:
    for c in r.iter(elem):
        if c.attrib:
            continue

        l = list(c)
        s = c.text if not l else "".join([et.tostring(x).decode("utf-8") for x in l])

        for lang in strings[s]:
            e = et.Element(elem, attrib={"xml:lang": lang})
            e.text = strings[s][lang]
            e.tail = c.tail
            r.append(e)

d.write(sys.argv[1] + "/org.qgis.qgis.appdata.xml", encoding="UTF-8", xml_declaration=True)

f = open(sys.argv[1] + "/org.qgis.qgis.desktop", "w")

for line in lines:
    skip = False
    for prefix in ['Name', 'GenericName']:
        if line.startswith(prefix + "="):
            skip = True
            f.write(line)

            t = line.strip()[len(prefix) + 1:]
            for lang in strings[t]:
                f.write("{}[{}]={}\n".format(prefix, lang, strings[t][lang]))

        elif line.startswith(prefix + "["):
            skip = True
            continue

    if not skip:
        f.write(line)

f.close()
