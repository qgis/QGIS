/***************************************************************************
  qgspdaldataitemguiprovider.h
  --------------------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPDALDATAITEMGUIPROVIDER_H
#define QGSPDALDATAITEMGUIPROVIDER_H

#include "qgsdataitemguiprovider.h"


class QgsPdalDataItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT
  public:

    QString name() override { return QStringLiteral( "pdal" ); }

    void populateContextMenu( QgsDataItem *item, QMenu *menu,
                              const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;
};

#endif // QGSPDALDATAITEMGUIPROVIDER_H
