/***************************************************************************
   qgsmaplayermodel.h
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

#ifndef QGSMAPLAYERMODEL_H
#define QGSMAPLAYERMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QStringList>

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsMapLayer;


/**
 * \ingroup core
 * \brief The QgsMapLayerModel class is a model to display layers in widgets.
 * \see QgsMapLayerProxyModel to sort and/filter the layers
 * \see QgsFieldModel to combine in with a field selector.
 * \since QGIS 2.3
 */
class CORE_EXPORT QgsMapLayerModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY( bool allowEmptyLayer READ allowEmptyLayer WRITE setAllowEmptyLayer )
    Q_PROPERTY( bool showCrs READ showCrs WRITE setShowCrs )
    Q_PROPERTY( bool itemsCheckable READ itemsCheckable WRITE setItemsCheckable )
    Q_PROPERTY( QStringList additionalItems READ additionalItems WRITE setAdditionalItems )

  public:

    //! Item data roles
    enum ItemDataRole
    {
      LayerIdRole = Qt::UserRole + 1, //!< Stores the map layer ID
      LayerRole, //!< Stores pointer to the map layer itself
      EmptyRole, //!< True if index corresponds to the empty (not set) value
      AdditionalRole, //!< True if index corresponds to an additional (non map layer) item
    };
    Q_ENUM( ItemDataRole )

    /**
     * \brief QgsMapLayerModel creates a model to display layers in widgets.
     */
    explicit QgsMapLayerModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * \brief QgsMapLayerModel creates a model to display a specific list of layers in a widget.
     */
    explicit QgsMapLayerModel( const QList<QgsMapLayer *> &layers, QObject *parent = nullptr );

    /**
     * \brief setItemsCheckable defines if layers should be selectable in the widget
     */
    void setItemsCheckable( bool checkable );

    /**
     * \brief checkAll changes the checkstate for all the layers
     */
    void checkAll( Qt::CheckState checkState );

    /**
     * Sets whether an optional empty layer ("not set") option is present in the model.
     * \see allowEmptyLayer()
     * \since QGIS 3.0
     */
    void setAllowEmptyLayer( bool allowEmpty );

    /**
     * Returns TRUE if the model allows the empty layer ("not set") choice.
     * \see setAllowEmptyLayer()
     * \since QGIS 3.0
     */
    bool allowEmptyLayer() const { return mAllowEmpty; }

    /**
     * Sets whether the CRS of layers is also included in the model's display role.
     * \see showCrs()
     * \since QGIS 3.0
     */
    void setShowCrs( bool showCrs );

    /**
     * Returns TRUE if the model includes layer's CRS in the display role.
     * \see setShowCrs()
     * \since QGIS 3.0
     */
    bool showCrs() const { return mShowCrs; }

    /**
     * \brief layersChecked returns the list of layers which are checked (or unchecked)
     */
    QList<QgsMapLayer *> layersChecked( Qt::CheckState checkState = Qt::Checked );
    //! returns if the items can be checked or not
    bool itemsCheckable() const { return mItemCheckable; }

    /**
     * \brief indexFromLayer returns the model index for a given layer
     * \see layerFromIndex()
     */
    QModelIndex indexFromLayer( QgsMapLayer *layer ) const;

    /**
     * Returns the map layer corresponding to the specified \a index.
     * \see indexFromLayer()
     * \since QGIS 3.0
     */
    QgsMapLayer *layerFromIndex( const QModelIndex &index ) const;

    /**
     * Sets a list of additional (non map layer) items to include at the end of the model.
     * These may represent additional layers such as layers which are not included in the map
     * layer registry, or paths to layers which have not yet been loaded into QGIS.
     * \see additionalItems()
     * \since QGIS 3.0
     */
    void setAdditionalItems( const QStringList &items );

    /**
     * Returns the list of additional (non map layer) items included at the end of the model.
     * \see setAdditionalItems()
     * \since QGIS 3.0
     */
    QStringList additionalItems() const { return mAdditionalItems; }

    // QAbstractItemModel interface
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;

    /**
     * Returns strings for all roles supported by this model.
     *
     * \note Available only with Qt5 (Python and c++)
     */
    QHash<int, QByteArray> roleNames() const override SIP_SKIP;

    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

    /**
     * Returns the icon corresponding to a specified map \a layer.
     * \since QGIS 3.0
     */
    static QIcon iconForLayer( QgsMapLayer *layer );

  protected slots:
    void removeLayers( const QStringList &layerIds );
    void addLayers( const QList<QgsMapLayer *> &layers );

  protected:
    QList<QgsMapLayer *> mLayers;
    QMap<QString, Qt::CheckState> mLayersChecked;
    bool mItemCheckable = false;

  private:

    bool mAllowEmpty = false;
    bool mShowCrs = false;
    QStringList mAdditionalItems;
};

#endif // QGSMAPLAYERMODEL_H
