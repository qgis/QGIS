# -*- coding: utf-8 -*-

"""
***************************************************************************
    customwidgets.py
    ---------------------
    Date                 : May 2014
    Copyright            : (C) 2014 by Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

"""
This file is used by pyuic to redirect includes
in custom widgets to the correct QGIS python packages.
It is copied on installation in /pythonX/dist-packages/PyQt4/uic/widget-plugins/
"""

# solution with CW_FILTER not fully working due to include of other files
# (e.g. for flags defined in other source files)

# pluginType = CW_FILTER
# def getFilter():
#       import qgis.gui
#
#       QGIS_widgets = {}
#       for pyClass in dir(qgis.gui):
#               QGIS_widgets[pyClass] = 'qgis.gui'
#
#       def _QGISfilter(widgetname, baseclassname, module):
#               print widgetname, baseclassname, module
#               if widgetname in QGIS_widgets:
#                       return (MATCH, (widgetname, baseclassname, QGIS_widgets[widgetname]))
#               else:
#                       return (NO_MATCH, None)
#
#       return _QGISfilter


pluginType = MODULE


def moduleInformation():
        try:
                import qgis.gui
                return "qgis.gui", dir(qgis.gui)
        except ImportError:
                return "", []
