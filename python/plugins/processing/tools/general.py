# -*- coding: utf-8 -*-

"""
***************************************************************************
    general.py
    ---------------------
    Date                 : April 2013
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
__date__ = 'April 2013'
__copyright__ = '(C) 2013, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import ConfigParser

from processing.core.Processing import Processing
from processing.core.alglist import algList
from processing.core.parameters import ParameterSelection
from processing.gui.Postprocessing import handleAlgorithmResults


def alglist(text=None):
    s = ''
    for provider in algList.algs.values():
        sortedlist = sorted(provider.values(), key=lambda alg: alg.name)
        for alg in sortedlist:
            if text is None or text.lower() in alg.name.lower():
                s += alg.name.ljust(50, '-') + '--->' + alg.commandLineName() \
                    + '\n'
    print s


def algoptions(name):
    alg = Processing.getAlgorithm(name)
    if alg is not None:
        s = ''
        for param in alg.parameters:
            if isinstance(param, ParameterSelection):
                s += param.name + '(' + param.description + ')\n'
                i = 0
                for option in param.options:
                    s += '\t' + unicode(i) + ' - ' + unicode(option) + '\n'
                    i += 1
        print s
    else:
        print 'Algorithm not found'


def alghelp(name):
    alg = Processing.getAlgorithm(name)
    if alg is not None:
        alg = alg.getCopy()
        print unicode(alg)
        algoptions(name)
    else:
        print 'Algorithm not found'


def runalg(algOrName, *args, **kwargs):
    alg = Processing.runAlgorithm(algOrName, None, *args, **kwargs)
    if alg is not None:
        return alg.getOutputValuesAsDictionary()


def runandload(name, *args, **kwargs):
    return Processing.runAlgorithm(name, handleAlgorithmResults, *args, **kwargs)


def version():
    pluginPath = os.path.split(os.path.dirname(__file__))[0]
    cfg = ConfigParser.SafeConfigParser()
    cfg.read(os.path.join(pluginPath, 'metadata.txt'))
    ver = cfg.get('general', 'version').split('.')
    return 10000 * int(ver[0]) + 100 * int(ver[1]) + int(ver[2])
