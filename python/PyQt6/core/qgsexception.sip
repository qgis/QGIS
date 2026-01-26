%Exception QgsCsException(SIP_Exception) /PyName=QgsCsException/
{
%TypeHeaderCode
#include "qgsexception.h"

%End
%RaiseCode
  SIP_BLOCK_THREADS
  PyErr_SetString(sipException_QgsCsException, sipExceptionRef.what().toUtf8().constData() );
  SIP_UNBLOCK_THREADS
%End
};

%TypeHintCode
class QgsCsException(Exception):
    def __init__(self, *args, **kwargs) -> None: ...
%End

%Exception QgsProcessingException(SIP_Exception) /PyName=QgsProcessingException/
{
%TypeHeaderCode
#include <qgsexception.h>
%End
%RaiseCode
  SIP_BLOCK_THREADS
  PyErr_SetString(sipException_QgsProcessingException, sipExceptionRef.what().toUtf8().constData() );
  SIP_UNBLOCK_THREADS
%End
};

%TypeHintCode
class QgsProcessingException(Exception):
    def __init__(self, *args, **kwargs) -> None: ...
%End

%Exception QgsProviderConnectionException(SIP_Exception) /PyName=QgsProviderConnectionException/
{
%TypeHeaderCode
#include <qgsexception.h>
%End
%RaiseCode
  SIP_BLOCK_THREADS
  PyErr_SetString(sipException_QgsProviderConnectionException, sipExceptionRef.what().toUtf8().constData() );
  SIP_UNBLOCK_THREADS
%End
};

%TypeHintCode
class QgsProviderConnectionException(Exception):
    def __init__(self, *args, **kwargs) -> None: ...
%End

%Exception QgsNotSupportedException(SIP_Exception) /PyName=QgsNotSupportedException/
{
%TypeHeaderCode
#include <qgsexception.h>
%End
%RaiseCode
  SIP_BLOCK_THREADS
  PyErr_SetString(sipException_QgsNotSupportedException, sipExceptionRef.what().toUtf8().constData() );
  SIP_UNBLOCK_THREADS
%End
};

%TypeHintCode
class QgsNotSupportedException(Exception):
    def __init__(self, *args, **kwargs) -> None: ...
%End

%Exception QgsSettingsException(SIP_Exception) /PyName=QgsSettingsException/
{
%TypeHeaderCode
#include <qgsexception.h>
%End
%RaiseCode
  SIP_BLOCK_THREADS
  PyErr_SetString(sipException_QgsSettingsException, sipExceptionRef.what().toUtf8().constData() );
  SIP_UNBLOCK_THREADS
%End
};

%TypeHintCode
class QgsSettingsException(Exception):
    def __init__(self, *args, **kwargs) -> None: ...
%End

%If ( HAVE_SFCGAL_SIP )
%Exception QgsSfcgalException(SIP_Exception) /PyName=QgsSfcgalException/
{
%TypeHeaderCode
#include <qgsexception.h>
%End
%RaiseCode
  SIP_BLOCK_THREADS
  PyErr_SetString(sipException_QgsSfcgalException, sipExceptionRef.what().toUtf8().constData() );
  SIP_UNBLOCK_THREADS
%End
};
%End

%TypeHintCode
class QgsSfcgalException(Exception):
    def __init__(self, *args, **kwargs) -> None: ...
%End

%Exception QgsInvalidArgumentException(SIP_Exception) /PyName=QgsInvalidArgumentException/
{
%TypeHeaderCode
#include <qgsexception.h>
%End
%RaiseCode
  SIP_BLOCK_THREADS
  PyErr_SetString(sipException_QgsInvalidArgumentException, sipExceptionRef.what().toUtf8().constData() );
  SIP_UNBLOCK_THREADS
%End
};

%TypeHintCode
class QgsInvalidArgumentException(Exception):
    def __init__(self, *args, **kwargs) -> None: ...
%End

// IMPORTANT -- QgsException MUST be last listed, or it will greedily prevent the more
// specialized exceptions from being raised

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

%TypeHintCode
class QgsException(Exception):
    def __init__(self, *args, **kwargs) -> None: ...
%End

// IMPORTANT -- QgsException MUST be last listed, or it will greedily prevent the more
// specialized exceptions from being raised
