%Exception QgsServerApiBadRequestException(SIP_Exception) /PyName=QgsServerApiBadRequestException/
{
%TypeHeaderCode
#include <qgsserverexception.h>
%End
%RaiseCode
  SIP_BLOCK_THREADS
  PyErr_SetString(sipException_QgsServerApiBadRequestException, sipExceptionRef.what().toUtf8().constData() );
  SIP_UNBLOCK_THREADS
%End
};

%Exception QgsServerApiInternalServerError(SIP_Exception) /PyName=QgsServerApiInternalServerError/
{
%TypeHeaderCode
#include <qgsserverexception.h>
%End
%RaiseCode
  SIP_BLOCK_THREADS
  PyErr_SetString(sipException_QgsServerApiInternalServerError, sipExceptionRef.what().toUtf8().constData() );
  SIP_UNBLOCK_THREADS
%End
};
