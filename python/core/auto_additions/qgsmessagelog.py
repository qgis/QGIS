# The following has been generated automatically from src/core/qgsmessagelog.h
try:
    QgsMessageLog.__attribute_docs__ = {'messageReceived': 'Emitted whenever the log receives a message which is not a\n:py:class:`Qgis`.MessageLevel.Info level message and which has the\n``notifyUser`` flag as ``True``.\n\nIf :py:class:`QgsMessageLogNotifyBlocker` objects have been created then\nthis signal may be temporarily suppressed.\n\n.. seealso:: :py:class:`QgsMessageLogNotifyBlocker`\n'}
    QgsMessageLog.logMessage = staticmethod(QgsMessageLog.logMessage)
    QgsMessageLog.__signal_arguments__ = {'messageReceived': ['received: bool']}
except (NameError, AttributeError):
    pass
