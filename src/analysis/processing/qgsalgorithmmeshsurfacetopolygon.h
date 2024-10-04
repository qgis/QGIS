/***************************************************************************
                         qgsalgorithmmeshsurfacetopolygon.h
                         ---------------------------
    begin                : September 2024
    copyright            : (C) 2024 by Jan Caha
    email                : jan.caha at outlook dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMMESHCONVEXHULL_H
#define QGSALGORITHMMESHCONVEXHULL_H

#define SIP_NO_FILE

#include "qgsprocessingalgorithm.h"
#include "qgsmeshdataprovider.h"
#include "qgstriangularmesh.h"

///@cond PRIVATE

class QgsMeshSurfaceToPolygonAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsMeshSurfaceToPolygonAlgorithm() = default;
    QString shortHelpString() const override;
    QString name() const override;
    QString displayName() const override;
    QString group() const override;
    QString groupId() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsProcessingAlgorithm *createInstance() const override SIP_FACTORY;

  private:
    QgsMesh mNativeMesh;
    QgsCoordinateTransform mTransform;
};

///@endcond PRIVATE

#endif // QGSALGORITHMMESHCONVEXHULL_H
