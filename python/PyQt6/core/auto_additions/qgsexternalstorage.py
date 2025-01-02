# The following has been generated automatically from src/core/externalstorage/qgsexternalstorage.h
try:
    QgsExternalStorageContent.__attribute_docs__ = {'errorOccurred': 'The signal is emitted when an error occurred. ``errorString`` is a textual description of the error\n', 'progressChanged': 'The signal is emitted whenever content fetching/storing estimated progression value ``progress`` has changed.\n``progress`` value is between 0 and 100.\n', 'canceled': 'The signal is emitted when content fetching/storing has been canceled\n'}
    QgsExternalStorageContent.__signal_arguments__ = {'errorOccurred': ['errorString: str'], 'progressChanged': ['progress: float']}
    QgsExternalStorageContent.__group__ = ['externalstorage']
except (NameError, AttributeError):
    pass
try:
    QgsExternalStorageFetchedContent.__attribute_docs__ = {'fetched': 'The signal is emitted when the resource has successfully been fetched\n'}
    QgsExternalStorageFetchedContent.__group__ = ['externalstorage']
except (NameError, AttributeError):
    pass
try:
    QgsExternalStorageStoredContent.__attribute_docs__ = {'stored': 'The signal is emitted when the resource has successfully been stored\n'}
    QgsExternalStorageStoredContent.__group__ = ['externalstorage']
except (NameError, AttributeError):
    pass
try:
    QgsExternalStorage.__group__ = ['externalstorage']
except (NameError, AttributeError):
    pass
