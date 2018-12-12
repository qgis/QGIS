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
      MeshLayer = 128, //!< QgsMeshLayer \since QGIS 3.6
      All = RasterLayer | VectorLayer | PluginLayer | MeshLayer
    };
    Q_DECLARE_FLAGS( Filters, Filter )
    Q_FLAG( Filters )

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
     * Sets \a filter flags which affect how layers are filtered within the model.
     *
     * \see filters()
     *
     * \since QGIS 2.3
     */
    QgsMapLayerProxyModel *setFilters( QgsMapLayerProxyModel::Filters filters );

    /**
     * Returns the filter flags which affect how layers are filtered within the model.
     *
     * \see setFilters()
     *
     * \since QGIS 2.3
     */
    const Filters &filters() const { return mFilters; }

    /**
     * Sets a whitelist of \a layers to include within the model. Only layers
     * from this list will be shown.
     *
     * An empty list indicates that no whitelisting should be performed.
     *
     * \see layerWhitelist()
     * \see setExceptedLayerList()
     *
     * \since QGIS 3.4
     */
    void setLayerWhitelist( const QList<QgsMapLayer *> &layers );

    /**
     * Returns the list of layers which are excluded from the model.
     *
     * An empty list indicates that no whitelisting should be performed.
     *
     * \see setLayerWhitelist()
     * \see exceptedLayerList()
     *
     * \since QGIS 3.4
     */
    QList<QgsMapLayer *> layerWhitelist() {return mLayerWhitelist;}

    /**
     * Sets a blacklist of layers to exclude from the model.
     * \see exceptedLayerList()
     * \see setExceptedLayerIds()
     * \see setLayerWhitelist()
     */
    void setExceptedLayerList( const QList<QgsMapLayer *> &exceptList );

    /**
     * Returns the blacklist of layers which are excluded from the model.
     * \see setExceptedLayerList()
     * \see exceptedLayerIds()
     * \see layerWhitelist()
     */
    QList<QgsMapLayer *> exceptedLayerList() {return mExceptList;}

    /**
     * Sets a blacklist of layers (by layer ID) to exclude from the model.
     * \see exceptedLayerIds()
     * \see setExceptedLayerList()
     */
    void setExceptedLayerIds( const QStringList &ids );

    /**
     * Returns the blacklist of layer IDs which are excluded from the model.
     * \see setExceptedLayerIds()
     * \see exceptedLayerList()
     */
    QStringList exceptedLayerIds() const;

    /**
     * Sets a blacklist of data providers which should be excluded from the model.
     * \see excludedProviders()
     * \since QGIS 3.0
     */
    void setExcludedProviders( const QStringList &providers );

    /**
     * Returns the blacklist of data providers which are excluded from the model.
     * \see setExcludedProviders()
     * \since QGIS 3.0
     */
    QStringList excludedProviders() const { return mExcludedProviders; }

    /**
     * Returns the current filter string, if set.
     *
     * \see setFilterString()
     * \since QGIS 3.4
     */
    QString filterString() const { return mFilterString; }

    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;

  public slots:

    /**
     * Sets a \a filter string, such that only layers with names matching the
     * specified string will be shown.
     *
     * \see filterString()
     * \since QGIS 3.4
    */
    void setFilterString( const QString &filter );

  private:
    Filters mFilters;
    QList<QgsMapLayer *> mExceptList;
    QList<QgsMapLayer *> mLayerWhitelist;
    QgsMapLayerModel *mModel = nullptr;
    QStringList mExcludedProviders;
    QString mFilterString;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapLayerProxyModel::Filters )

#endif // QGSMAPLAYERPROXYMODEL_H
