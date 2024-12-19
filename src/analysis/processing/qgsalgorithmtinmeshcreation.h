/***************************************************************************
                         qgsalgorithmtinmeshcreation.h
                         ---------------------------
    begin                : August 2020
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

#ifndef QGSALGORITHMTINMESHCREATION_H
#define QGSALGORITHMTINMESHCREATION_H

#define SIP_NO_FILE

#include "qgsprocessingalgorithm.h"

///@cond PRIVATE
struct QgsMesh;

class QgsTinMeshCreationAlgorithm: public QgsProcessingAlgorithm
{
  public:

    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QString name() const override;
    QString displayName() const override;
    bool canExecute( QString *errorMessage ) const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;

    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QStringList mAvailableFormat;
    QMap<QString, QString> mDriverSuffix;

    struct Layer
    {
      QgsFeatureIterator fit;
      QgsCoordinateTransform transform;
      int attributeIndex;
      long long featureCount;
    };

    QList<Layer> mVerticesLayer;
    QList<Layer> mBreakLinesLayer;

    void addZValueDataset( const QString &fileName, const QgsMesh &mesh, const QString &driver );
};

///@endcond PRIVATE

#endif // QGSALGORITHMTINMESHCREATION_H
