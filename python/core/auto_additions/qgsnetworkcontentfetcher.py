# The following has been generated automatically from src/core/network/qgsnetworkcontentfetcher.h
try:
    QgsNetworkContentFetcher.__attribute_docs__ = {'finished': 'Emitted when content has loaded\n', 'downloadProgress': 'Emitted when data is received.\n\n.. versionadded:: 3.2\n', 'errorOccurred': 'Emitted when an error with ``code`` error occurred while processing the request\n``errorMsg`` is a textual description of the error\n\n.. versionadded:: 3.22\n'}
    QgsNetworkContentFetcher.__signal_arguments__ = {'downloadProgress': ['bytesReceived: int', 'bytesTotal: int'], 'errorOccurred': ['code: QNetworkReply.NetworkError', 'errorMsg: str']}
    QgsNetworkContentFetcher.__group__ = ['network']
except (NameError, AttributeError):
    pass
