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
		aboutText = QString("The goal of fTools is to provide a one-stop resource for many common vector-based GIS tasks, ")
		aboutText.append("without the need for additional software, libraries, or complex workarounds.\n\n")
		aboutText.append("fTools is designed to extend the functionality of Quantum GIS using only core QGIS and python ")
		aboutText.append("libraries. It provides a growing suite of spatial data management and analysis functions that are ")
		aboutText.append("both quick and functional. In addition, the geoprocessing functions of  Dr. Horst Duester and ")
		aboutText.append("Stefan Ziegler have been incorporated to futher facilitate and streamline GIS based research and analysis.\n\n")
		aboutText.append("If you would like to report a bug, make suggestions for improving fTools, or have a question about ")
		aboutText.append("the tools, please email me: carson.farmer@gmail.com\n\n")
		licenceString = QString("LICENSING INFORMATION:\n")
		licenceString.append("fTools is copyright (C) 2009  Carson J.Q. Farmer\n")
		licenceString.append("Geoprocessing functions adapted from 'Geoprocessing Plugin',\n")
		licenceString.append("(C) 2008 by Dr. Horst Duester, Stefan Ziegler\n\n")
		licenceString.append("licensed under the terms of GNU GPL 2\n")
		licenceString.append("This program is free software; you can redistribute it and/or modify")
		licenceString.append("it under the terms of the GNU General Public License as published by")
		licenceString.append("the Free Software Foundation; either version 2 of the License, or")
		licenceString.append("(at your option) any later version.\n")
		licenceString.append("This program is distributed in the hope that it will be useful,")
		licenceString.append("but WITHOUT ANY WARRANTY; without even the implied warranty of")
		licenceString.append("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the")
		licenceString.append("GNU General Public License for more details.\n")
		licenceString.append("You should have received a copy of the GNU General Public License along")
		licenceString.append("with this program; if not, write to the Free Software Foundation, Inc.,")
		licenceString.append("51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.\n\n")
		aknowledgeString = QString("AKNOWLEDGEMENTS:\n")
		aknowledgeString.append("The following individuals (whether they know it or not) have contributed ")
		aknowledgeString.append("ideas, help, testing, code, and guidence towards this project, and I thank them.\n")
		aknowledgeString.append("Hawthorn Beyer\n")
		aknowledgeString.append("Borys Jurgiel\n")
		aknowledgeString.append("Tim Sutton\n")
		aknowledgeString.append("Barry Rowlingson\n")
		aknowledgeString.append("Horst Duester and Stefan Ziegler\n")
		aknowledgeString.append("Paolo Cavallini\n")
		aknowledgeString.append("Aaron Racicot\n")
		aknowledgeString.append("Colin Robertson\n")
		aknowledgeString.append("Agustin Lobo\n")		
		aknowledgeString.append("Jurgen E. Fischer\n")
		aknowledgeString.append("QGis developer and user communities\n")
		aknowledgeString.append("Folks on #qgis at freenode.net\n")
		aknowledgeString.append("All those who have reported bugs/fixes/suggestions/comments/etc.")
		return QString(aboutText.append(licenceString.append(aknowledgeString)))

	def openWeb(self):
		webbrowser.open("http://www.ftools.ca/")

	def openHelp(self):
		webbrowser.open(currentPath + "/ftools_help.xml")
		
