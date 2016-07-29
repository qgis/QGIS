/***************************************************************************
  qgstrackedvectorlayertools.sip - QgsTrackedVectorLayerTools

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
class QgsTrackedVectorLayerTools : QgsVectorLayerTools
{
%TypeHeaderCode
#include "qgstrackedvectorlayertools.h"
%End
  public:
    QgsTrackedVectorLayerTools();

    bool addFeature( QgsVectorLayer* layer, const QgsAttributeMap& defaultValues, const QgsGeometry& defaultGeometry, QgsFeature* feature ) const;
    bool startEditing( QgsVectorLayer* layer ) const ;
    bool stopEditing( QgsVectorLayer* layer, bool allowCancel ) const ;
    bool saveEdits( QgsVectorLayer* layer ) const ;

    /**
     * Set the vector layer tools that will be used to interact with the data
     */
    void setVectorLayerTools(const QgsVectorLayerTools* tools );

    /**
     * Delete all features which have been added via this object.
     */
    void rollback();
};
