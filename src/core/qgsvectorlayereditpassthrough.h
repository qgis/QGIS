/***************************************************************************
    qgsvectorlayereditpassthrough.h
    ---------------------
    begin                : Jan 12 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : manisandro at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVECTORLAYEREDITPASSTHROUGH_H
#define QGSVECTORLAYEREDITPASSTHROUGH_H

#include "qgsvectorlayereditbuffer.h"

class QgsVectorLayer;

class CORE_EXPORT QgsVectorLayerEditPassthrough : public QgsVectorLayerEditBuffer
{
    Q_OBJECT
  public:
    QgsVectorLayerEditPassthrough( QgsVectorLayer* layer ) { L = layer; }
    bool isModified() const { return false; }
    bool addFeature( QgsFeature& f );
    bool addFeatures( QgsFeatureList& features );
    bool deleteFeature( QgsFeatureId fid );
    bool changeGeometry( QgsFeatureId fid, QgsGeometry* geom );
    bool changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &oldValue = QVariant() );
    bool addAttribute( const QgsField &field );
    bool deleteAttribute( int attr );
    bool commitChanges( QStringList& commitErrors );
    void rollBack();

};

#endif // QGSVECTORLAYEREDITPASSTHROUGH_H
