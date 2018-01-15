/***************************************************************************
                         qgsalgorithmorderbyexpression.h
                         ---------------------
    begin                : November 2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne dot trimaille at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#ifndef QGSALGORITHMORDERBYEXPRESSION_H
#define QGSALGORITHMORDERBYEXPRESSION_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native order by expression algorithm.
 */
class QgsOrderByExpressionAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsOrderByExpressionAlgorithm() = default;
    Flags flags() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsOrderByExpressionAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMORDERBYEXPRESSION_H
