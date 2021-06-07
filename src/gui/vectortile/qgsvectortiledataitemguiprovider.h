/***************************************************************************
  qgsvectortiledataitemguiprovider.h
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILEDATAITEMGUIPROVIDER_H
#define QGSVECTORTILEDATAITEMGUIPROVIDER_H

///@cond PRIVATE
#define SIP_NO_FILE

#include "qgsdataitemguiprovider.h"


class QgsVectorTileDataItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT
  public:

    QString name() override { return QStringLiteral( "Vector Tiles" ); }

    void populateContextMenu( QgsDataItem *item, QMenu *menu,
                              const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;

  private:
    static void editConnection( QgsDataItem *item );
    static void deleteConnection( QgsDataItem *item );
    static void newConnection( QgsDataItem *item );
    static void newArcGISConnection( QgsDataItem *item );
    static void saveXyzTilesServers();
    static void loadXyzTilesServers( QgsDataItem *item );

};

///@endcond

#endif // QGSVECTORTILEDATAITEMGUIPROVIDER_H
