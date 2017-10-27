/***************************************************************************
                         qgsalgorithmjoinwithlines.h
                         ---------------------
    begin                : April 2017
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

#ifndef QGSALGORITHMJOINWITHLINES_H
#define QGSALGORITHMJOINWITHLINES_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native join by lines ("hub lines") algorithm.
 */
class QgsJoinWithLinesAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsJoinWithLinesAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    virtual QStringList tags() const override;
    QString group() const override;
    QString shortHelpString() const override;
    QgsJoinWithLinesAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

///@endcond PRIVATE

#endif // QGSALGORITHMJOINWITHLINES_H


