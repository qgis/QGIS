# The following has been generated automatically from src/gui/qgsfilewidget.h
QgsFileWidget.GetFile = QgsFileWidget.StorageMode.GetFile
QgsFileWidget.GetDirectory = QgsFileWidget.StorageMode.GetDirectory
QgsFileWidget.GetMultipleFiles = QgsFileWidget.StorageMode.GetMultipleFiles
QgsFileWidget.SaveFile = QgsFileWidget.StorageMode.SaveFile
QgsFileWidget.StorageMode.baseClass = QgsFileWidget
QgsFileWidget.Absolute = QgsFileWidget.RelativeStorage.Absolute
QgsFileWidget.RelativeProject = QgsFileWidget.RelativeStorage.RelativeProject
QgsFileWidget.RelativeDefaultPath = QgsFileWidget.RelativeStorage.RelativeDefaultPath
QgsFileWidget.RelativeStorage.baseClass = QgsFileWidget
try:
    QgsFileWidget.__attribute_docs__ = {'fileChanged': 'Emitted whenever the current file or directory ``path`` is changed.\n'}
    QgsFileWidget.splitFilePaths = staticmethod(QgsFileWidget.splitFilePaths)
    QgsFileWidget.isMultiFiles = staticmethod(QgsFileWidget.isMultiFiles)
    QgsFileWidget.__signal_arguments__ = {'fileChanged': ['path: str']}
except (NameError, AttributeError):
    pass
