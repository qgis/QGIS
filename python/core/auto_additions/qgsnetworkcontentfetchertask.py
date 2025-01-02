# The following has been generated automatically from src/core/network/qgsnetworkcontentfetchertask.h
try:
    QgsNetworkContentFetcherTask.__attribute_docs__ = {'fetched': 'Emitted when the network content has been fetched, regardless\nof whether the fetch was successful or not.\n\nUsers of QgsNetworkContentFetcherTask should connect to this signal,\nand from the associated slot they can then safely access the network :py:func:`~QgsNetworkContentFetcherTask.reply`\nwithout danger of the task being first removed by the :py:class:`QgsTaskManager`.\n', 'errorOccurred': 'Emitted when an error with ``code`` error occurred while processing the request\n``errorMsg`` is a textual description of the error\n\n.. versionadded:: 3.22\n'}
    QgsNetworkContentFetcherTask.__signal_arguments__ = {'errorOccurred': ['code: QNetworkReply.NetworkError', 'errorMsg: str']}
    QgsNetworkContentFetcherTask.__group__ = ['network']
except (NameError, AttributeError):
    pass
