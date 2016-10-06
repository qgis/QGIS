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

#include "qgsvectorlayertools.h"

/** \ingroup gui
 * \class QgsTrackedVectorLayerTools
 */
class GUI_EXPORT QgsTrackedVectorLayerTools : public QgsVectorLayerTools
{
  public:
    QgsTrackedVectorLayerTools();

    bool addFeature( QgsVectorLayer* layer, const QgsAttributeMap& defaultValues, const QgsGeometry& defaultGeometry, QgsFeature* feature ) const override;
    bool startEditing( QgsVectorLayer* layer ) const override;
    bool stopEditing( QgsVectorLayer* layer, bool allowCancel ) const override;
    bool saveEdits( QgsVectorLayer* layer ) const override;

    /**
     * Set the vector layer tools that will be used to interact with the data
     */
    void setVectorLayerTools( const QgsVectorLayerTools* tools );

    /**
     * Delete all features which have been added via this object.
     */
    void rollback();

  private:

    const QgsVectorLayerTools* mBackend;
    // TODO QGIS3: remove mutable once methods are no longer const
    mutable QMap<QgsVectorLayer*, QgsFeatureIds> mAddedFeatures;
};

#endif // QGSTRACKEDVECTORLAYERTOOLS_H
