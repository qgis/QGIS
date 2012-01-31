%Exception QgsCsException(SIP_Exception) /PyName=QgsCsException/
{
%RaiseCode
  SIP_BLOCK_THREADS
  PyErr_SetString(sipException_QgsCsException, sipExceptionRef.what().toUtf8().constData() );
  SIP_UNBLOCK_THREADS
%End
};

%Exception QgsException(SIP_Exception) /PyName=QgsException/
{
%RaiseCode
  SIP_BLOCK_THREADS
  PyErr_SetString(sipException_QgsException, sipExceptionRef.what().toUtf8().constData() );
  SIP_UNBLOCK_THREADS
%End
};
