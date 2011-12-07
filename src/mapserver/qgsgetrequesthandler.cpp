#include "qgsgetrequesthandler.h"
#include "qgslogger.h"
#include "qgsremotedatasourcebuilder.h"
#include <QStringList>
#include <QUrl>
#include <stdlib.h>

QgsGetRequestHandler::QgsGetRequestHandler(): QgsHttpRequestHandler()
{
}

QMap<QString, QString> QgsGetRequestHandler::parseInput()
{
  QString queryString;
  QMap<QString, QString> parameters;

  const char* qs = getenv( "QUERY_STRING" );
  if ( qs )
  {
    queryString = QString( qs );
    QgsDebugMsg( "query string is: " + queryString );
  }
  else
  {
    QgsDebugMsg( "error, no query string found" );
    return parameters; //no query string? something must be wrong...
  }

  requestStringToParameterMap( queryString, parameters );
  return parameters;
}
