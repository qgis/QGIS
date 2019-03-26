# -#- coding: utf-8 -#-

###########################################################################
#    __init__.py
#    ---------------------
#    Date                 : March 2019
#    Copyright            : (C) 2019 by Nyall Dawson
#    Email                : nyall dot dawson at gmail dot com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

"""3rd party QGIS plugin additions.

This module contains stable API added by QGIS plugins. It is provided
as a container for these plugins to monkey-patch their stable, external
API into.

An example of a plugin patching a function into the qgis.plugins library:

.. code-block:: python

       from qgis import plugins

       def my_function():
           print('Test')

       qgis.plugins.my_function = my_function


An example of a plugin patching a module into the qgis.plugins library:

.. code-block:: python

       from qgis import plugins
       from my_plugin import my_module

       qgis.plugins.my_module = my_module


.. warning::

    Any API exposed through this module is 3rd party, and is not the
    responsibility of the QGIS project. Bugs must be filed with the plugin
    authors themselves, and not on the QGIS project issue tracker.
"""

__author__ = 'Nyall Dawson'
__date__ = 'March 2019'
__copyright__ = '(C) 2019, Nyall Dawson'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'
