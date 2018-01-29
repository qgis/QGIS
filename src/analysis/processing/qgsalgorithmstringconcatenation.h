/***************************************************************************
                         qgsalgorithmstringconcatenation.h
                         ---------------------
    begin                : October 2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMSTRINGCONCATENATION_H
#define QGSALGORITHMSTRINGCONCATENATION_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native string concatenation algorithm.
 */
class QgsStringConcatenationAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsStringConcatenationAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    Flags flags() const override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsStringConcatenationAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback * ) override;

};

///@endcond PRIVATE

#endif // QGSALGORITHMSTRINGCONCATENATION_H
