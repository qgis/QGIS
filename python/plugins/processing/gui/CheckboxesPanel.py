"""
***************************************************************************
    CheckBoxesPanel.py
    ---------------------
    Date                 : January 2015
    Copyright            : (C) 2015 by Arnaud Morvan
    Email                : arnaud dot morvan at camptocamp dot com
    Contributors         : Arnaud Morvan
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Arnaud Morvan"
__date__ = "January 2015"
__copyright__ = "(C) 2015, Arnaud Morvan"

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtWidgets import (
    QCheckBox,
    QRadioButton,
    QGridLayout,
    QButtonGroup,
    QSizePolicy,
    QSpacerItem,
    QWidget,
    QMenu,
    QAction,
)
from qgis.PyQt.QtGui import QCursor


class CheckboxesPanel(QWidget):

    def __init__(self, options, multiple, columns=2, parent=None):
        super().__init__(parent)

        self._options = []
        for i, option in enumerate(options):
            if isinstance(option, str):
                self._options.append((i, option))
            else:
                self.options.append(option)
        self._multiple = multiple
        self._buttons = []
        rows = len(options) / columns

        self._buttonGroup = QButtonGroup()
        self._buttonGroup.setExclusive(not multiple)
        layout = QGridLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setMargin(0)
        for i, (v, t) in enumerate(self._options):
            if multiple:
                button = QCheckBox(t)
            else:
                button = QRadioButton(t)
            self._buttons.append((v, button))
            self._buttonGroup.addButton(button, i)
            layout.addWidget(button, i % rows, i / rows)
        layout.addItem(
            QSpacerItem(0, 0, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum),
            0,
            columns,
        )
        self.setLayout(layout)

        if multiple:
            self.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
            self.customContextMenuRequested.connect(self.showPopupMenu)

    def showPopupMenu(self):
        popup_menu = QMenu()
        select_all_action = QAction(self.tr("Select All"), popup_menu)
        select_all_action.triggered.connect(self.selectAll)
        clear_all_action = QAction(self.tr("Clear Selection"), popup_menu)
        clear_all_action.triggered.connect(self.deselectAll)
        popup_menu.addAction(select_all_action)
        popup_menu.addAction(clear_all_action)
        popup_menu.exec(QCursor.pos())

    def selectAll(self):
        for v, button in self._buttons:
            button.setChecked(True)

    def deselectAll(self):
        for v, button in self._buttons:
            button.setChecked(False)

    def value(self):
        if self._multiple:
            return [v for (v, checkbox) in self._buttons if checkbox.isChecked()]
        else:
            return self._options[self._buttonGroup.checkedId()][0]

    def setValue(self, value):
        for v, button in self._buttons:
            if self._multiple:
                button.setChecked(v in value)
            else:
                button.setChecked(v == value)
