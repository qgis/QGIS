# -*- coding: utf-8 -*-

"""
***************************************************************************
    v_lrs_create.py
    ---------------
    Date                 : March 2016
    Copyright            : (C) 2016 by Médéric Ribreux
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
__date__ = 'March 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


def processOutputs(alg):
    # add some export commands
    command = 'v.build.all'
    alg.commands.append(command)

    # export the SQLite table to CSV
    rstable = alg.getOutputValue('rstable')
    # I don't use db.out.ogr because it doesn't work
    command = 'db.select table={} separator=comma output=\"{}\" --overwrite'.format(
        alg.exportedLayers[rstable],
        rstable
    )
    alg.commands.append(command)
    command = 'echo \"Integer\",\"Integer\",\"Integer\",\"Real\",\"Real\",\"Real\",\"Real\",\"Real\",\"Real\",\"Real\" > \"{}t\"'.format(rstable)
    alg.commands.append(command)
    alg.processOutputs()
