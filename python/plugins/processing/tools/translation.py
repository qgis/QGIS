# -*- coding: utf-8 -*-

"""
***************************************************************************
    classification.py
    ---------------------
    Date                 : July 2015
    Copyright            : (C) 2015 by Arnaud Morvan
    Email                : arnaud dot morvan at camptocamp dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Arnaud Morvan'
__date__ = 'July 2015'
__copyright__ = '(C) 2015, Arnaud Morvan'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from processing.core.Processing import Processing
from processing.gui.AlgorithmClassification import (
    loadClassification, getClassificationEn)


def updateTranslations():
    """Update processing.algs.translations module.

    Need QGIS python API on python path, can be run from QGIS console. Example:

    from processing.tools.translation import updateTranslations
    updateTranslations()
    """

    loadClassification()

    f = open(os.path.join(os.path.dirname(__file__), '../algs/translations.py'), 'w')
    f.write('''# -*- coding: utf-8 -*-

"""
Don't edit this file manually.
Update it from QGIS console:

from processing.tools.translation import updateTranslations
updateTranslations()
"""

from qgis.PyQt.QtCore import QCoreApplication

def translationShadow():
''')
    groups = {}
    for provider in Processing.providers:
        f.write('''
    """{}"""
'''.format(provider.__class__.__name__))
        for alg in provider.algs:
            display_name = alg.name
            f.write("    QCoreApplication.translate(\"{}\", \"{}\")\n"
                    .format(alg.__class__.__name__,
                            display_name.replace('"', '\\"')))
            if alg.group not in groups:
                groups[alg.group] = 'AlgorithmClassification'
            group, subgroup = getClassificationEn(alg)
            if group is not None and group not in groups:
                groups[group] = 'AlgorithmClassification'
            if subgroup is not None and subgroup not in groups:
                groups[subgroup] = 'AlgorithmClassification'

    f.write('''
    """Groups and subgroups"""
''')
    for group, context in groups.iteritems():
        f.write("    QCoreApplication.translate(\"{}\", \"{}\")\n"
                .format(context,
                        group.replace('"', '\\"')))
