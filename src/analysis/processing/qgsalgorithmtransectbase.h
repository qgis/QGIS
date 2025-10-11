/***************************************************************************
                         qgsalgorithmtransectbase.h
                         -------------------------
    begin                : September 2025
    copyright            : (C) 2025 by Loïc Bartoletti
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

#ifndef QGSALGORITHMTRANSECTBASE_H
#define QGSALGORITHMTRANSECTBASE_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Base class for transect algorithms.
 */
class QgsTransectAlgorithmBase : public QgsProcessingAlgorithm
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

    QString group() const final;
    QString groupId() const final;
    QStringList tags() const override;
    QString shortDescription() const override;
    Qgis::ProcessingAlgorithmDocumentationFlags documentationFlags() const final;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) final;

  protected:
    /**
     * Adds specific subclass algorithm parameters. The common parameters (INPUT, LENGTH, ANGLE, SIDE, OUTPUT)
     * are automatically added by the base class.
     */
    virtual void addAlgorithmParams() = 0;

    /**
     * Prepares the transect algorithm subclass for execution.
     */
    virtual bool prepareAlgorithmTransectParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) = 0;

    /**
     * Processes a line geometry using the specific sampling strategy implemented in subclasses.
     */
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) final;

    /**
     * Pure virtual method that generates sampling points along a line geometry.
     * Subclasses implement their specific sampling strategy here.
     */
    virtual std::vector<QgsPoint> generateSamplingPoints( const QgsLineString &line, const QVariantMap &parameters, QgsProcessingContext &context ) = 0;

    /**
     * Calculate the azimuth at a given point for transect orientation.
     * Subclasses can override this if they need different azimuth calculation.
     */
    virtual double calculateAzimuth( const QgsLineString &line, const QgsPoint &point, int pointIndex ) = 0;

    /**
     * Returns the transect geometry at the specified point.
     */
    static QgsGeometry calcTransect( const QgsPoint &point, double angleAtVertex, double length, Side orientation, double angle );

    // Shared member variables accessible to subclasses
    Side mOrientation = Both;
    double mAngle = 90.0;
    double mLength = 5.0;
    bool mDynamicAngle = false;
    bool mDynamicLength = false;
    QgsProperty mAngleProperty;
    QgsProperty mLengthProperty;
};

///@endcond PRIVATE

#endif // QGSALGORITHMTRANSECTBASE_H
