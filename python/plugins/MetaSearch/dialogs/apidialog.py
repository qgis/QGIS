###############################################################################
#
# CSW Client
# ---------------------------------------------------------
# QGIS Catalog Service client.
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

import json

from qgis.PyQt.QtWidgets import QDialog, QVBoxLayout
from qgis.gui import QgsCodeEditorJson, QgsCodeEditorHTML
from MetaSearch.util import get_ui_class, prettify_xml

BASE_CLASS = get_ui_class("apidialog.ui")


class APIRequestResponseDialog(QDialog, BASE_CLASS):
    """Raw XML Dialogue"""

    def __init__(self, request, response, mime_type: str):
        """init"""
        super().__init__()

        self.setupUi(self)

        if mime_type == "json":
            self.txtbrAPIRequest = QgsCodeEditorJson()
            self.txtbrAPIResponse = QgsCodeEditorJson()
            self.txtbrAPIRequest.setText(json.dumps(request, indent=4))
            self.txtbrAPIResponse.setText(json.dumps(response, indent=4))
        else:
            self.txtbrAPIRequest = QgsCodeEditorHTML()
            self.txtbrAPIResponse = QgsCodeEditorHTML()
            self.txtbrAPIRequest.setText(prettify_xml(request))
            self.txtbrAPIResponse.setText(prettify_xml(response))

        self.txtbrAPIRequest.setReadOnly(True)
        self.txtbrAPIResponse.setReadOnly(True)

        request_layout = QVBoxLayout()
        request_layout.setContentsMargins(0, 0, 0, 0)
        request_layout.addWidget(self.txtbrAPIRequest)
        self.txtbrAPIRequestFrame.setLayout(request_layout)

        response_layout = QVBoxLayout()
        response_layout.setContentsMargins(0, 0, 0, 0)
        response_layout.addWidget(self.txtbrAPIResponse)
        self.txtbrAPIResponseFrame.setLayout(response_layout)
