# -*- coding: utf-8 -*-

"""
***************************************************************************
    FieldsMapper.py
    ---------------------
    Date                 : October 2014
    Copyright            : (C) 2014 by Arnaud Morvan
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
__date__ = 'October 2014'
__copyright__ = '(C) 2014, Arnaud Morvan'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


from processing.core.parameters import Parameter


class ParameterFieldsMapping(Parameter):

    def __init__(self, name='', description='', parent=None):
        Parameter.__init__(self, name, description)
        self.parent = parent
        self.value = []

    def getValueAsCommandLineParameter(self):
        return '"' + unicode(self.value) + '"'

    def setValue(self, value):
        if value is None:
            return False
        if isinstance(value, list):
            self.value = value
            return True
        if isinstance(value, unicode):
            try:
                self.value = eval(value)
                return True
            except Exception as e:
                print unicode(e) # display error in console
                return False
        return False
