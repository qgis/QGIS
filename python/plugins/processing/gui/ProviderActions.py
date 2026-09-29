"""
***************************************************************************
    ProviderActions.py
    -------------------
    Date                 : April 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Nyall Dawson"
__date__ = "April 2017"
__copyright__ = "(C) 2017, Nyall Dawson"
from warnings import warn

from qgis.gui import QgsGui


class ProviderActions:
    @staticmethod
    def registerProviderActions(provider, actions):
        """Adds actions for a provider"""
        warn(
            "The ProviderActions class is deprecated, use QgsProcessingGuiRegistry.registerProviderToolboxAction instead",
            DeprecationWarning,
            2,
        )
        for action in actions:
            QgsGui.processingGuiRegistry().registerProviderToolboxAction(
                provider.id(), action
            )

    @staticmethod
    def deregisterProviderActions(provider):
        """Removes actions for a provider"""
        warn(
            "The ProviderActions class is deprecated, use QgsProcessingGuiRegistry.deregisterProviderToolboxActions instead",
            DeprecationWarning,
            2,
        )
        QgsGui.processingGuiRegistry().deregisterProviderToolboxActions(provider.id())


class ProviderContextMenuActions:
    @staticmethod
    def registerProviderContextMenuActions(actions):
        """Adds context menu actions for a provider"""
        warn(
            "The ProviderContextMenuActions class is deprecated, use QgsProcessingGuiRegistry.registerProviderToolboxContextAction instead",
            DeprecationWarning,
            2,
        )
        for action in actions:
            QgsGui.processingGuiRegistry().registerProviderToolboxContextAction(
                "deprecated", action
            )

    @staticmethod
    def deregisterProviderContextMenuActions(actions):
        """Removes context menu actions for a provider"""
        warn(
            "The ProviderContextMenuActions class is deprecated, use QgsProcessingGuiRegistry.deregisterProviderToolboxContextActions instead",
            DeprecationWarning,
            2,
        )
        for act in actions:
            QgsGui.processingGuiRegistry().deregisterProviderToolboxContextAction(act)
