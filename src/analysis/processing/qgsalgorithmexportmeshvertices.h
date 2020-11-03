/***************************************************************************
                         qgsalgorithmtinmeshcreation.h
                         ---------------------------
    begin                : October 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMEXPORTMESHVERTICES_H
#define QGSALGORITHMEXPORTMESHVERTICES_H

#define SIP_NO_FILE

#include "qgsprocessingalgorithm.h"
#include "qgsmeshdataset.h"
#include "qgsmeshdataprovider.h"

///@cond PRIVATE

class QgsExportMeshVerticesAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString name() const override;
    QString displayName() const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;

    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QgsMesh mNativeMesh;

    struct DataGroup
    {
      QgsMeshDatasetGroupMetadata metadata;
      QgsMeshDataBlock datasetValues;
      QgsMeshDataBlock activeFaceFlagValues;
    };

    QList<DataGroup> mDataPerGroup;
    QgsCoordinateTransform mTransform;
    int mExportVectorOption = 2;
};

///@endcond PRIVATE

#endif // QGSALGORITHMEXPORTMESHVERTICES_H
