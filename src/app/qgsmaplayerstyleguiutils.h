#ifndef QGSMAPLAYERSTYLEGUIUTILS_H
#define QGSMAPLAYERSTYLEGUIUTILS_H

#include <QObject>

#include "qgssingleton.h"

class QgsMapLayer;
class QMenu;

/** Various GUI utility functions for dealing with map layer's style manager */
class QgsMapLayerStyleGuiUtils : public QObject, public QgsSingleton<QgsMapLayerStyleGuiUtils>
{
    Q_OBJECT
  public:

    void addStyleManagerMenu( QMenu* menu, QgsMapLayer* layer );

  private:
    QString defaultStyleName();

  private slots:
    void addStyle();
    void useStyle();
    void removeStyle();

};

#endif // QGSMAPLAYERSTYLEGUIUTILS_H
