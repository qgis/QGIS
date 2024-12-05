# The following has been generated automatically from src/core/qgsmessagelog.h
try:
    QgsMessageLog.__attribute_docs__ = {'messageReceived': 'Emitted whenever the log receives a message which is not a :py:class:`Qgis`.MessageLevel.Info level message\nand which has the ``notifyUser`` flag as ``True``.\n\nIf :py:class:`QgsMessageLogNotifyBlocker` objects have been created then this signal may be\ntemporarily suppressed.\n\n.. seealso:: :py:class:`QgsMessageLogNotifyBlocker`\n'}
    QgsMessageLog.logMessage = staticmethod(QgsMessageLog.logMessage)
    QgsMessageLog.__signal_arguments__ = {'messageReceived': ['received: bool']}
except (NameError, AttributeError):
    pass
