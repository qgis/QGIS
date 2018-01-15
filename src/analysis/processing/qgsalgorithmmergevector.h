/***************************************************************************
                         qgsalgorithmmergevector.h
                         ------------------
    begin                : December 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMMERGEVECTOR_H
#define QGSALGORITHMMERGEVECTOR_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native vector layer merge algorithm.
 */
class QgsMergeVectorAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsMergeVectorAlgorithm() = default;
    QgsProcessingAlgorithm::Flags flags() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsMergeVectorAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

///@endcond PRIVATE

#endif // QGSALGORITHMMERGEVECTOR_H


