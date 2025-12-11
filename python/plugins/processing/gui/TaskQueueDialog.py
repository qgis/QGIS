"""
***************************************************************************
    TaskQueueDialog.py
    ---------------------
    Date                 : December 2024
    Copyright            : (C) 2024 by Nass Lanckmann
    Email                : nass dot lanckmann at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Nass Lanckmann"
__date__ = "December 2024"
__copyright__ = "(C) 2024, Nass Lanckmann"

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtWidgets import (
    QDialog,
    QVBoxLayout,
    QHBoxLayout,
    QTableWidget,
    QTableWidgetItem,
    QPushButton,
    QToolButton,
    QMessageBox,
    QHeaderView,
    QAbstractItemView,
    QDialogButtonBox,
)
from qgis.PyQt.QtGui import QIcon, QBrush, QColor
from qgis.core import QgsApplication, QgsProcessingRegistry

from processing.gui.TaskQueue import ProcessingTaskQueue
from processing.gui.BatchAlgorithmDialog import BatchAlgorithmDialog
from processing.tools import dataobjects


class TaskQueueDialog(QDialog):
    """Dialog for managing the processing task queue."""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.queue = ProcessingTaskQueue.instance()
        self.setupUi()
        self.queue.register_callback(self.refresh)
        self.refresh()

    def setupUi(self):
        self.setWindowTitle(self.tr("Processing Task Queue"))
        self.setMinimumSize(800, 400)

        layout = QVBoxLayout()

        self.table = QTableWidget()
        self.table.setColumnCount(3)
        self.table.setHorizontalHeaderLabels(
            [self.tr("#"), self.tr("Algorithm"), self.tr("Description")]
        )
        self.table.horizontalHeader().setStretchLastSection(True)
        self.table.setSelectionBehavior(QAbstractItemView.SelectionBehavior.SelectRows)
        self.table.setSelectionMode(QAbstractItemView.SelectionMode.SingleSelection)
        self.table.itemSelectionChanged.connect(self.updateButtons)
        layout.addWidget(self.table)

        buttonLayout = QHBoxLayout()

        self.removeButton = QToolButton()
        self.removeButton.setIcon(
            QgsApplication.getThemeIcon("/mActionDeleteSelected.svg")
        )
        self.removeButton.setToolTip(self.tr("Remove selected task"))
        self.removeButton.clicked.connect(self.removeSelected)
        buttonLayout.addWidget(self.removeButton)

        self.moveUpButton = QToolButton()
        self.moveUpButton.setIcon(QgsApplication.getThemeIcon("/mActionArrowUp.svg"))
        self.moveUpButton.setToolTip(self.tr("Move selected task up"))
        self.moveUpButton.clicked.connect(self.moveUp)
        buttonLayout.addWidget(self.moveUpButton)

        self.moveDownButton = QToolButton()
        self.moveDownButton.setIcon(
            QgsApplication.getThemeIcon("/mActionArrowDown.svg")
        )
        self.moveDownButton.setToolTip(self.tr("Move selected task down"))
        self.moveDownButton.clicked.connect(self.moveDown)
        buttonLayout.addWidget(self.moveDownButton)

        buttonLayout.addStretch()

        self.clearButton = QPushButton(self.tr("Clear Queue"))
        self.clearButton.clicked.connect(self.clearQueue)
        buttonLayout.addWidget(self.clearButton)

        self.executeButton = QPushButton(self.tr("Execute Queue"))
        self.executeButton.setDefault(True)
        self.executeButton.clicked.connect(self.executeQueue)
        buttonLayout.addWidget(self.executeButton)

        layout.addLayout(buttonLayout)

        closeButtonBox = QDialogButtonBox(QDialogButtonBox.StandardButton.Close)
        closeButtonBox.rejected.connect(self.reject)
        layout.addWidget(closeButtonBox)

        self.setLayout(layout)

    def refresh(self):
        """Refreshes the table to reflect the current queue state."""
        self.table.setRowCount(0)

        tasks = self.queue.get_tasks()
        for i, task in enumerate(tasks):
            self.table.insertRow(i)

            indexItem = QTableWidgetItem(str(i + 1))
            indexItem.setFlags(indexItem.flags() & ~Qt.ItemFlag.ItemIsEditable)
            self.table.setItem(i, 0, indexItem)

            alg = QgsApplication.processingRegistry().algorithmById(task.algorithm_id)
            algName = task.algorithm_id
            if alg:
                algName = alg.displayName()

            algItem = QTableWidgetItem(algName)
            algItem.setFlags(algItem.flags() & ~Qt.ItemFlag.ItemIsEditable)
            self.table.setItem(i, 1, algItem)

            descItem = QTableWidgetItem(task.description)
            descItem.setFlags(descItem.flags() & ~Qt.ItemFlag.ItemIsEditable)
            self.table.setItem(i, 2, descItem)

        self.table.resizeColumnsToContents()
        self.updateButtons()

    def removeSelected(self):
        """Removes the selected task from the queue."""
        selected = self.table.selectedItems()
        if not selected:
            return

        row = selected[0].row()
        self.queue.remove_task(row)

    def moveUp(self):
        """Moves the selected task up in the queue."""
        selected = self.table.selectedItems()
        if not selected:
            return

        row = selected[0].row()
        if self.queue.move_task_up(row):
            self.table.selectRow(row - 1)

    def moveDown(self):
        """Moves the selected task down in the queue."""
        selected = self.table.selectedItems()
        if not selected:
            return

        row = selected[0].row()
        if self.queue.move_task_down(row):
            self.table.selectRow(row + 1)

    def clearQueue(self):
        """Clears all tasks from the queue after confirmation."""
        if (
            QMessageBox.question(
                self,
                self.tr("Clear Queue"),
                self.tr("Are you sure you want to clear all tasks from the queue?"),
                QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
                QMessageBox.StandardButton.No,
            )
            == QMessageBox.StandardButton.Yes
        ):
            self.queue.clear()

    def executeQueue(self):
        """Executes all tasks in the queue sequentially."""
        if self.queue.is_empty():
            return

        from processing.gui.MessageBarProgress import MessageBarProgress

        self.tasks_to_execute = self.queue.get_tasks()
        self.current_task_index = 0
        self.task_results = []
        self.task_errors = []

        self.progress_feedback = MessageBarProgress()
        self.executeNextTask()

    def executeNextTask(self):
        """Executes the next task in the queue."""
        if self.current_task_index >= len(self.tasks_to_execute):
            self.onQueueExecutionComplete()
            return

        if self.progress_feedback and self.progress_feedback.isCanceled():
            self.onQueueExecutionComplete(canceled=True)
            return

        task = self.tasks_to_execute[self.current_task_index]
        alg = QgsApplication.processingRegistry().algorithmById(task.algorithm_id)

        if not alg:
            self.task_errors.append(
                self.tr("Algorithm '{}' not found").format(task.algorithm_id)
            )
            self.markTaskFailed(self.current_task_index)
            self.current_task_index += 1
            self.executeNextTask()
            return

        self.markTaskExecuting(self.current_task_index)

        if self.progress_feedback:
            self.progress_feedback.setProgressText(
                self.tr("Processing task {}/{}").format(
                    self.current_task_index + 1, len(self.tasks_to_execute)
                )
            )

        alg_instance = alg.create()
        context = dataobjects.createContext()

        from processing.gui.Postprocessing import handleAlgorithmResults

        try:
            ok, results = alg_instance.run(
                task.parameters, context, self.progress_feedback
            )
            if ok:
                self.task_results.append(results)
                self.markTaskCompleted(self.current_task_index)
                handleAlgorithmResults(alg_instance, context, self.progress_feedback)
            else:
                error_msg = self.tr("Algorithm execution failed")
                self.task_errors.append(error_msg)
                self.markTaskFailed(self.current_task_index)
        except Exception as e:
            self.task_errors.append(str(e))
            self.markTaskFailed(self.current_task_index)

        self.current_task_index += 1
        self.executeNextTask()

    def onQueueExecutionComplete(self, canceled=False):
        """Called when queue execution is complete."""
        if self.progress_feedback:
            self.progress_feedback.close()
            self.progress_feedback = None

        if canceled:
            QMessageBox.information(
                self,
                self.tr("Queue Execution Canceled"),
                self.tr("Queue execution was canceled by user."),
            )
        elif self.task_errors:
            QMessageBox.warning(
                self,
                self.tr("Queue Execution Complete with Errors"),
                self.tr(
                    "{} tasks completed successfully, {} failed.\n\nErrors:\n{}"
                ).format(
                    len(self.task_results),
                    len(self.task_errors),
                    "\n".join(self.task_errors[:5]),
                ),
            )
        else:
            QMessageBox.information(
                self,
                self.tr("Queue Execution Complete"),
                self.tr("All {} tasks completed successfully.").format(
                    len(self.task_results)
                ),
            )

    def markTaskExecuting(self, index):
        """Marks a task as currently executing."""
        if index < self.table.rowCount():
            for col in range(self.table.columnCount()):
                item = self.table.item(index, col)
                if item:
                    item.setBackground(QBrush(QColor(255, 255, 200)))

    def markTaskCompleted(self, index):
        """Marks a task as completed successfully."""
        if index < self.table.rowCount():
            for col in range(self.table.columnCount()):
                item = self.table.item(index, col)
                if item:
                    item.setBackground(QBrush(QColor(200, 255, 200)))

    def markTaskFailed(self, index):
        """Marks a task as failed."""
        if index < self.table.rowCount():
            for col in range(self.table.columnCount()):
                item = self.table.item(index, col)
                if item:
                    item.setBackground(QBrush(QColor(255, 200, 200)))

    def updateButtons(self):
        """Updates the enabled state of buttons based on selection and queue state."""
        hasQueue = not self.queue.is_empty()
        hasSelection = len(self.table.selectedItems()) > 0

        self.executeButton.setEnabled(hasQueue)
        self.clearButton.setEnabled(hasQueue)
        self.removeButton.setEnabled(hasSelection)
        self.moveUpButton.setEnabled(hasSelection)
        self.moveDownButton.setEnabled(hasSelection)

    def closeEvent(self, event):
        """Cleanup when dialog is closed."""
        self.queue.unregister_callback(self.refresh)
        super().closeEvent(event)
