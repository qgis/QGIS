# -#- coding: utf-8 -#-

###########################################################################
#    __init__.py
#    ---------------------
#    Date                 : November 2018
#    Copyright            : (C) 2018 by Nathan Woodrow
#    Email                : woodrow dot nathan at gmail dot com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

"""
QGIS Processing Python additions.

This module contains stable API adding additional Python specific functionality
to the core QGIS c++ Processing classes.
"""

__author__ = 'Nathan Woodrow'
__date__ = 'November 2018'
__copyright__ = '(C) 2018, Nathan Woodrow'

from .algfactory import ProcessingAlgFactory

alg = ProcessingAlgFactory()


# "Forward declare" functions which will be patched in when the Processing plugin loads:

def algorithmHelp(id: str):
    """
    Prints algorithm parameters with their types. Also
    provides information about parameters and outputs,
    and their acceptable values.

    :param id: An algorithm's ID
    :type id: str
    """
    from qgis.core import QgsNotSupportedException
    raise QgsNotSupportedException('Processing plugin has not been loaded')


def run(algOrName, parameters, onFinish=None, feedback=None, context=None, is_child_algorithm=False):
    """
    Executes given algorithm and returns its outputs as dictionary object.

    :param algOrName: Either an instance of an algorithm, or an algorithm's ID
    :param parameters: Algorithm parameters dictionary
    :param onFinish: optional function to run after the algorithm has completed
    :param feedback: Processing feedback object
    :param context: Processing context object
    :param is_child_algorithm: Set to True if this algorithm is being run as part of a larger algorithm,
    i.e. it is a sub-part of an algorithm which calls other Processing algorithms.

    :return: algorithm results as a dictionary, or None if execution failed
    :rtype: Union[dict, None]
    """
    from qgis.core import QgsNotSupportedException
    raise QgsNotSupportedException('Processing plugin has not been loaded')


def runAndLoadResults(algOrName, parameters, feedback=None, context=None):
    """
    Executes given algorithm and load its results into the current QGIS project
    when possible.

    :param algOrName: Either an instance of an algorithm, or an algorithm's ID
    :param parameters: Algorithm parameters dictionary
    :param feedback: Processing feedback object
    :param context: Processing context object

    :return: algorithm results as a dictionary, or None if execution failed
    :rtype: Union[dict, None]
    """
    from qgis.core import QgsNotSupportedException
    raise QgsNotSupportedException('Processing plugin has not been loaded')


def createAlgorithmDialog(algOrName, parameters={}):
    """
    Creates and returns an algorithm dialog for the specified algorithm, prepopulated
    with a given set of parameters. It is the caller's responsibility to execute
    and delete this dialog.

    :param algOrName: Either an instance of an algorithm, or an algorithm's ID
    :param parameters: Initial algorithm parameters dictionary

    :return: algorithm results as a dictionary, or None if execution failed
    :rtype: Union[dict, None]
    """
    from qgis.core import QgsNotSupportedException
    raise QgsNotSupportedException('Processing plugin has not been loaded')


def execAlgorithmDialog(algOrName, parameters={}):
    """
    Executes an algorithm dialog for the specified algorithm, prepopulated
    with a given set of parameters.

    :param algOrName: Either an instance of an algorithm, or an algorithm's ID
    :param parameters: Initial algorithm parameters dictionary

    :return: algorithm results as a dictionary, or None if execution failed
    :rtype: Union[dict, None]
    """
    from qgis.core import QgsNotSupportedException
    raise QgsNotSupportedException('Processing plugin has not been loaded')


def createContext(feedback=None):
    """
    Creates a default processing context

    :param feedback: Optional existing QgsProcessingFeedback object, or None to use a default feedback object
    :type feedback: Optional[QgsProcessingFeedback]

    :returns: New QgsProcessingContext object
    :rtype: QgsProcessingContext
    """
    from qgis.core import QgsNotSupportedException
    raise QgsNotSupportedException('Processing plugin has not been loaded')
