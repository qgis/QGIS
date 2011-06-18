#ifndef QGSPOSTREQUESTHANDLER_H
#define QGSPOSTREQUESTHANDLER_H

#include "qgshttprequesthandler.h"

/**Request handler for HTTP POST*/
class QgsPostRequestHandler: public QgsHttpRequestHandler
{
  public:
    QgsPostRequestHandler();
    ~QgsPostRequestHandler();

    /**Parses the input and creates a request neutral Parameter/Value map*/
    std::map<QString, QString> parseInput();
};

#endif // QGSPOSTREQUESTHANDLER_H
