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
#include "qgis_sip.h"
#include "qgis.h"

class QgsMapLayerModel;
class QgsMapLayer;
class QgsProject;

/**
 * \ingroup core
 * \brief The QgsMapLayerProxyModel class provides an easy to use model to display the list of layers in widgets.
 */
class CORE_EXPORT QgsMapLayerProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY( Qgis::LayerFilters filters READ filters WRITE setFilters )
    Q_PROPERTY( QList<QgsMapLayer *> exceptedLayerList READ exceptedLayerList WRITE setExceptedLayerList )
    Q_PROPERTY( QStringList exceptedLayerIds READ exceptedLayerIds WRITE setExceptedLayerIds )

  public:

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
     */
    QgsMapLayerProxyModel *setFilters( Qgis::LayerFilters filters );

    /**
     * Filters according to layer type and/or geometry type.
     * \note for API compatibility
     * \since QGIS 3.34
     * \deprecated QGIS 3.34. Use the flag signature instead.
     */
    Q_DECL_DEPRECATED void setFilters( int filters ) SIP_DEPRECATED { setFilters( static_cast<Qgis::LayerFilters>( filters ) ); }

    /**
     * Returns the filter flags which affect how layers are filtered within the model.
     *
     * \see setFilters()
     *
     */
    const Qgis::LayerFilters &filters() const { return mFilters; }

    /**
     * Sets the \a project from which map layers are shown.
     *
     * If \a project is NULLPTR then QgsProject.instance() will be used.
     *
     * \since QGIS 3.24
     */
    void setProject( QgsProject *project );

    /**
     * Returns if the \a layer matches the given \a filters
     * \since QGIS 3.14
     */
    static bool layerMatchesFilters( const QgsMapLayer *layer, const Qgis::LayerFilters &filters );

    /**
     * Sets an allowlist of \a layers to include within the model. Only layers
     * from this list will be shown.
     *
     * An empty list indicates that no filter by allowlist should be performed.
     *
     * \see layerAllowlist()
     * \see setExceptedLayerList()
     *
     * \deprecated QGIS 3.40. Use setLayerAllowList().
     */
    Q_DECL_DEPRECATED void setLayerWhitelist( const QList<QgsMapLayer *> &layers ) SIP_DEPRECATED;

    /**
     * Sets an allowlist of \a layers to include within the model. Only layers
     * from this list will be shown.
     *
     * An empty list indicates that no filter by allowlist should be performed.
     *
     * \see layerAllowlist()
     * \see setExceptedLayerList()
     *
     * \since QGIS 3.14
     */
    void setLayerAllowlist( const QList<QgsMapLayer *> &layers );

    /**
     * Returns the list of layers which are excluded from the model.
     *
     * An empty list indicates that no filtering by allowlist should be performed.
     *
     * \see setLayerAllowlist()
     * \see exceptedLayerList()
     *
     * \deprecated QGIS 3.40. Use layerAllowlist() instead.
     */
    Q_DECL_DEPRECATED QList<QgsMapLayer *> layerWhitelist() SIP_DEPRECATED {return mLayerAllowlist;}

    /**
     * Returns the list of layers which are excluded from the model.
     *
     * An empty list indicates that no filtering by allowlist should be performed.
     *
     * \see setLayerAllowlist()
     * \see exceptedLayerList()
     *
     * \since QGIS 3.14
     */
    QList<QgsMapLayer *> layerAllowlist() {return mLayerAllowlist;}

    /**
     * Sets a blocklist of layers to exclude from the model.
     * \see exceptedLayerList()
     * \see setExceptedLayerIds()
     * \see setLayerAllowlist()
     */
    void setExceptedLayerList( const QList<QgsMapLayer *> &exceptList );

    /**
     * Returns the blocklist of layers which are excluded from the model.
     * \see setExceptedLayerList()
     * \see exceptedLayerIds()
     * \see layerAllowlist()
     */
    QList<QgsMapLayer *> exceptedLayerList() {return mExceptList;}

    /**
     * Sets a blocklist of layers (by layer ID) to exclude from the model.
     * \see exceptedLayerIds()
     * \see setExceptedLayerList()
     */
    void setExceptedLayerIds( const QStringList &ids );

    /**
     * Returns the blocklist of layer IDs which are excluded from the model.
     * \see setExceptedLayerIds()
     * \see exceptedLayerList()
     */
    QStringList exceptedLayerIds() const;

    /**
     * Sets a blocklist of data providers which should be excluded from the model.
     * \see excludedProviders()
     */
    void setExcludedProviders( const QStringList &providers );

    /**
     * Returns the blocklist of data providers which are excluded from the model.
     * \see setExcludedProviders()
     */
    QStringList excludedProviders() const { return mExcludedProviders; }

    /**
     * Returns the current filter string, if set.
     *
     * \see setFilterString()
     * \since QGIS 3.4
     */
    QString filterString() const { return mFilterString; }

    /**
     * Returns TRUE if the proxy model accepts the specified map \a layer.
     *
     * \since QGIS 3.8
     */
    bool acceptsLayer( QgsMapLayer *layer ) const;

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
    Qgis::LayerFilters mFilters;
    QList<QgsMapLayer *> mExceptList;
    QList<QgsMapLayer *> mLayerAllowlist;
    QgsMapLayerModel *mModel = nullptr;
    QStringList mExcludedProviders;
    QString mFilterString;
};

#endif // QGSMAPLAYERPROXYMODEL_H
