"""
allows multiple selection in a large list

Contact : marco@opengis.ch

.. note:: This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.
"""

__author__ = 'marco@opengis.ch'
__revision__ = '$Format:%H$'
__date__ = '9/07/2013'

from qgis.PyQt.QtWidgets import (QGroupBox,
                                 QListWidget,
                                 QPushButton,
                                 QSizePolicy,
                                 QLabel,
                                 QHBoxLayout,
                                 QVBoxLayout,
                                 QListWidget,
                                 QAbstractItemView)
from qgis.PyQt.QtGui import QFont
from qgis.PyQt.QtCore import pyqtSignal


class ListMultiSelectWidget(QGroupBox):

    """Widget to show two parallel lists and move elements between the two

    usage from code:
        self.myWidget = ListMultiSelectWidget(title='myTitle')
        self.myLayout.insertWidget(1, self.myWidget)
    usage from designer:
        insert a QGroupBox in your UI file
        optionally give a title to the QGroupBox
        promote it to ListMultiSelectWidget
    """

    selection_changed = pyqtSignal()

    def __init__(self, parent=None, title=None):
        QGroupBox.__init__(self)
        self.setTitle(title)

        self.selected_widget = None
        self.unselected_widget = None
        self._setupUI()

        # connect actions
        self.select_all_btn.clicked.connect(self._select_all)
        self.deselect_all_btn.clicked.connect(self._deselect_all)
        self.select_btn.clicked.connect(self._select)
        self.deselect_btn.clicked.connect(self._deselect)

        self.unselected_widget.itemDoubleClicked.connect(self._select)
        self.selected_widget.itemDoubleClicked.connect(self._deselect)

    def get_selected_items(self):
        """
        :return list with all the selected items text
        """
        return self._get_items(self.selected_widget)

    def get_unselected_items(self):
        """
        :return list with all the unselected items text
        """
        return self._get_items(self.unselected_widget)

    def add_selected_items(self, items):
        """
        :param items list of strings to be added in the selected list
        """
        self._add_items(self.selected_widget, items)

    def add_unselected_items(self, items):
        """
        :param items list of strings to be added in the unselected list
        """
        self._add_items(self.unselected_widget, items)

    def set_selected_items(self, items):
        """
        :param items list of strings to be set as the selected list
        """
        self._set_items(self.selected_widget, items)

    def set_unselected_items(self, items):
        """
        :param items list of strings to be set as the unselected list
        """
        self._set_items(self.unselected_widget, items)

    def clear(self):
        """
        removes all items from selected and unselected
        """
        self.set_selected_items([])
        self.set_unselected_items([])

    def addItem(self, item):
        """
        This is for Processing
        :param item: string to be added in the unselected list
        """
        self.add_unselected_items([item])

    def addItems(self, items):
        """
        This is for Processing
        :param items: list of strings to be added in the unselected list
        """
        self.add_unselected_items(items)

    def _get_items(self, widget):
        for i in range(widget.count()):
            yield widget.item(i).text()

    def _set_items(self, widget, items):
        widget.clear()
        self._add_items(widget, items)

    def _add_items(self, widget, items):
        widget.addItems(items)

    def _select_all(self):
        self.unselected_widget.selectAll()
        self._do_move(self.unselected_widget, self.selected_widget)

    def _deselect_all(self):
        self.selected_widget.selectAll()
        self._do_move(self.selected_widget, self.unselected_widget)

    def _select(self):
        self._do_move(self.unselected_widget, self.selected_widget)

    def _deselect(self):
        self._do_move(self.selected_widget, self.unselected_widget)

    def _do_move(self, fromList, toList):
        for item in fromList.selectedItems():
            prev_from_item = fromList.item(fromList.row(item) - 1)
            toList.addItem(fromList.takeItem(fromList.row(item)))
            fromList.scrollToItem(prev_from_item)
        self.selection_changed.emit()

    def _setupUI(self):
        self.setSizePolicy(
            QSizePolicy.Preferred, QSizePolicy.Ignored)

        self.setMinimumHeight(180)

        self.main_horizontal_layout = QHBoxLayout(self)

        italic_font = QFont()
        italic_font.setItalic(True)

        # unselected widget
        self.unselected_widget = QListWidget(self)
        self._set_list_widget_defaults(self.unselected_widget)
        unselected_label = QLabel()
        unselected_label.setText('Unselected')
        unselected_label.setAlignment(Qt.Qt.AlignCenter)
        unselected_label.setFont(italic_font)
        unselected_v_layout = QVBoxLayout()
        unselected_v_layout.addWidget(unselected_label)
        unselected_v_layout.addWidget(self.unselected_widget)

        # selected widget
        self.selected_widget = QListWidget(self)
        self._set_list_widget_defaults(self.selected_widget)
        selected_label = QLabel()
        selected_label.setText('Selected')
        selected_label.setAlignment(Qt.Qt.AlignCenter)
        selected_label.setFont(italic_font)
        selected_v_layout = QVBoxLayout()
        selected_v_layout.addWidget(selected_label)
        selected_v_layout.addWidget(self.selected_widget)

        # buttons
        self.buttons_vertical_layout = QVBoxLayout()
        self.buttons_vertical_layout.setContentsMargins(0, -1, 0, -1)

        self.select_all_btn = SmallQPushButton('>>')
        self.deselect_all_btn = SmallQPushButton('<<')
        self.select_btn = SmallQPushButton('>')
        self.deselect_btn = SmallQPushButton('<')
        self.select_btn.setToolTip('Add the selected items')
        self.deselect_btn.setToolTip('Remove the selected items')
        self.select_all_btn.setToolTip('Add all')
        self.deselect_all_btn.setToolTip('Remove all')

        # add buttons
        spacer_label = QLabel()  # pragmatic way to create a spacer with
        # the same height of the labels on top
        # of the lists, in order to align the
        # buttons with the lists.
        self.buttons_vertical_layout.addWidget(spacer_label)
        self.buttons_vertical_layout.addWidget(self.select_btn)
        self.buttons_vertical_layout.addWidget(self.deselect_btn)
        self.buttons_vertical_layout.addWidget(self.select_all_btn)
        self.buttons_vertical_layout.addWidget(self.deselect_all_btn)

        # add sub widgets
        self.main_horizontal_layout.addLayout(unselected_v_layout)
        self.main_horizontal_layout.addLayout(self.buttons_vertical_layout)
        self.main_horizontal_layout.addLayout(selected_v_layout)

    def _set_list_widget_defaults(self, widget):
        widget.setAlternatingRowColors(True)
        widget.setSortingEnabled(True)
        widget.setDragEnabled(True)
        widget.setDragDropMode(QAbstractItemView.DragDrop)
        widget.setDragDropOverwriteMode(False)
        widget.setDefaultDropAction(QtCore.Qt.MoveAction)
        widget.setSelectionMode(QAbstractItemView.MultiSelection)


class SmallQPushButton(QPushButton):

    def __init__(self, text):
        QPushButton.__init__(self)
        self.setText(text)
        buttons_size_policy = QSizePolicy(
            QSizePolicy.Fixed, QSizePolicy.Fixed)
        self.setSizePolicy(buttons_size_policy)
        self.setMaximumSize(QtCore.QSize(30, 30))
