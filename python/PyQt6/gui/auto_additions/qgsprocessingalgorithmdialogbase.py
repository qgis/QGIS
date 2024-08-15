# The following has been generated automatically from src/gui/processing/qgsprocessingalgorithmdialogbase.h
QgsProcessingAlgorithmDialogBase.FormatPlainText = QgsProcessingAlgorithmDialogBase.LogFormat.FormatPlainText
QgsProcessingAlgorithmDialogBase.FormatHtml = QgsProcessingAlgorithmDialogBase.LogFormat.FormatHtml
# monkey patching scoped based enum
QgsProcessingAlgorithmDialogBase.DialogMode.Single.__doc__ = "Single algorithm execution mode"
QgsProcessingAlgorithmDialogBase.DialogMode.Batch.__doc__ = "Batch processing mode"
QgsProcessingAlgorithmDialogBase.DialogMode.__doc__ = "Dialog modes.\n\n.. versionadded:: 3.24\n\n" + '* ``Single``: ' + QgsProcessingAlgorithmDialogBase.DialogMode.Single.__doc__ + '\n' + '* ``Batch``: ' + QgsProcessingAlgorithmDialogBase.DialogMode.Batch.__doc__
# --
try:
    QgsProcessingAlgorithmDialogBase.__attribute_docs__ = {'algorithmAboutToRun': 'Emitted when the algorithm is about to run in the specified ``context``.\n\nThis signal can be used to tweak the ``context`` prior to the algorithm execution.\n\n.. versionadded:: 3.38\n', 'algorithmFinished': 'Emitted whenever an algorithm has finished executing in the dialog.\n\n.. versionadded:: 3.14\n'}
except NameError:
    pass
QgsProcessingAlgorithmDialogBase.formatStringForLog = staticmethod(QgsProcessingAlgorithmDialogBase.formatStringForLog)
try:
    QgsProcessingAlgorithmDialogBase.__group__ = ['processing']
except NameError:
    pass
