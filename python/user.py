# -*- coding: utf-8 -*-

"""
***************************************************************************
    user.py
    ---------------------
    Date                 : January 2015
    Copyright            : (C) 2015 by Nathan Woodrow
    Email                : woodrow dot nathan at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nathan Woodrow'
__date__ = 'January 2015'
__copyright__ = '(C) 2015, Nathan Woodrow'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import sys
import glob
import traceback

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import Qgis, QgsApplication, QgsMessageLog


def load_user_expressions(path):
    """
    Load all user expressions from the given paths
    """
    #Loop all py files and import them
    modules = glob.glob(path + "/*.py")
    names = [os.path.basename(f)[:-3] for f in modules]
    for name in names:
        if name == "__init__":
            continue
        # As user expression functions should be registered with qgsfunction
        # just importing the file is enough to get it to load the functions into QGIS
        try:
            __import__("expressions.{0}".format(name), locals(), globals())
        except:
            error = traceback.format_exc()
            msgtitle = QCoreApplication.translate("UserExpressions", "User expressions")
            msg = QCoreApplication.translate("UserExpressions", "The user expression {0} is not valid").format(name)
            QgsMessageLog.logMessage(msg + "\n" + error, msgtitle, Qgis.Warning)


userpythonhome = os.path.join(QgsApplication.qgisSettingsDirPath(), "python")
expressionspath = os.path.join(userpythonhome, "expressions")

sys.path.append(userpythonhome)

if not os.path.exists(expressionspath):
    os.makedirs(expressionspath)

initfile = os.path.join(expressionspath, "__init__.py")
if not os.path.exists(initfile):
    open(initfile, "w").close()

template = """\"\"\"
Define a new function using the @qgsfunction decorator.

The function accept the following parameters

:param [any]: Define any parameters you want to pass to your function before
              the following arguments.
:param feature: The current feature
:param parent: The QgsExpression object
:param context: If there is an argument called ``context`` found at the last
                position, this variable will contain a ``QgsExpressionContext``
                object, that gives access to various additional information like
                expression variables. E.g. ``context.variable('layer_id')``
:returns: The result of the expression.


The @qgsfunction decorator accepts the following arguments:

:param args: Defines the number of arguments. With ``args='auto'`` the number of
             arguments will automatically be extracted from the signature.
             With ``args=-1``, you can pass any number of arguments.
:param group: The name of the group under which this expression function will
              be listed.
:param handlesnull: Set this to True if your function has custom handling for NULL values.
                    If False, the result will always be NULL as soon as any parameter is NULL.
                    Defaults to False.
:param usesgeometry: Set this to False if your function does not access
                     feature.geometry(). Defaults to True.
:param referenced_columns: An array of attribute names that are required to run
                           this function. Defaults to
                           [QgsFeatureRequest.ALL_ATTRIBUTES].
\"\"\"

from qgis.core import *
from qgis.gui import *

@qgsfunction(args='auto', group='Custom')
def my_sum(value1, value2, feature, parent):
    \"\"\"
    Calculates the sum of the two parameters value1 and value2.
    <h2>Example usage:</h2>
    <ul>
      <li>my_sum(5, 8) -> 13</li>
      <li>my_sum(\"field1\", \"field2\") -> 42</li>
    </ul>
    \"\"\"
    return value1 + value2
"""


try:
    import expressions

    expressions.load = load_user_expressions
    expressions.load(expressionspath)
    expressions.template = template
except ImportError:
    # We get a import error and crash for some reason even if we make the expressions package
    # TODO Fix the crash on first load with no expressions folder
    # But for now it's not the end of the world if it doesn't load the first time
    pass
