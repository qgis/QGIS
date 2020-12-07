# -*- coding: utf-8 -*-

"""
***************************************************************************
    Help2Html.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
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
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

import os
import re
import json

from qgis.PyQt.QtCore import QCoreApplication, QUrl

from processing.tools import system

ALG_DESC = 'ALG_DESC'
ALG_CREATOR = 'ALG_CREATOR'
ALG_HELP_CREATOR = 'ALG_HELP_CREATOR'
ALG_VERSION = 'ALG_VERSION'

exps = [(r"\*(.*?)\*", r"<i>\1</i>"),
        ("``(.*?)``", r'<FONT FACE="courier">\1</FONT>'),
        ("(.*?)\n==+\n+?", r'<h2>\1</h2>'),
        ("(.*?)\n--+\n+?", r'<h3>\1</h3>'),
        (r"::(\s*\n(\s*\n)*((\s+).*?\n)(((\4).*?\n)|(\s*\n))*)", r"<pre>\1</pre>"),
        ("\n+", "</p><p>")]


def getHtmlFromRstFile(rst):
    if not os.path.exists(rst):
        return None
    with open(rst) as f:
        lines = f.readlines()
    s = "".join(lines)
    for exp, replace in exps:
        p = re.compile(exp)
        s = p.sub(replace, s)
    return s


def getHtmlFromHelpFile(alg, helpFile):
    if not os.path.exists(helpFile):
        return None
    try:
        with open(helpFile) as f:
            descriptions = json.load(f)

        content = getHtmlFromDescriptionsDict(alg, descriptions)
        algGroup, algName = alg.id().split(':')
        filePath = os.path.join(system.tempHelpFolder(), "{}_{}.html".format(algGroup, algName))
        with open(filePath, 'w', encoding='utf-8') as f:
            f.write(content)
        return QUrl.fromLocalFile(filePath).toString()
    except:
        return None


def getHtmlFromDescriptionsDict(alg, descriptions):
    s = tr('<html><body><h2>Algorithm description</h2>\n')
    s += '<p>' + getDescription(ALG_DESC, descriptions) + '</p>\n'
    s += tr('<h2>Input parameters</h2>\n')
    for param in alg.parameterDefinitions():
        s += '<h3>' + param.description() + '</h3>\n'
        s += '<p>' + getDescription(param.name(), descriptions) + '</p>\n'
    s += tr('<h2>Outputs</h2>\n')
    for out in alg.outputs:
        s += '<h3>' + out.description() + '</h3>\n'
        s += '<p>' + getDescription(out.name(), descriptions) + '</p>\n'
    s += '<br>'
    s += tr('<p align="right">Algorithm author: {0}</p>').format(getDescription(ALG_CREATOR, descriptions))
    s += tr('<p align="right">Help author: {0}</p>').format(getDescription(ALG_HELP_CREATOR, descriptions))
    s += tr('<p align="right">Algorithm version: {0}</p>').format(getDescription(ALG_VERSION, descriptions))
    s += '</body></html>'
    return s


def getDescription(name, descriptions):
    if name in descriptions:
        return str(descriptions[name]).replace("\n", "<br>")
    else:
        return ''


def tr(string):
    return QCoreApplication.translate('Help2Html', string)
