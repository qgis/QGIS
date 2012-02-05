%Exception QgsCsException(SIP_Exception) /PyName=QgsCsException/
{
%TypeHeaderCode
#include <qgscsexception.h>
%End
%RaiseCode
  SIP_BLOCK_THREADS
  PyErr_SetString(sipException_QgsCsException, sipExceptionRef.what().toUtf8().constData() );
  SIP_UNBLOCK_THREADS
%End
};

%Exception QgsException(SIP_Exception) /PyName=QgsException/
{
%TypeHeaderCode
#include <qgsexception.h>
%End
%RaiseCode
  SIP_BLOCK_THREADS
  PyErr_SetString(sipException_QgsException, sipExceptionRef.what().toUtf8().constData() );
  SIP_UNBLOCK_THREADS
%End
};
