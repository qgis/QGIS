/***************************************************************************
                         qgsalgorithmd8base.h
                         ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#ifndef QGSALGORITHMD8BASE_H
#define QGSALGORITHMD8BASE_H

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

#define SIP_NO_FILE

///@cond PRIVATE

/**
 * \brief Common base class for processing D8 hydrological algorithms.
 */
class QgsD8AnalysisAlgorithmBase : public QgsProcessingAlgorithm
{
  public:
    QgsD8AnalysisAlgorithmBase() = default;

    QString group() const override;
    QString groupId() const override;
    QList<QgsAcademicReference> academicReferences() const override;
    QList<QgsProcessingAlgorithm::ExternalLink> externalLinks() const override;

  protected:
    /**
     * Executes the stack-based Strahler stream ordering algorithm across the channel network using a D8 direction matrix.
     */
    void computeStrahlerOrder(
      const QgsRasterBlock *demBlock, const std::vector<int8_t> &d8Directions, int width, int height, int threshold, int16_t *outOrder, QgsProcessingFeedback *feedback, int outputNoData
    );
};

#endif // QGSALGORITHMD8BASE_H
