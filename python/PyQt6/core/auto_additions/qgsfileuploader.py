# The following has been generated automatically from src/core/network/qgsfileuploader.h
try:
    QgsFileUploader.__attribute_docs__ = {'uploadCompleted': 'Emitted when the upload has completed successfully\n', 'uploadExited': 'Emitted always when the uploader exits\n', 'uploadCanceled': 'Emitted when the upload was canceled by the user.\n\n.. seealso:: :py:func:`cancelUpload`\n', 'uploadError': 'Emitted when an error makes the upload fail\n', 'uploadProgress': 'Emitted when data are ready to be processed\n'}
    QgsFileUploader.__signal_arguments__ = {'uploadCompleted': ['url: QUrl'], 'uploadError': ['errorMessages: List[str]'], 'uploadProgress': ['bytesSent: int', 'bytesTotal: int']}
    QgsFileUploader.__group__ = ['network']
except (NameError, AttributeError):
    pass
