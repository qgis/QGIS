# -*- coding: utf-8 -*-

"""
***************************************************************************
    GeoAlgorithmExecutionException.py
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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


class GeoAlgorithmExecutionException(Exception):

    def __init__(self, msg, stack=None, cause=None):
        Exception.__init__(self)
        self.msg = msg
        self.stack = stack
        self.cause = cause

    def __str__(self):
        msg = self.msg.split('\n')
        msg = '  | ' + '\n  | '.join(msg)

        stack = '\n'.join(self.stack)
        stack = stack.split('\n')
        stack = '    ' + '\n    '.join(stack)

        return '\n\n Message:\n{}\n\n Stack:\n\n{}'.format(msg, stack)
