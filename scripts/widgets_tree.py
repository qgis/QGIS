#!/usr/bin/env python3

"""
***************************************************************************
    widgets_tree.py
    ---------------------
    Date                 : May 2011
    Copyright            : (C) 2011 by Martin Dobias
    Email                : wonder dot sk at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Martin Dobias"
__date__ = "May 2011"
__copyright__ = "(C) 2011, Martin Dobias"

"""
Reads .ui files from ../src/ui/ directory and write to stdout an XML describing
widgets tree.

Python bindings must be compiled and in PYTHONPATH

QGIS libraries must be in LD_LIBRARY_PATH

Output should go to ../resources/customization.xml

"""

import glob
import os
import sys

# qwt_plot is missing somehow but it may depend on installed packages
from qgis.PyQt import Qwt5 as qwt_plot
from qgis.PyQt.QtWidgets import (
    QCheckBox,
    QComboBox,
    QDateEdit,
    QDateTimeEdit,
    QDial,
    QDialog,
    QDialogButtonBox,
    QGroupBox,
    QLabel,
    QLCDNumber,
    QLineEdit,
    QListView,
    QProgressBar,
    QPushButton,
    QRadioButton,
    QScrollArea,
    QScrollBar,
    QSlider,
    QSpinBox,
    QStackedWidget,
    QTableView,
    QTabWidget,
    QTextBrowser,
    QTextEdit,
    QTimeEdit,
    QWidget,
)
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.uic import loadUi

sys.modules["qwt_plot"] = qwt_plot

# loadUi is looking for custom widget in module which is lowercase version of
# the class, which do not exist (AFAIK) -> preload them, problems anyway:
# missing in gui: QgsColorRampComboBox, QgsRendererRulesTreeWidget,
# QgsRendererRulesTreeWidget
# and QgsProjectionSelector cannot open db file
from qgis import gui

for m in [
    "qgscolorbutton",
    "qgscolorrampcombobox",
    "qgsprojectionselector",
    "qgslabelpreview",
    "qgsrulebasedrendererwidget",
    "qgscollapsiblegroupbox",
    "qgsblendmodecombobox",
    "qgsexpressionbuilderwidget",
    "qgsrasterformatsaveoptionswidget",
    "qgsrasterpyramidsoptionswidget",
    "qgsscalecombobox",
    "qgsfilterlineedit",
    "qgsdualview",
]:
    sys.modules[m] = gui


class UiInspector:

    def __init__(self):
        self.ui_dir = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../src/ui/*.ui")
        )
        self.printMsg("Loading UI files " + self.ui_dir)
        # list of widget classes we want to follow
        self.follow = [
            QWidget,
            QDialog,
            QCheckBox,
            QComboBox,
            QDial,
            QPushButton,
            QLabel,
            QLCDNumber,
            QLineEdit,
            QRadioButton,
            QScrollBar,
            QSlider,
            QSpinBox,
            QTextEdit,
            QDateEdit,
            QTimeEdit,
            QDateTimeEdit,
            QListView,
            QProgressBar,
            QTableView,
            QTabWidget,
            QTextBrowser,
            QDialogButtonBox,
            QScrollArea,
            QGroupBox,
            QStackedWidget,
        ]

    def printMsg(self, msg):
        sys.stderr.write(msg + "\n")

    def widgetXml(self, element, widget, level=0, label=None):
        # print tostring ( element )
        # self.printMsg ( "class: " + str( type ( widget ) ) )
        # self.printMsg ( "objectName: " + widget.objectName() )
        # self.printMsg ( "windowTitle: " + widget.windowTitle() )

        if not widget.objectName():
            return

        lab = label
        if hasattr(widget, "text"):
            lab = widget.text()
        if widget.windowTitle():
            label = widget.windowTitle()
        if not lab:
            lab = ""

        subElement = self.doc.createElement("widget")
        subElement.setAttribute("class", widget.__class__.__name__)
        subElement.setAttribute("objectName", widget.objectName())
        subElement.setAttribute("label", lab)
        element.appendChild(subElement)

        # print str ( widget.children () )
        # tab widget label is stored in QTabWidget->QTabBarPrivate->tabList->QTab ..
        if type(widget) in [QTabWidget]:
            children = list(
                {"widget": widget.widget(i), "label": widget.tabText(i)}
                for i in range(0, widget.count())
            )
        else:
            children = list({"widget": c, "label": None} for c in widget.children())
        for child in children:
            w = child["widget"]
            if w.isWidgetType() and (type(w) in self.follow):
                self.widgetXml(subElement, w, level + 1, child["label"])

    def xml(self):
        self.doc = QDomDocument()
        element = self.doc.createElement("qgiswidgets")
        self.doc.appendChild(element)
        for p in glob.glob(self.ui_dir):
            self.printMsg("Loading " + p)
            # qgsrasterlayerpropertiesbase.ui is giving: No module named qwt_plot
            try:
                widget = loadUi(p)
                # print dir ( ui )
                self.widgetXml(element, widget)
            except Exception as e:
                self.printMsg(str(e))

        return self.doc.toString(2)


if __name__ == "__main__":
    from qgis.PyQt.QtCore import QApplication

    app = QApplication(sys.argv)  # required by loadUi
    inspector = UiInspector()
    xml = inspector.xml()
    sys.stdout.write(xml)
    sys.stdout.flush()

    del app
    sys.exit(0)
