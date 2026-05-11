# The following has been generated automatically from src/gui/processing/qgsprocessingalgorithmwidgetbase.h
# monkey patching scoped based enum
QgsProcessingAlgorithmWidgetBase.LogFormat.FormatPlainText.__doc__ = "Plain text file (.txt)"
QgsProcessingAlgorithmWidgetBase.LogFormat.FormatHtml.__doc__ = "HTML file (.html)"
QgsProcessingAlgorithmWidgetBase.LogFormat.__doc__ = """Log format options.

.. versionadded:: 3.2

* ``FormatPlainText``: Plain text file (.txt)
* ``FormatHtml``: HTML file (.html)

"""
# --
# monkey patching scoped based enum
QgsProcessingAlgorithmWidgetBase.WidgetMode.Single.__doc__ = "Single algorithm execution mode"
QgsProcessingAlgorithmWidgetBase.WidgetMode.Batch.__doc__ = "Batch processing mode"
QgsProcessingAlgorithmWidgetBase.WidgetMode.__doc__ = """Widget modes.

.. versionadded:: 3.24

* ``Single``: Single algorithm execution mode
* ``Batch``: Batch processing mode

"""
# --
try:
    QgsProcessingAlgorithmWidgetBase.__attribute_docs__ = {'algorithmAboutToRun': 'Emitted when the algorithm is about to run in the specified ``context``.\n\nThis signal can be used to tweak the ``context`` prior to the algorithm\nexecution.\n\n.. versionadded:: 3.38\n', 'algorithmFinished': 'Emitted whenever an algorithm has finished executing in the widget.\n\n.. versionadded:: 3.14\n'}
    QgsProcessingAlgorithmWidgetBase.formatStringForLog = staticmethod(QgsProcessingAlgorithmWidgetBase.formatStringForLog)
    QgsProcessingAlgorithmWidgetBase.__virtual_methods__ = ['setParameters', 'isRunning', 'resetAdditionalGui', 'blockAdditionalControlsWhileRunning', 'isFinalized', 'finished', 'runAlgorithm', 'algExecuted']
    QgsProcessingAlgorithmWidgetBase.__overridden_methods__ = ['reject', 'closeEvent']
    QgsProcessingAlgorithmWidgetBase.__signal_arguments__ = {'algorithmAboutToRun': ['context: QgsProcessingContext'], 'algorithmFinished': ['successful: bool', 'result: Dict[str, object]']}
    QgsProcessingAlgorithmWidgetBase.__group__ = ['processing']
except (NameError, AttributeError):
    pass
