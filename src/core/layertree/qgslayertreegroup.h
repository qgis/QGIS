/***************************************************************************
  qgslayertreegroup.h
  --------------------------------------
  Date                 : May 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEGROUP_H
#define QGSLAYERTREEGROUP_H

#include "qgslayertreenode.h"

class QgsMapLayer;
class QgsLayerTreeLayer;

/**
 * Layer tree group node serves as a container for layers and further groups.
 *
 * @note added in 2.4
 */
class CORE_EXPORT QgsLayerTreeGroup : public QgsLayerTreeNode
{
    Q_OBJECT
  public:
    QgsLayerTreeGroup( const QString& name = QString(), Qt::CheckState checked = Qt::Checked );
    QgsLayerTreeGroup( const QgsLayerTreeGroup& other );

    QString name() const { return mName; }
    void setName( const QString& n ) { mName = n; }

    QgsLayerTreeGroup* addGroup( const QString& name );
    QgsLayerTreeLayer* insertLayer( int index, QgsMapLayer* layer );
    QgsLayerTreeLayer* addLayer( QgsMapLayer* layer );

    void insertChildNodes( int index, QList<QgsLayerTreeNode*> nodes );
    void insertChildNode( int index, QgsLayerTreeNode* node );
    void addChildNode( QgsLayerTreeNode* node );

    void removeChildNode( QgsLayerTreeNode* node );

    void removeLayer( QgsMapLayer* layer );

    void removeChildren( int from, int count );

    void removeAllChildren();

    QgsLayerTreeLayer* findLayer( const QString& layerId );
    QList<QgsLayerTreeLayer*> findLayers() const;
    QgsLayerTreeGroup* findGroup( const QString& name );

    static QgsLayerTreeGroup* readXML( QDomElement& element );
    virtual void writeXML( QDomElement& parentElement );

    void readChildrenFromXML( QDomElement& element );

    virtual QString dump() const;

    virtual QgsLayerTreeNode* clone() const;

    Qt::CheckState isVisible() const { return mChecked; }
    void setVisible( Qt::CheckState state );

    QStringList childLayerIds() const;

  protected slots:
    void layerDestroyed();
    void nodeVisibilityChanged( QgsLayerTreeNode* node );

  protected:
    void updateVisibilityFromChildren();

  protected:
    QString mName;
    Qt::CheckState mChecked;

    bool mChangingChildVisibility;
};


#endif // QGSLAYERTREEGROUP_H
