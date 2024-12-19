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
__copyright__ = "(C) 2017, Nyall Dason"


class ProviderActions:
    actions = {}

    @staticmethod
    def registerProviderActions(provider, actions):
        """Adds actions for a provider"""
        ProviderActions.actions[provider.id()] = actions

    @staticmethod
    def deregisterProviderActions(provider):
        """Removes actions for a provider"""
        if provider.id() in ProviderActions.actions:
            del ProviderActions.actions[provider.id()]


class ProviderContextMenuActions:
    # All the registered context menu actions for the toolbox
    actions = []

    @staticmethod
    def registerProviderContextMenuActions(actions):
        """Adds context menu actions for a provider"""
        ProviderContextMenuActions.actions.extend(actions)

    @staticmethod
    def deregisterProviderContextMenuActions(actions):
        """Removes context menu actions for a provider"""
        for act in actions:
            ProviderContextMenuActions.actions.remove(act)
