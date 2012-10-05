import os

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from sextante.__init__ import version
from ui_aboutdialogbase import Ui_DlgAbout

import sextante.resources_rc

class AboutDialog(QDialog, Ui_DlgAbout):

    def __init__(self):
        QDialog.__init__(self)
        self.setupUi(self)

        self.buttonBox.helpRequested.connect(self.openHelp)

        self.setAboutText()

    def setAboutText(self):
        strAbout = self.tr("""
            <img src="qrc:/sextante/images/sextante_logo.png" />
            <h2>SEXTANTE for QGIS</h2>
            <p>SEXTANTE, a geoprocessing platform for QGIS</p>
            <p>A development by Victor Olaya (volayaf@gmail.com).</p>
            <p>Portions of this software contributed by:
            <ul>
            <li>Alexander Bruy</li>
            <li>Carson Farmer (fTools algorithms)</li>
            <li>Julien Malik (Orfeo Toolbox connectors)</li>
            <li>Evgeniy Nikulin (Original Field Pyculator code)</li>
            <li>Michael Nimm (mmqgis algorithms)</li>
            <li>Camilo Polymeris (Threading). Developed as part of Google
            Summer of Code 2012</li>
            </ul>
            </p>
            <p>You are currently using SEXTANTE v%1</p>
            <p>This software is distributed under the terms of the GNU GPL License v2.
            <p>For more information, please visit our website at
            <a href="http://sextantegis.com">http://sextantegis.com</a></p>
            """).arg(version())
        self.webView.setHtml(strAbout)

    def openHelp(self):
        QDesktopServices.openUrl(QUrl(os.path.join(os.path.dirname(__file__), os.path.pardir) + "/help/index.html"))
