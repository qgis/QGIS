# The following has been generated automatically from src/core/qgsabstractcontentcache.h
try:
    QgsAbstractContentCacheEntry.__attribute_docs__ = {'path': 'Represents the absolute path to a file, a remote URL, or a base64 encoded string.', 'fileModified': 'Timestamp when file was last modified', 'fileModifiedLastCheckTimer': 'Time since last check of file modified date', 'mFileModifiedCheckTimeout': 'Timeout before re-checking whether the file modified date has changed.', 'nextEntry': 'Entries are kept on a linked list, sorted by last access. This point refers\nto the next entry in the cache.', 'previousEntry': 'Entries are kept on a linked list, sorted by last access. This point refers\nto the previous entry in the cache.'}
except (NameError, AttributeError):
    pass
try:
    QgsAbstractContentCacheBase.__attribute_docs__ = {'remoteContentFetched': 'Emitted when the cache has finished retrieving content from a remote ``url``.\n'}
    QgsAbstractContentCacheBase.parseBase64DataUrl = staticmethod(QgsAbstractContentCacheBase.parseBase64DataUrl)
    QgsAbstractContentCacheBase.isBase64Data = staticmethod(QgsAbstractContentCacheBase.isBase64Data)
    QgsAbstractContentCacheBase.__signal_arguments__ = {'remoteContentFetched': ['url: str']}
except (NameError, AttributeError):
    pass
