# The following has been generated automatically from src/core/qgsabstractcontentcache.h
try:
    QgsAbstractContentCacheEntry.__attribute_docs__ = {'path': 'Represents the absolute path to a file, a remote URL, or a base64 encoded string.', 'fileModified': 'Timestamp when file was last modified', 'fileModifiedLastCheckTimer': 'Time since last check of file modified date', 'mFileModifiedCheckTimeout': 'Timeout before re-checking whether the file modified date has changed.', 'nextEntry': 'Entries are kept on a linked list, sorted by last access. This point refers\nto the next entry in the cache.', 'previousEntry': 'Entries are kept on a linked list, sorted by last access. This point refers\nto the previous entry in the cache.'}
    QgsAbstractContentCacheEntry.__annotations__ = {'path': str, 'fileModified': 'QDateTime', 'fileModifiedLastCheckTimer': 'QElapsedTimer', 'mFileModifiedCheckTimeout': int, 'nextEntry': 'QgsAbstractContentCacheEntry', 'previousEntry': 'QgsAbstractContentCacheEntry'}
    QgsAbstractContentCacheEntry.__abstract_methods__ = ['dataSize', 'dump', 'isEqual']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractContentCacheBase.__attribute_docs__ = {'remoteContentFetched': 'Emitted when the cache has finished retrieving content from a remote\n``url``.\n'}
    QgsAbstractContentCacheBase.parseBase64DataUrl = staticmethod(QgsAbstractContentCacheBase.parseBase64DataUrl)
    QgsAbstractContentCacheBase.parseEmbeddedStringData = staticmethod(QgsAbstractContentCacheBase.parseEmbeddedStringData)
    QgsAbstractContentCacheBase.isBase64Data = staticmethod(QgsAbstractContentCacheBase.isBase64Data)
    QgsAbstractContentCacheBase.__virtual_methods__ = ['invalidateCacheEntry', 'checkReply', 'onRemoteContentFetched']
    QgsAbstractContentCacheBase.__signal_arguments__ = {'remoteContentFetched': ['url: str']}
except (NameError, AttributeError):
    pass
