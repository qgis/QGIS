# -*- coding: utf-8 -*-

"""
***************************************************************************
    v_net_flow.py
    ---------------------
    Date                 : December 2015
    Copyright            : (C) 2015 by Médéric Ribreux
    Email                : medspx at medspx dot fr
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Médéric Ribreux'
__date__ = 'December 2015'
__copyright__ = '(C) 2015, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


from .v_net import incorporatePoints, variableOutput


def checkParameterValuesBeforeExecuting(alg):
    """ Verify if we have the right parameters """
    params = [u'where', u'cats']
    values = []
    for param in params:
        for i in [u'source', u'sink']:
            values.append(alg.getParameterValue(u'{}_{}'.format(i, param)))

    if (values[0] or values[2]) and (values[1] or values[3]):
        return None

    return alg.tr("You need to set at least source/sink_where or source/sink_cats parameters for each set!")


def processCommand(alg):
    incorporatePoints(alg)


def processOutputs(alg):
    outputParameter = {u"output": [u"line", 1],
                       u"cut": [u"line", 1]}
    variableOutput(alg, outputParameter)
