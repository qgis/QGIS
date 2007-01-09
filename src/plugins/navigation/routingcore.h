
#ifndef ROUTINGCORE_H
#define ROUTINGCORE_H

#include <QObject>

class QgisInterface;

class RoutingCore : public QObject
{
  Q_OBJECT
  
  public:
    RoutingCore(QgisInterface* qgis);
    
  private:
    
    QgisInterface* mQgis;
};

#endif
