/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgsserverrequest.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsServerRequest
{

%TypeHeaderCode
#include "qgsserverrequest.h"
%End
  public:

    typedef QMap<QString, QString> Parameters;
    typedef QMap<QString, QString> Headers;

    enum Method
    {
      HeadMethod,
      PutMethod,
      GetMethod,
      PostMethod,
      DeleteMethod
    };


    QgsServerRequest();
%Docstring
 Constructor
%End

    QgsServerRequest( const QString &url, QgsServerRequest::Method method = QgsServerRequest::GetMethod, const QgsServerRequest::Headers &headers = QgsServerRequest::Headers() );
%Docstring
 Constructor

 \param url the url string
 \param method the request method
 \param headers
%End

    QgsServerRequest( const QUrl &url, QgsServerRequest::Method method = QgsServerRequest::GetMethod, const QgsServerRequest::Headers &headers = QgsServerRequest::Headers() );
%Docstring
 Constructor

 \param url QUrl
 \param method the request method
 \param headers
%End

    virtual ~QgsServerRequest();
%Docstring
destructor
%End

    QUrl url() const;
%Docstring
 :return:  the request url
 :rtype: QUrl
%End

    QgsServerRequest::Method method() const;
%Docstring
 :return: the request method
 :rtype: QgsServerRequest.Method
%End

    QgsServerRequest::Parameters parameters() const;
%Docstring
 Return a map of query parameters with keys converted
 to uppercase
 :rtype: QgsServerRequest.Parameters
%End

    void setParameter( const QString &key, const QString &value );
%Docstring
 Set a parameter
%End

    QString parameter( const QString &key ) const;
%Docstring
 Get a parameter value
 :rtype: str
%End

    void removeParameter( const QString &key );
%Docstring
 Remove a parameter
%End

    QString header( const QString &name ) const;
%Docstring
 Return the header value
 @param name of the header
 @return the header value or an empty string
 :rtype: str
%End

    void setHeader( const QString &name, const QString &value );
%Docstring
 Set an header
 @param name
 @param value
%End

    QMap<QString, QString> headers() const;
%Docstring
 Return the header map
 @return the headers map
 :rtype: QMap<str, QString>
%End

    void removeHeader( const QString &name );
%Docstring
 Remove an header
 @param name
%End

    virtual QByteArray data() const;
%Docstring
 Return post/put data
 Check for QByteArray.isNull() to check if data
 is available.
 :rtype: QByteArray
%End

    void setUrl( const QUrl &url );
%Docstring
 Set the request url
%End

    void setMethod( QgsServerRequest::Method method );
%Docstring
 Set the request method
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgsserverrequest.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
