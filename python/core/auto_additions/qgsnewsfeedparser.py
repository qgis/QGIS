# The following has been generated automatically from src/core/network/qgsnewsfeedparser.h
try:
    QgsNewsFeedParser.Entry.__attribute_docs__ = {'key': 'Unique entry identifier', 'title': 'Entry title', 'imageUrl': 'Optional URL for image associated with entry', 'image': 'Optional image data', 'content': 'HTML content of news entry', 'link': 'Optional URL link for entry', 'sticky': '``True`` if entry is "sticky" and should always be shown at the top', 'expiry': 'Optional auto-expiry time for entry'}
    QgsNewsFeedParser.Entry.__group__ = ['network']
except (NameError, AttributeError):
    pass
try:
    QgsNewsFeedParser.__attribute_docs__ = {'fetched': 'Emitted when ``entries`` have been fetched from the feed.\n\n.. seealso:: :py:func:`fetch`\n', 'entryAdded': 'Emitted whenever a new ``entry`` is available from the feed (as a result\nof a call to :py:func:`~QgsNewsFeedParser.fetch`).\n\n.. seealso:: :py:func:`fetch`\n', 'entryUpdated': 'Emitted whenever an existing ``entry`` is available from the feed (as a result\nof a call to :py:func:`~QgsNewsFeedParser.fetch`).\n\n.. seealso:: :py:func:`fetch`\n\n.. versionadded:: 3.36\n', 'entryDismissed': 'Emitted whenever an ``entry`` is dismissed (as a result of a call\nto :py:func:`~QgsNewsFeedParser.dismissEntry`).\n\n.. seealso:: :py:func:`dismissEntry`\n', 'imageFetched': 'Emitted when the image attached to the entry with the specified ``key`` has been fetched\nand is now available.\n'}
    QgsNewsFeedParser.keyForFeed = staticmethod(QgsNewsFeedParser.keyForFeed)
    QgsNewsFeedParser.__signal_arguments__ = {'fetched': ['entries: List[QgsNewsFeedParser.Entry]'], 'entryAdded': ['entry: QgsNewsFeedParser.Entry'], 'entryUpdated': ['entry: QgsNewsFeedParser.Entry'], 'entryDismissed': ['entry: QgsNewsFeedParser.Entry'], 'imageFetched': ['key: int', 'pixmap: QPixmap']}
    QgsNewsFeedParser.__group__ = ['network']
except (NameError, AttributeError):
    pass
