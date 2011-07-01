/***************************************************************************
                         qgslegendmodel.h  -  description
                         -----------------
    begin                : June 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLEGENDMODEL_H
#define QGSLEGENDMODEL_H

#include <QStandardItemModel>
#include <QStringList>
#include <QSet>

class QDomDocument;
class QDomElement;
class QgsMapLayer;
class QgsSymbol;
class QgsSymbolV2;
class QgsVectorLayer;

//Information about relationship between groups and layers
//key: group name (or null strings for single layers without groups)
//value: containter with layer ids contained in the group
typedef QPair< QString, QList<QString> > GroupLayerInfo;

/** \ingroup MapComposer
 * A model that provides group, layer and classification items.
 */
class CORE_EXPORT QgsLegendModel: public QStandardItemModel
{
    Q_OBJECT

  public:

    enum ItemType
    {
      GroupItem = 0,
      LayerItem,
      ClassificationItem
    };

    QgsLegendModel();
    ~QgsLegendModel();

    /**Sets layer set and groups*/
    void setLayerSetAndGroups( const QStringList& layerIds, const QList< GroupLayerInfo >& groupInfo );
    void setLayerSet( const QStringList& layerIds );
    /**Adds a group to a toplevel position (or -1 if it should be placed at the end of the legend). Returns a pointer to the added group*/
    QStandardItem* addGroup( QString text = tr( "Group" ), int position = -1 );

    /**Tries to automatically update a model entry (e.g. a whole layer or only a single item)*/
    void updateItem( QStandardItem* item );
    /**Updates the whole symbology of a layer*/
    void updateLayer( QStandardItem* layerItem );
    /**Tries to update a single classification item*/
    void updateVectorClassificationItem( QStandardItem* classificationItem, QgsSymbol* symbol, QString itemText )
    { Q_UNUSED( classificationItem ); Q_UNUSED( symbol ); Q_UNUSED( itemText ); }
    void updateVectorV2ClassificationItem( QStandardItem* classificationItem, QgsSymbolV2* symbol, QString itemText )
    { Q_UNUSED( classificationItem ); Q_UNUSED( symbol ); Q_UNUSED( itemText ); }
    void updateRasterClassificationItem( QStandardItem* classificationItem )
    { Q_UNUSED( classificationItem ); }

    bool writeXML( QDomElement& composerLegendElem, QDomDocument& doc ) const;
    bool readXML( const QDomElement& legendModelElem, const QDomDocument& doc );

    Qt::DropActions supportedDropActions() const;
    Qt::ItemFlags flags( const QModelIndex &index ) const;

    /**Implemented to support drag operations*/
    virtual bool removeRows( int row, int count, const QModelIndex & parent = QModelIndex() );

    /**For the drag operation*/
    QMimeData* mimeData( const QModelIndexList &indexes ) const;
    QStringList mimeTypes() const;

    /**Implements the drop operation*/
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );

    void setAutoUpdate( bool autoUpdate );
    bool autoUpdate() { return mAutoUpdate; }

  public slots:
    void removeLayer( const QString& layerId );
    void addLayer( QgsMapLayer* theMapLayer );

  signals:
    void layersChanged();

  private:
    /**Adds classification items of vector layers
     @return 0 in case of success*/
    int addVectorLayerItems( QStandardItem* layerItem, QgsVectorLayer* vlayer );

    /**Adds classification items of vector layers using new symbology*/
    int addVectorLayerItemsV2( QStandardItem* layerItem, QgsVectorLayer* vlayer );

    /**Adds item of raster layer
     @return 0 in case of success*/
    int addRasterLayerItems( QStandardItem* layerItem, QgsMapLayer* rlayer );

    /**Creates a model item for a vector symbol. The calling function takes ownership*/
    QStandardItem* itemFromSymbol( QgsSymbol* s, int opacity, const QString& layerID );

  protected:
    QStringList mLayerIds;
    /**True if this application has toplevel windows (normally true). If this is false, this means that the application
       might not have a running x-server on unix systems and so QPixmap and QIcon cannot be used*/
    bool mHasTopLevelWindow;

    /**True if the legend is auto updated when layers are added or removed from the map canvas */
    bool mAutoUpdate;
};

#endif
