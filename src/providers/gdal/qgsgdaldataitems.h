#ifndef QGSGDALDATAITEMS_H
#define QGSGDALDATAITEMS_H

#include "qgsdataitem.h"

class QgsGdalLayerItem : public QgsLayerItem
{
  public:
    QgsGdalLayerItem( QgsDataItem* parent,
                      QString name, QString path, QString uri );
    ~QgsGdalLayerItem();

    bool setCrs( QgsCoordinateReferenceSystem crs );
    Capability capabilities();
};


#endif // QGSGDALDATAITEMS_H
