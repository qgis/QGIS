# The following has been generated automatically from src/core/network/qgsfiledownloader.h
try:
    QgsFileDownloader.__attribute_docs__ = {'downloadCompleted': 'Emitted when the download has completed successfully\n', 'downloadExited': 'Emitted always when the downloader exits\n', 'downloadCanceled': 'Emitted when the download was canceled by the user.\n\n.. seealso:: :py:func:`cancelDownload`\n', 'downloadError': 'Emitted when an error makes the download fail\n', 'downloadProgress': 'Emitted when data are ready to be processed\n'}
    QgsFileDownloader.__signal_arguments__ = {'downloadCompleted': ['url: QUrl'], 'downloadError': ['errorMessages: List[str]'], 'downloadProgress': ['bytesReceived: int', 'bytesTotal: int']}
    QgsFileDownloader.__group__ = ['network']
except (NameError, AttributeError):
    pass
