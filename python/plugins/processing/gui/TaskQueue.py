"""
***************************************************************************
    TaskQueue.py
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


class QueuedTask:
    """Represents a single queued processing task."""

    def __init__(self, algorithm_id, parameters, description=""):
        self.algorithm_id = algorithm_id
        self.parameters = parameters
        self.description = description


class ProcessingTaskQueue:
    """
    Singleton manager for a queue of processing tasks to be executed sequentially.

    This allows users to queue up different processing algorithms and execute them
    one after another, avoiding concurrent execution that could overload system resources.
    """

    _instance = None

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super().__new__(cls)
            cls._instance._initialized = False
        return cls._instance

    def __init__(self):
        if self._initialized:
            return
        self._initialized = True
        self._queue = []
        self._callbacks = []

    @classmethod
    def instance(cls):
        """Returns the singleton instance of the task queue."""
        return cls()

    def add_task(self, algorithm_id, parameters, description=""):
        """
        Adds a task to the queue.

        :param algorithm_id: The ID of the algorithm to execute
        :param parameters: The parameters to pass to the algorithm
        :param description: Optional description for the task
        """
        task = QueuedTask(algorithm_id, parameters, description)
        self._queue.append(task)
        self._notify_changed()

    def remove_task(self, index):
        """
        Removes a task at the specified index.

        :param index: The index of the task to remove
        :returns: True if the task was successfully removed
        """
        if 0 <= index < len(self._queue):
            self._queue.pop(index)
            self._notify_changed()
            return True
        return False

    def move_task_up(self, index):
        """
        Moves a task up in the queue.

        :param index: The index of the task to move
        :returns: True if the task was successfully moved
        """
        if 1 <= index < len(self._queue):
            self._queue[index], self._queue[index - 1] = (
                self._queue[index - 1],
                self._queue[index],
            )
            self._notify_changed()
            return True
        return False

    def move_task_down(self, index):
        """
        Moves a task down in the queue.

        :param index: The index of the task to move
        :returns: True if the task was successfully moved
        """
        if 0 <= index < len(self._queue) - 1:
            self._queue[index], self._queue[index + 1] = (
                self._queue[index + 1],
                self._queue[index],
            )
            self._notify_changed()
            return True
        return False

    def clear(self):
        """Clears all tasks from the queue."""
        self._queue.clear()
        self._notify_changed()

    def get_tasks(self):
        """Returns a copy of all tasks in the queue."""
        return list(self._queue)

    def count(self):
        """Returns the number of tasks in the queue."""
        return len(self._queue)

    def is_empty(self):
        """Returns True if the queue is empty."""
        return len(self._queue) == 0

    def register_callback(self, callback):
        """
        Registers a callback to be called when the queue changes.

        :param callback: A callable that will be invoked when the queue changes
        """
        if callback not in self._callbacks:
            self._callbacks.append(callback)

    def unregister_callback(self, callback):
        """
        Unregisters a callback.

        :param callback: The callback to remove
        """
        if callback in self._callbacks:
            self._callbacks.remove(callback)

    def _notify_changed(self):
        """Notifies all registered callbacks that the queue has changed."""
        for callback in self._callbacks:
            try:
                callback()
            except Exception:
                pass  # Ignore callback errors
