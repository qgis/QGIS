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
#include "qgsexpressioncontext.h"

/**
 * \ingroup core
 * \class QgsTrackedVectorLayerTools
 */
class CORE_EXPORT QgsTrackedVectorLayerTools : public QgsVectorLayerTools
{
    Q_OBJECT
  public:

    QgsTrackedVectorLayerTools() = default;

    /**
     * This method calls the addFeature method of the backend QgsVectorLayerTools
     *
     * \param layer           The layer to which the feature should be added
     * \param defaultValues   Default values for the feature to add
     * \param defaultGeometry A default geometry to add to the feature
     * \param feature         A pointer to the feature
     * \param context         A context object to be used for e.g. to calculate feature expression-based values (since QGIS 3.38)
     *
     * \returns               TRUE in case of success, FALSE if the operation failed/was aborted
     */
    bool addFeatureV2( QgsVectorLayer *layer, const QgsAttributeMap &defaultValues, const QgsGeometry &defaultGeometry, QgsFeature *feature SIP_OUT, const QgsVectorLayerToolsContext &context ) const override;
    bool startEditing( QgsVectorLayer *layer ) const override;
    bool stopEditing( QgsVectorLayer *layer, bool allowCancel ) const override;
    bool saveEdits( QgsVectorLayer *layer ) const override;
    bool copyMoveFeatures( QgsVectorLayer *layer, QgsFeatureRequest &request, double  dx = 0, double dy = 0, QString *errorMsg = nullptr, const bool topologicalEditing = false, QgsVectorLayer *topologicalLayer = nullptr, QString *childrenInfoMsg = nullptr ) const override;

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
