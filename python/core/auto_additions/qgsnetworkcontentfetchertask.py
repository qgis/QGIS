# The following has been generated automatically from src/core/network/qgsnetworkcontentfetchertask.h
try:
    QgsNetworkContentFetcherTask.__attribute_docs__ = {'fetched': 'Emitted when the network content has been fetched, regardless of whether\nthe fetch was successful or not.\n\nUsers of QgsNetworkContentFetcherTask should connect to this signal, and\nfrom the associated slot they can then safely access the network\n:py:func:`~QgsNetworkContentFetcherTask.reply` without danger of the\ntask being first removed by the :py:class:`QgsTaskManager`.\n', 'errorOccurred': 'Emitted when an error with ``code`` error occurred while processing the\nrequest ``errorMsg`` is a textual description of the error\n\n.. versionadded:: 3.22\n'}
    QgsNetworkContentFetcherTask.__overridden_methods__ = ['run', 'cancel']
    QgsNetworkContentFetcherTask.__signal_arguments__ = {'errorOccurred': ['code: QNetworkReply.NetworkError', 'errorMsg: str']}
    QgsNetworkContentFetcherTask.__group__ = ['network']
except (NameError, AttributeError):
    pass
