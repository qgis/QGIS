/***************************************************************************
                              qgstininterpolator.h
                              --------------------
  begin                : March 10, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTININTERPOLATOR_H
#define QGSTININTERPOLATOR_H

#include "qgsinterpolator.h"
#include <QString>
#include "qgis_analysis.h"

class QgsFeatureSink;
class Triangulation;
class TriangleInterpolator;
class QgsFeature;
class QgsFeedback;
class QgsFields;

/**
 * \ingroup analysis
 *  Interpolation in a triangular irregular network*/
class ANALYSIS_EXPORT QgsTINInterpolator: public QgsInterpolator
{
  public:
    //describes the type of interpolation
    enum TINInterpolation
    {
      Linear,
      CloughTocher
    };

    /**
     * Constructor for QgsTINInterpolator.
     * The \a feedback object specifies an optional QgsFeedback object for progress reports and cancelation support.
     * Ownership of \a feedback is not transferred and callers must ensure that it exists for the lifetime of this object.
     */
    QgsTINInterpolator( const QList<QgsInterpolator::LayerData> &inputData, TINInterpolation interpolation = Linear, QgsFeedback *feedback = nullptr );
    ~QgsTINInterpolator();

    /**
     * Calculates interpolation value for map coordinates x, y
       \param x x-coordinate (in map units)
       \param y y-coordinate (in map units)
       \param result out: interpolation result
       \returns 0 in case of success*/
    int interpolatePoint( double x, double y, double &result ) override;

    /**
     * Returns the fields output by features when saving the triangulation.
     * These fields should be used when creating
     * a suitable feature sink for setTriangulationSink()
     * \see setTriangulationSink()
     * \since QGIS 3.0
     */
    static QgsFields triangulationFields();

    /**
     * Sets the optional \a sink for saving the triangulation features.
     *
     * The sink must be setup to accept LineString features, with fields matching
     * those returned by triangulationFields().
     *
     * \see triangulationFields()
     *  \since QGIS 3.0
     */
    void setTriangulationSink( QgsFeatureSink *sink );

  private:
    Triangulation *mTriangulation = nullptr;
    TriangleInterpolator *mTriangleInterpolator = nullptr;
    bool mIsInitialized;
    QgsFeedback *mFeedback = nullptr;

    //! Feature sink for triangulation
    QgsFeatureSink *mTriangulationSink = nullptr;
    //! Type of interpolation
    TINInterpolation mInterpolation;

    //! Create dual edge triangulation
    void initialize();

    /**
     * Inserts the vertices of a feature into the triangulation
      \param f the feature
      \param zCoord true if the z coordinate is the interpolation attribute
      \param attr interpolation attribute index (if zCoord is false)
      \param type point/structure line, break line
      \returns 0 in case of success, -1 if the feature could not be inserted because of numerical problems*/
    int insertData( QgsFeature *f, bool zCoord, int attr, InputType type );
};

#endif
