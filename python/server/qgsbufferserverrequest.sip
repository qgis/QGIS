/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * ../src/server/qgsbufferserverrequest.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsBufferServerRequest : QgsServerRequest
{
%Docstring
 QgsBufferServerRequest
 Class defining request with  data
%End

%TypeHeaderCode
#include "qgsbufferserverrequest.h"
%End
  public:

    QgsBufferServerRequest( const QString &url, Method method = GetMethod, QByteArray *data = 0 );
%Docstring
 Constructor

 \param url the url string
 \param method the request method
%End

    QgsBufferServerRequest( const QUrl &url, Method method = GetMethod, QByteArray *data = 0 );
%Docstring
 Constructor

 \param url QUrl
 \param method the request method
%End

    ~QgsBufferServerRequest();

    virtual QByteArray data() const;
%Docstring
 :rtype: QByteArray
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * ../src/server/qgsbufferserverrequest.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
