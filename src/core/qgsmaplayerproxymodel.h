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
#include <QStringList>

#include "qgis_core.h"
#include "qgis.h"

class QgsMapLayerModel;
class QgsMapLayer;

/**
 * \ingroup core
 * \brief The QgsMapLayerProxyModel class provides an easy to use model to display the list of layers in widgets.
 * \since QGIS 2.3
 */
class CORE_EXPORT QgsMapLayerProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_FLAGS( Filters )

    Q_PROPERTY( QgsMapLayerProxyModel::Filters filters READ filters WRITE setFilters )
    Q_PROPERTY( QList<QgsMapLayer *> exceptedLayerList READ exceptedLayerList WRITE setExceptedLayerList )
    Q_PROPERTY( QStringList exceptedLayerIds READ exceptedLayerIds WRITE setExceptedLayerIds )

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
      WritableLayer = 64,
      All = RasterLayer | VectorLayer | PluginLayer
    };
    Q_DECLARE_FLAGS( Filters, Filter )

    /**
     * \brief QgsMapLayerProxModel creates a proxy model with a QgsMapLayerModel as source model.
     * It can be used to filter the layers list in a widget.
     */
    explicit QgsMapLayerProxyModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * \brief layerModel returns the QgsMapLayerModel used in this QSortFilterProxyModel
     */
    QgsMapLayerModel *sourceLayerModel() const { return mModel; }

    /**
     * \brief setFilters set flags that affect how layers are filtered
     * \param filters are Filter flags
     * \since QGIS 2.3
     */
    QgsMapLayerProxyModel *setFilters( QgsMapLayerProxyModel::Filters filters );
    const Filters &filters() const { return mFilters; }

    //! offer the possibility to except some layers to be listed
    void setExceptedLayerList( const QList<QgsMapLayer *> &exceptList );
    //! Get the list of maplayers which are excluded from the list
    QList<QgsMapLayer *> exceptedLayerList() {return mExceptList;}

    //! Set the list of maplayer ids which are excluded from the list
    void setExceptedLayerIds( const QStringList &ids );
    //! Get the list of maplayer ids which are excluded from the list
    QStringList exceptedLayerIds() const;

    /**
     * Sets a list of data providers which should be excluded from the model.
     * \since QGIS 3.0
     * \see excludedProviders()
     */
    void setExcludedProviders( const QStringList &providers );

    /**
     * Returns the list of data providers which are excluded from the model.
     * \see setExcludedProviders()
     * \since QGIS 3.0
     */
    QStringList excludedProviders() const { return mExcludedProviders; }

    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;

  private:
    Filters mFilters;
    QList<QgsMapLayer *> mExceptList;
    QgsMapLayerModel *mModel = nullptr;
    QStringList mExcludedProviders;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapLayerProxyModel::Filters )

#endif // QGSMAPLAYERPROXYMODEL_H
