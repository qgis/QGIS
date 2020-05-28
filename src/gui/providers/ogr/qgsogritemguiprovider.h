/***************************************************************************
      qgsogritemguiprovider.h
      -------------------
    begin                : June, 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOGRITEMGUIPROVIDER_H
#define QGSOGRITEMGUIPROVIDER_H

#include <QObject>
#include "qgsdataitemguiprovider.h"
#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsOgrItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT

  public:

    QgsOgrItemGuiProvider() = default;

    QString name() override { return QStringLiteral( "ogr_items" ); }

    void populateContextMenu( QgsDataItem *item, QMenu *menu,
                              const QList<QgsDataItem *> &selectedItems,
                              QgsDataItemGuiContext context ) override;

  protected slots:
    void onDeleteLayer();
    void deleteCollection();
};

///@endcond
#endif // QGSOGRITEMGUIPROVIDER_H
