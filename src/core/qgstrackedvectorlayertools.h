/***************************************************************************
  qgstrackedvectorlayertools.h - QgsTrackedVectorLayerTools

 ---------------------
 begin                : 16.5.2016
 copyright            : (C) 2016 by Matthias Kuhn, OPENGIS.ch
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSTRACKEDVECTORLAYERTOOLS_H
#define QGSTRACKEDVECTORLAYERTOOLS_H

#include "qgis_core.h"
#include "qgsvectorlayertools.h"

/**
 * \ingroup core
 * \class QgsTrackedVectorLayerTools
 */
class CORE_EXPORT QgsTrackedVectorLayerTools : public QgsVectorLayerTools
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsTrackedVectorLayerTools.
     */
    QgsTrackedVectorLayerTools() = default;

    bool addFeature( QgsVectorLayer *layer, const QgsAttributeMap &defaultValues, const QgsGeometry &defaultGeometry, QgsFeature *feature ) const override;
    bool startEditing( QgsVectorLayer *layer ) const override;
    bool stopEditing( QgsVectorLayer *layer, bool allowCancel ) const override;
    bool saveEdits( QgsVectorLayer *layer ) const override;
    bool copyMoveFeatures( QgsVectorLayer *layer, QgsFeatureRequest &request, double  dx = 0, double dy = 0, QString *errorMsg = nullptr ) const override;

    /**
     * Set the vector layer tools that will be used to interact with the data
     */
    void setVectorLayerTools( const QgsVectorLayerTools *tools );

    /**
     * Delete all features which have been added via this object.
     */
    void rollback();

  private:

    const QgsVectorLayerTools *mBackend = nullptr;
    // TODO QGIS3: remove mutable once methods are no longer const
    mutable QMap<QgsVectorLayer *, QgsFeatureIds> mAddedFeatures;
};

#endif // QGSTRACKEDVECTORLAYERTOOLS_H
