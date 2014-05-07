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

# If pluginType is MODULE, the plugin loader will call moduleInformation.  The
# variable MODULE is inserted into the local namespace by the plugin loader.
pluginType = MODULE


# moduleInformation() must return a tuple (module, widget_list).  If "module"
# is "A" and any widget from this module is used, the code generator will write
# "import A".  If "module" is "A[.B].C", the code generator will write
# "from A[.B] import C".  Each entry in "widget_list" must be unique.
def moduleInformation():
    return "qgis.gui", \
        ("QgsCollapsibleGroupbox"  ,\
         "QgsFieldComboBox"        ,\
         "QgsFieldExpressionWidget",\
         "QgsMapLayerComboBox"     ,\
         "QgsMapLayerProxyModel"     ,\
         "QgsScalevisibilityWidget",\
        )
