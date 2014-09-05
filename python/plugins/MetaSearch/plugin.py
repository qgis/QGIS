# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright (C) 2010 NextGIS (http://nextgis.org),
#                    Alexander Bruy (alexander.bruy@gmail.com),
#                    Maxim Dubinin (sim@gis-lab.info),
#
# Copyright (C) 2014 Tom Kralidis (tomkralidis@gmail.com)
#
# This source is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This code is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
###############################################################################

import logging
import os

from PyQt4.QtCore import QCoreApplication, QLocale, QSettings, QTranslator
from PyQt4.QtGui import QAction, QIcon

from MetaSearch.dialogs.maindialog import MetaSearchDialog
from MetaSearch.util import StaticContext, open_url

LOGGER = logging.getLogger('MetaSearch')


class MetaSearchPlugin(object):
    """base plugin"""
    def __init__(self, iface):
        """init"""

        self.iface = iface
        self.context = StaticContext()
        self.action_run = None
        self.action_help = None
        self.dialog = None
        self.web_menu = '&MetaSearch'

        LOGGER.debug('Setting up i18n')

        # TODO: does this work for locales like: pt_BR ?
        locale_name = QSettings().value("locale/userLocale")[0:2]
        # this one below does not pick up when you load QGIS with --lang param
#        locale_name = str(QLocale.system().name()).split('_')[0]

        LOGGER.debug('Locale name: %s', locale_name)

        # load if exists
        tr_file = os.path.join(self.context.ppath, 'locale', locale_name,
                               'LC_MESSAGES', 'ui.qm')

        if os.path.exists(tr_file):
            self.translator = QTranslator()
            result = self.translator.load(tr_file)
            if not result:
                msg = 'Failed to load translation: %s' % tr_file
                LOGGER.error(msg)
                raise RuntimeError(msg)
            QCoreApplication.installTranslator(self.translator)

        LOGGER.debug(QCoreApplication.translate('MetaSearch',
                     'Translation loaded: %s' % tr_file))

    def initGui(self):
        """startup"""

        # run
        run_icon = QIcon('%s/%s' % (self.context.ppath,
                                    'images/MetaSearch.png'))
        self.action_run = QAction(run_icon, 'MetaSearch',
                                  self.iface.mainWindow())
        self.action_run.setWhatsThis(QCoreApplication.translate('MetaSearch',
                                     'MetaSearch plugin'))
        self.action_run.setStatusTip(QCoreApplication.translate('MetaSearch',
                                     'Search Metadata Catalogues'))

        self.action_run.triggered.connect(self.run)

        self.iface.addWebToolBarIcon(self.action_run)
        self.iface.addPluginToWebMenu(self.web_menu, self.action_run)

        # help
        help_icon = QIcon('%s/%s' % (self.context.ppath, 'images/help.png'))
        self.action_help = QAction(help_icon, 'Help', self.iface.mainWindow())
        self.action_help.setWhatsThis(QCoreApplication.translate('MetaSearch',
                                      'MetaSearch plugin help'))
        self.action_help.setStatusTip(QCoreApplication.translate('MetaSearch',
                                      'Get Help on MetaSearch'))
        self.action_help.triggered.connect(self.help)

        self.iface.addPluginToWebMenu(self.web_menu, self.action_help)

        # prefab the dialog but not open it yet
        self.dialog = MetaSearchDialog(self.iface)

    def unload(self):
        """teardown"""

        # remove the plugin menu item and icon
        self.iface.removePluginWebMenu(self.web_menu, self.action_run)
        self.iface.removePluginWebMenu(self.web_menu, self.action_help)
        self.iface.removeWebToolBarIcon(self.action_run)

    def run(self):
        """open MetaSearch"""

        self.dialog.exec_()

    def help(self):
        """open help in user's default web browser"""

        open_url(self.context.metadata.get('general', 'homepage'))
