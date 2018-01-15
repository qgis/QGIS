/***************************************************************************
                         qgsalgorithmextractnodes.h
                         -------------------------
    begin                : November 2017
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMEXTRACTNODES_H
#define QGSALGORITHMEXTRACTNODES_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native extract nodes algorithm.
 */
class QgsExtractNodesAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsExtractNodesAlgorithm() = default;
    Flags flags() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsExtractNodesAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

///@endcond PRIVATE

#endif // QGSALGORITHMEXTRACTNODES_H


