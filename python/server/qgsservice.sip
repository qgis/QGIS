/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgsservice.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsService
{
%Docstring
 QgsService
 Class defining interfaces for QGIS server services

 This class provides methods for executing server requests
 They are registered at runtime for a given service name.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsservice.h"
#include "qgsserverrequest.h"
#include "qgsserverresponse.h"
%End
  public:

    QgsService();
%Docstring
Constructor
%End

    virtual ~QgsService();
%Docstring
Destructor
%End

    virtual QString name() const = 0;
%Docstring
 :return: the name of the service
 :rtype: str
%End

    virtual QString version() const = 0;
%Docstring
 :return: the version of the service
 :rtype: str
%End

    virtual bool allowMethod( QgsServerRequest::Method ) const = 0;
%Docstring
 Return true if the given method is supported for that
 service.
 :rtype: bool
%End

    virtual void executeRequest( const QgsServerRequest &request,
                                 QgsServerResponse &response,
                                 const QgsProject *project ) = 0;
%Docstring
 Execute the requests and set result in QgsServerRequest
%End
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgsservice.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
