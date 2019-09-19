/***************************************************************************
  qgslayertreeviewbadlayerindicatorprovider.h - QgsLayerTreeViewBadLayerIndicatorProvider

 ---------------------
 begin                : 17.10.2018
 copyright            : (C) 2018 by Alessandro Pasotti
 email                : elpaso@itopen.it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYERTREEVIEWBADLAYERINDICATORPROVIDER_H
#define QGSLAYERTREEVIEWBADLAYERINDICATORPROVIDER_H

#include "qgslayertreeviewindicatorprovider.h"
#include "qgsmaplayer.h"

#include <QObject>
#include <QPointer>

//! Indicators for bad layers
class QgsLayerTreeViewBadLayerIndicatorProvider : public QgsLayerTreeViewIndicatorProvider
{
    Q_OBJECT

  public:
    explicit QgsLayerTreeViewBadLayerIndicatorProvider( QgsLayerTreeView *view );

  public slots:

    /**
     * Used to report an \a error, e.g. a rendering or data access error, with the specified \a layer.
     */
    void reportLayerError( const QString &error, QgsMapLayer *layer );

  signals:

    /**
     * Emitted when the user clicks on the bad layer indicator icon
     * \param maplayer for change data source request
     */
    void requestChangeDataSource( QgsMapLayer *maplayer );

  protected slots:
    void onIndicatorClicked( const QModelIndex &index ) override;


  private:
    QString iconName( QgsMapLayer *layer ) override;
    QString tooltipText( QgsMapLayer *layer ) override;
    bool acceptLayer( QgsMapLayer *layer ) override;

    struct Error
    {
      Error( const QString &error, QgsMapLayer *layer = nullptr )
        : error( error )
        , layer( layer )
      {}
      QString error;
      QPointer< QgsMapLayer >  layer;
    };

    QList< Error > mErrors;
};

#endif // QGSLAYERTREEVIEWBADLAYERINDICATORPROVIDER_H
