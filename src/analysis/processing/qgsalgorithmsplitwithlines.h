/***************************************************************************
                         qgsalgorithmsplitwithlines.h
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

#ifndef QGSALGORITHMSPLITWITHLINES_H
#define QGSALGORITHMSPLITWITHLINES_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE


/**
 * Native split with lines algorithm.
 */
class QgsSplitWithLinesAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsSplitWithLinesAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    Qgis::ProcessingAlgorithmDocumentationFlags documentationFlags() const override;
    QgsSplitWithLinesAlgorithm *createInstance() const override SIP_FACTORY;
    Qgis::ProcessingAlgorithmFlags flags() const override;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;
  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

///@endcond PRIVATE

#endif // QGSALGORITHMSPLITWITHLINES_H


