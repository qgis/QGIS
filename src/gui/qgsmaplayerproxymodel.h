/***************************************************************************
   qgsmaplayerproxymodel.h
    --------------------------------------
   Date                 : 01.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSMAPLAYERPROXYMODEL_H
#define QGSMAPLAYERPROXYMODEL_H

#include <QSortFilterProxyModel>

class QgsMapLayerModel;

/**
 * @brief The QgsMapLayerProxModel class provides an easy to use model to display the list of layers in widgets.
 * @note added in 2.3
 */
class GUI_EXPORT QgsMapLayerProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_FLAGS( Filters )
  public:
    enum Filter
    {
      RasterLayer = 1,
      NoGeometry = 2,
      PointLayer = 4,
      LineLayer = 8,
      PolygonLayer = 16,
      HasGeometry = PointLayer | LineLayer | PolygonLayer,
      VectorLayer = NoGeometry | HasGeometry,
      PluginLayer = 32,
      All = RasterLayer | PolygonLayer | PluginLayer
    };
    Q_DECLARE_FLAGS( Filters, Filter )

    /**
     * @brief QgsMapLayerProxModel creates a proxy model with a QgsMapLayerModel as source model.
     * It can be used to filter the layers list in a widget.
     */
    explicit QgsMapLayerProxyModel( QObject *parent = 0 );

    /**
     * @brief layerModel returns the QgsMapLayerModel used in this QSortFilterProxyModel
     */
    QgsMapLayerModel* sourceLayerModel() { return mModel; }

    /**
     * @brief setFilters set flags that affect how layers are filtered
     * @param filters are Filter flags
     * @note added in 2.3
     */
    QgsMapLayerProxyModel* setFilters( Filters filters );
    const Filters& filters() const { return mFilters; }

  private:
    Filters mFilters;
    QgsMapLayerModel* mModel;

    // QSortFilterProxyModel interface
  public:
    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const;
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapLayerProxyModel::Filters )

#endif // QGSMAPLAYERPROXYMODEL_H
