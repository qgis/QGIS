/***************************************************************************
                         qgsalgorithmtransect.h
                         -------------------------
    begin                : October 2017
    copyright            : (C) 2017 by Loïc Bartoletti
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMTRANSECT_H
#define QGSALGORITHMTRANSECT_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsalgorithmtransectbase.h"

///@cond PRIVATE

/**
 * Native transect algorithm.
 */
class QgsTransectAlgorithm : public QgsTransectAlgorithmBase
{
  public:
    /**
     * Draw the transect on which side of the line
     */
    enum Side
    {
      Left,
      Right,
      Both
    };
    QgsTransectAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QString shortHelpString() const override;
    QgsTransectAlgorithm *createInstance() const override SIP_FACTORY;

  private:
    /**
     * Returns the transect of the point \a point with \a length, \a orientation and \a angle.
     * \param point The vertex
     * \param angleAtVertex Angle at the vertex
     * \param length Length of the transect Distance to extend line from input feature
     * \param orientation Orientation of the transect
     * \param angle Angle of the transect relative to the segment [\a p1 - \a p2] (degrees clockwise)
     */
    QgsGeometry calcTransect( const QgsPoint &point, double angleAtVertex, double length, Side orientation, double angle );
    void addAlgorithmParams() override;
    bool
      prepareAlgorithmTransectParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    std::vector<QgsPoint>
      generateSamplingPoints( const QgsLineString &line, const QVariantMap &parameters, QgsProcessingContext &context ) override;
    double calculateAzimuth( const QgsLineString &line, const QgsPoint &point, int pointIndex ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMTRANSECT_H
