# -*- coding: utf-8 -*-

"""
***************************************************************************
    help.py
    ---------------------
    Date                 : March 2013
    Copyright            : (C) 2013 by Victor Olaya
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
__date__ = 'March 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import codecs

from processing.core.Processing import Processing
from processing.core.parameters import ParameterMultipleInput, ParameterTableField, ParameterVector, ParameterSelection
from processing.tools.system import mkdir


def baseHelpForAlgorithm(alg, folder):
    baseDir = os.path.join(folder, alg.provider.getName().lower())
    mkdir(baseDir)

    groupName = alg.group.lower()
    groupName = groupName.replace('[', '').replace(']', '').replace(' - ', '_')
    groupName = groupName.replace(' ', '_')
    cmdLineName = alg.commandLineName()
    algName = cmdLineName[cmdLineName.find(':') + 1:].lower()
    validChars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_'
    safeGroupName = ''.join(c for c in groupName if c in validChars)
    safeAlgName = ''.join(c for c in algName if c in validChars)

    dirName = os.path.join(baseDir, safeGroupName)
    mkdir(dirName)
    filePath = os.path.join(dirName, safeAlgName + '.rst')

    with codecs.open(filePath, 'w', encoding='utf-8') as f:
        f.write('{}\n'.format(alg.name))
        f.write('{}\n\n'.format('=' * len(alg.name)))
        f.write('Description\n')
        f.write('-----------\n\n<put algorithm description here>\n\n')

        # Algorithm parameters
        f.write('Parameters\n')
        f.write('----------\n\n')
        for p in alg.parameters:
            if isinstance(p, (ParameterMultipleInput, ParameterTableField, ParameterVector)):
                f.write('``{}`` [{}: {}]\n'.format(p.description, p.typeName(), p.dataType()))
            else:
                f.write('``{}`` [{}]\n'.format(p.description, p.typeName()))

            if hasattr(p, 'optional'):
                if p.optional:
                    f.write('  Optional.\n\n')

            f.write('  <put parameter description here>\n\n')

            if isinstance(p, ParameterSelection):
                f.write('  Options:\n\n')
                for count, opt in enumerate(p.options):
                    f.write('  * {} --- {}\n'.format(count, opt))
                f.write('\n')

            if hasattr(p, 'default'):
                f.write('  Default: *{}*\n\n'.format(p.default if p.default != '' else '(not set)'))

        # Algorithm outputs
        f.write('Outputs\n')
        f.write('-------\n\n')
        for o in alg.outputs:
            f.write('``{}`` [{}]\n'.format(o.description, o.typeName()))
            f.write('  <put output description here>\n\n')

        # Console usage
        f.write('Console usage\n')
        f.write('-------------\n')
        f.write('\n::\n\n')
        cmd = "  processing.runalg('{}', ".format(alg.commandLineName())
        for p in alg.parameters:
            cmd += '{}, '.format(p.name.lower().strip())

        for o in alg.outputs:
            if not o.hidden:
                cmd += '{}, '.format(o.name.lower().strip())
        cmd = cmd[:-2] + ')\n\n'
        f.write(cmd)

        f.write('See also\n')
        f.write('--------\n\n')


def createBaseHelpFiles(folder):
    for provider in Processing.providers:
        if 'grass' in provider.getName():
            continue

        for alg in provider.algs:
            baseHelpForAlgorithm(alg, folder)


def createAlgorithmHelp(algName, folder):
    alg = Processing.getAlgorithm(algName)
    baseHelpForAlgorithm(alg, folder)
