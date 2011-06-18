#include "qgspostrequesthandler.h"
#include "qgslogger.h"

QgsPostRequestHandler::QgsPostRequestHandler()
{
}

QgsPostRequestHandler::~QgsPostRequestHandler()
{
}

std::map<QString, QString> QgsPostRequestHandler::parseInput()
{
  QgsDebugMsg("QgsPostRequestHandler::parseInput");
  std::map<QString, QString> parameters;
  QString inputString = readPostBody();
  QgsDebugMsg(inputString);
  requestStringToParameterMap( inputString, parameters );
  return parameters;
}
