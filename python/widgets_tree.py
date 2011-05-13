#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

"""
Reads .ui files from ../src/ui/ directory and write to stdout an XML describing
widgets tree.

Python bindings must be compiled and in PYTHONPATH

QGIS libraries must be in LD_LIBRARY_PATH

Output should go to ../resources/customization.xml

"""

import sys
import os, glob, imp
from PyQt4.QtGui import *
from PyQt4.uic import loadUi, compileUi
from xml.etree.ElementTree import Element, SubElement, Comment, tostring
from xml.dom import minidom

# qwt_plot is missing somehow but it may depend on installed packages
from PyQt4 import Qwt5 as qwt_plot
sys.modules['qwt_plot'] = qwt_plot

# loadUi is looking for custom widget in module which is lowercase version of 
# the class, which do not exist (AFAIK) -> preload them, problems anyway:
# missing in gui: QgsColorRampComboBox, QgsRendererRulesTreeWidget,
# QgsRendererRulesTreeWidget, QgsAttributeTableView
# and QgsProjectionSelector cannot open db file
from qgis import gui
for m in ['qgscolorbutton', 'qgscolorrampcombobox', 'qgsprojectionselector', 'qgslabelpreview', 'qgsrulebasedrendererv2widget', 'qgsattributetableview' ]:
	sys.modules[m] = gui

class UiInspector:
	def __init__(self ):
		self.ui_dir = os.path.abspath ( os.path.join(  os.path.dirname(__file__)	, '../src/ui/*.ui' ) )
		self.printMsg ( "Loading UI files " + self.ui_dir )
		# list of widget classes we want to follow 
		self.follow = [
			QWidget, QDialog, 
			QCheckBox, QComboBox, QDial, QPushButton, QLabel, QLCDNumber, QLineEdit, QRadioButton, QScrollBar, QSlider, QSpinBox, QTextEdit,
			QDateEdit, QTimeEdit, QDateTimeEdit, QListView, QProgressBar, QTableView, QTabWidget, QTextBrowser, QDialogButtonBox,
      QScrollArea, QGroupBox, QStackedWidget,
		]

	def printMsg ( self, msg ):
		sys.stderr.write( msg + "\n" ) 

	def widgetXml(self, element, widget, level = 0, label = None ):
		#print tostring ( element )
		#self.printMsg ( "class: " + str( type ( widget ) ) )
		#self.printMsg ( "objectName: " + widget.objectName() )
		#self.printMsg ( "windowTitle: " + widget.windowTitle() )

		if not widget.objectName(): return
	
		lab = label
		if hasattr( widget, 'text' ):
			lab = widget.text()
		if widget.windowTitle():
			label = widget.windowTitle()
		if not lab:
			lab = ''

		lab = unicode(lab).encode("ascii","replace")

		sub_element = SubElement( element, 'widget')
		sub_element.set('class', widget.__class__.__name__ )
		sub_element.set('objectName', widget.objectName() )
		sub_element.set('label', lab )

		#print str ( widget.children () )
		# tab widget label is stored in QTabWidget->QTabBarPrivate->tabList->QTab ..
		if type(widget) in [ QTabWidget ]:
			children = list ( { 'widget': widget.widget(i), 'label':  widget.tabText(i) } for i in range ( 0, widget.count() ) )
		else:
			children = list ( { 'widget': c, 'label': None } for c in widget.children () )
		for child in children:
			w  = child['widget']
			if w.isWidgetType() and ( type(w) in self.follow ):
				self.widgetXml ( sub_element, w, level+1, child['label'] )
		

	def treeXml(self, element ):
		xml = ''
    # debug
		for p in glob.glob( self.ui_dir ):
		#for p in ['/home/radim/devel/qgis_trunk/src/ui/qgsabout.ui']: 
		#for p in ['/home/radim/devel/qgis_trunk/src/ui/qgsrasterlayerpropertiesbase.ui']: 
			self.printMsg ( "Loading " + p )
			# qgsrasterlayerpropertiesbase.ui is giving: No module named qwt_plot
			try:
				widget = loadUi ( p )
				#print dir ( ui )
				self.widgetXml ( element, widget )
			except Exception, e:
			#except IOError, e:
				self.printMsg ( str(e) )

		return xml

	def xml( self ) :
		#xml = "<?xml version='1.0' encoding='UTF-8'?>\n"
		#xml += "<!DOCTYPE qgiswidgets SYSTEM 'http://mrcc.com/qgiswidgets.dtd'>\n"
		element = Element('qgiswidgets')
		self.treeXml( element )

		string =  tostring ( element, 'utf-8' )
		reparsed = minidom.parseString(string)
		xml = reparsed.toprettyxml(indent="  ")
		return xml
		

if __name__ == '__main__':
	app = QApplication(sys.argv) # required by loadUi
	inspector = UiInspector()
	xml = inspector.xml()
	sys.stdout.write( xml )
	sys.stdout.flush()

	del app
	sys.exit(0)
