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

/** \ingroup core
 * \class QgsVectorLayerEditPassthrough
 */
class CORE_EXPORT QgsVectorLayerEditPassthrough : public QgsVectorLayerEditBuffer
{
    Q_OBJECT
  public:
    QgsVectorLayerEditPassthrough( QgsVectorLayer* layer );
    bool isModified() const override;
    bool addFeature( QgsFeature& f ) override;
    bool addFeatures( QgsFeatureList& features ) override;
    bool deleteFeature( QgsFeatureId fid ) override;
    bool deleteFeatures( const QgsFeatureIds& fids ) override;
    bool changeGeometry( QgsFeatureId fid, QgsGeometry* geom ) override;
    bool changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &oldValue = QVariant() ) override;
    bool addAttribute( const QgsField &field ) override;
    bool deleteAttribute( int attr ) override;
    bool renameAttribute( int attr, const QString& newName ) override;
    bool commitChanges( QStringList& commitErrors ) override;
    void rollBack() override;

  private:
    bool mModified;
};

#endif // QGSVECTORLAYEREDITPASSTHROUGH_H
