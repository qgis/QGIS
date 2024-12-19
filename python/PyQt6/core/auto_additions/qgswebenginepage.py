# The following has been generated automatically from src/core/web/qgswebenginepage.h
try:
    QgsWebEnginePage.__attribute_docs__ = {'loadStarted': 'This signal is emitted when the page starts loading content.\n', 'loadProgress': 'This signal is emitted when the global ``progress`` status changes.\n\nThe current value is provided by ``progress`` and scales from 0 to 100.\nIt accumulates changes from all the child frames.\n', 'loadFinished': 'This signal is emitted when the page finishes loading content.\n\nThis signal is independent of script execution or page rendering.\n\n``ok`` will indicate whether the load was successful or any error occurred.\n'}
    QgsWebEnginePage.__signal_arguments__ = {'loadProgress': ['progress: int'], 'loadFinished': ['ok: bool']}
    QgsWebEnginePage.__group__ = ['web']
except (NameError, AttributeError):
    pass
