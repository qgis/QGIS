/***************************************************************************
                        qgsalgorithmcheckgeometrydangle.h
                        ---------------------
   begin                : November 2023
   copyright            : (C) 2023 by Loïc Bartoletti
   email                : loic dot bartoletti at oslandia dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMCHECKGEOMETRYDANGLE_H
#define QGSALGORITHMCHECKGEOMETRYDANGLE_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsfeaturepool.h"

///@cond PRIVATE

class QgsGeometryCheckDangleAlgorithm : public QgsProcessingAlgorithm
{
  public:

    QgsGeometryCheckDangleAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    Flags flags() const override;
    QgsGeometryCheckDangleAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  private:
    QgsFeaturePool *createFeaturePool( QgsVectorLayer *layer, bool selectedOnly = false ) const;

    std::unique_ptr< QgsVectorLayer > mInputLayer;
    int mTolerance{8};
    bool mIsInPlace{false};
};

///@endcond PRIVATE

#endif // QGSALGORITHMCHECKGEOMETRYDANGLE_H
