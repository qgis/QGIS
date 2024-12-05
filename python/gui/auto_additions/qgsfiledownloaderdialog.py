# The following has been generated automatically from src/gui/qgsfiledownloaderdialog.h
try:
    QgsFileDownloaderDialog.__attribute_docs__ = {'downloadCompleted': 'Emitted when the download has completed successfully\n', 'downloadExited': 'Emitted always when the downloader exits\n', 'downloadCanceled': 'Emitted when the download was canceled by the user\n', 'downloadError': 'Emitted when an error makes the download fail\n', 'downloadProgress': 'Emitted when data are ready to be processed\n'}
    QgsFileDownloaderDialog.__signal_arguments__ = {'downloadError': ['errorMessages: List[str]'], 'downloadProgress': ['bytesReceived: int', 'bytesTotal: int']}
except (NameError, AttributeError):
    pass
