# -*- coding: utf-8 -*-
# licensed under the terms of GNU GPL 2
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *
import webbrowser, os
from ui_frmAbout import Ui_Dialog
import resources_rc
currentPath = os.path.dirname(__file__)

class Dialog(QDialog, Ui_Dialog):
	def __init__(self, iface):
		QDialog.__init__(self)
		self.iface = iface
		# Set up the user interface from Designer.
		self.setupUi(self)
		QObject.connect(self.btnWeb, SIGNAL("clicked()"), self.openWeb)
		QObject.connect(self.btnHelp, SIGNAL("clicked()"), self.openHelp)
		self.fToolsLogo.setPixmap(QPixmap(":/icons/default/ftools_logo.png"))
		self.label_3.setText("fTools 0.5.10")
		self.textEdit.setText(self.getText())

	def getText(self):
		return self.tr("""
The goal of fTools is to provide a one-stop resource for many common vector-based GIS tasks, without the need for additional software, libraries, or complex workarounds.

fTools is designed to extend the functionality of Quantum GIS using only core QGIS and python libraries. It provides a growing suite of spatial data management and analysis functions that are both quick and functional. In addition, the geoprocessing functions of  Dr. Horst Duester and Stefan Ziegler have been incorporated to futher facilitate and streamline GIS based research and analysis.

If you would like to report a bug, make suggestions for improving fTools, or have a question about the tools, please email me: carson.farmer@gmail.com

LICENSING INFORMATION:
fTools is copyright (C) 2009  Carson J.Q. Farmer
Geoprocessing functions adapted from 'Geoprocessing Plugin',
(C) 2008 by Dr. Horst Duester, Stefan Ziegler

licensed under the terms of GNU GPL 2
This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

fTools DEVELOPERS:
Carson J. Q. Farmer
Alexander Bruy
**If you have contributed code to fTools and I haven't mentioned your name here, please contact me and I will add your name.

ACKNOWLEDGEMENTS:
The following individuals (whether they know it or not) have contributed ideas, help, testing, code, and guidence towards this project, and I thank them.
Hawthorn Beyer
Borys Jurgiel
Tim Sutton
Barry Rowlingson
Horst Duester and Stefan Ziegler
Paolo Cavallini
Aaron Racicot
Colin Robertson
Agustin Lobo
Jurgen E. Fischer
QGis developer and user communities
Folks on #qgis at freenode.net
All those who have reported bugs/fixes/suggestions/comments/etc.
""")

	def openWeb(self):
		webbrowser.open("http://www.ftools.ca/")

	def openHelp(self):
		webbrowser.open(currentPath + "/ftools_help.xml")
