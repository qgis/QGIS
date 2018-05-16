/***************************************************************************
                         qgsmeshrenderersettings.h
                         -------------------------
    begin                : May 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHRENDERERSETTINGS_H
#define QGSMESHRENDERERSETTINGS_H

#include <QColor>
#include <limits>

#include "qgis_core.h"
#include "qgis.h"

/**
 * \ingroup core
 *
 * Represents a mesh renderer settings for mesh object
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshRendererMeshSettings
{
  public:
    //! Returns whether mesh structure rendering is enabled
    bool isEnabled() const;
    //! Sets whether mesh structure rendering is enabled
    void setEnabled( bool enabled );

    //! Returns line width used for rendering (in millimeters)
    double lineWidth() const;
    //! Sets line width used for rendering (in millimeters)
    void setLineWidth( double lineWidth );

    //! Returns color used for rendering
    QColor color() const;
    //! Sets color used for rendering of the mesh
    void setColor( const QColor &color );

  private:
    bool mEnabled = false;
    double mLineWidth = DEFAULT_LINE_WIDTH;
    QColor mColor = Qt::black;
};

/**
 * \ingroup core
 *
 * Represents a mesh renderer settings for scalar datasets
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshRendererScalarSettings
{
  public:
    //! Returns color representing maximum scalar value in the dataset
    QColor maxColor() const;
    //! Sets color representing maximum scalar value in the dataset
    void setMaxColor( const QColor &maxColor );

    //! Returns color representing minimum scalar value in the dataset
    QColor minColor() const;
    //! Sets color representing maximum scalar value in the dataset
    void setMinColor( const QColor &minColor );

    /**
     * Returns min scalar value that represents minColor()
     *
     * if set to numerical_limits<double>::quiet_NaN(), value for minColor() is
     * taken from minimum value of active scalar dataset
     */
    double minValue() const;

    /**
     * Sets min scalar value that represents minColor()
     * \see QgsMeshRendererScalarSettings::minValue()
     */
    void setMinValue( double minValue );

    /**
     * Returns max scalar value that represents maxColor()
     *
     * if set to numerical_limits<double>::quiet_NaN(), value for maxColor() is
     * taken from maximum value of active scalar dataset
     */
    double maxValue() const;

    /**
     * Sets min scalar value that represents minColor()
     * \see QgsMeshRendererScalarSettings::maxValue()
     */
    void setMaxValue( double maxValue );

  private:
    QColor mMaxColor = QColor::fromRgb( 255, 0, 0 );
    QColor mMinColor = QColor::fromRgb( 0, 0, 255 );
    double mMaxValue = std::numeric_limits<double>::quiet_NaN(); //disabled
    double mMinValue = std::numeric_limits<double>::quiet_NaN(); //disabled
};

/**
 * \ingroup core
 *
 * Represents a mesh renderer settings for vector datasets
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshRendererVectorSettings
{
  public:
    //! Algorithm how to transform vector magnitude to length of arrow on the device in pixels
    enum ArrowScalingMethod
    {

      /**
       * Scale vector magnitude linearly to fit in range of vectorFilterMin() and vectorFilterMax()
       */
      MinMax = 0,

      /**
       * Scale vector magnitude by factor scaleFactor()
       */
      Scaled,

      /**
       * Use fixed length fixedShaftLength() regardless of vector's magnitude
       */
      Fixed
    };

    //! Returns line width of the arrow (in millimeters)
    double lineWidth() const;
    //! Sets line width of the arrow in pixels (in millimeters)
    void setLineWidth( double lineWidth );

    //! Returns color used for drawing arrows
    QColor color() const;
    //! Sets color used for drawing arrows
    void setColor( const QColor &color );

    /**
     * Returns filter value for vector magnitudes.
     *
     * If magnitude of the vector is lower than this value, the vector is not
     * drawn. -1 represents that filtering is not active.
     */
    double filterMin() const;

    /**
     * Sets filter value for vector magnitudes.
     * \see QgsMeshRendererVectorSettings::vectorFilterMin()
     */
    void setFilterMin( double filterMin );

    /**
     * Returns filter value for vector magnitudes.
     *
     * If magnitude of the vector is higher than this value, the vector is not
     * drawn. -1 represents that filtering is not active.
     */
    double filterMax() const;

    /**
     * Sets filter value for vector magnitudes.
     * \see QgsMeshRendererVectorSettings::vectorFilterMax()
     */
    void setFilterMax( double filterMax );

    //! Returns method used for drawing arrows
    QgsMeshRendererVectorSettings::ArrowScalingMethod shaftLengthMethod() const;
    //! Sets method used for drawing arrows
    void setShaftLengthMethod( const QgsMeshRendererVectorSettings::ArrowScalingMethod &shaftLengthMethod );

    /**
     * Returns mininimum shaft length (in millimeters)
     *
     * Only for QgsMeshRendererVectorSettings::ArrowScalingMethod::MinMax
     */
    double minShaftLength() const;

    /**
     * Sets mininimum shaft length (in millimeters)
     *
     * Only for QgsMeshRendererVectorSettings::ArrowScalingMethod::MinMax
     */
    void setMinShaftLength( double minShaftLength );

    /**
     * Returns maximum shaft length (in millimeters)
     *
     * Only for QgsMeshRendererVectorSettings::ArrowScalingMethod::MinMax
     */
    double maxShaftLength() const;

    /**
     * Sets maximum shaft length (in millimeters)
     *
     * Only for QgsMeshRendererVectorSettings::ArrowScalingMethod::MinMax
     */
    void setMaxShaftLength( double maxShaftLength );

    /**
     * Returns scale factor
     *
     * Only for QgsMeshRendererVectorSettings::ArrowScalingMethod::Scaled
     */
    double scaleFactor() const;

    /**
     * Sets scale factor
     *
     * Only for QgsMeshRendererVectorSettings::ArrowScalingMethod::Scaled
     */
    void setScaleFactor( double scaleFactor );

    /**
     * Returns fixed arrow length (in millimeters)
     *
     * Only for QgsMeshRendererVectorSettings::ArrowScalingMethod::Fixed
     */
    double fixedShaftLength() const;

    /**
     * Sets fixed length  (in millimeters)
     *
     * Only for QgsMeshRendererVectorSettings::ArrowScalingMethod::Fixed
     */
    void setFixedShaftLength( double fixedShaftLength );

    //! Returns ratio of the head width of the arrow (range 0-1)
    double arrowHeadWidthRatio() const;
    //! Sets ratio of the head width of the arrow (range 0-1)
    void setArrowHeadWidthRatio( double arrowHeadWidthRatio );

    //! Returns ratio of the head length of the arrow (range 0-1)
    double arrowHeadLengthRatio() const;
    //! Sets ratio of the head length of the arrow (range 0-1)
    void setArrowHeadLengthRatio( double arrowHeadLengthRatio );

  private:
    double mLineWidth = DEFAULT_LINE_WIDTH; //in milimeters
    QColor mColor = Qt::black;
    double mFilterMin = -1; //disabled
    double mFilterMax = -1; //disabled
    QgsMeshRendererVectorSettings::ArrowScalingMethod mShaftLengthMethod = QgsMeshRendererVectorSettings::ArrowScalingMethod::MinMax;
    double mMinShaftLength = 0.8; //in milimeters
    double mMaxShaftLength = 10; //in milimeters
    double mScaleFactor = 10;
    double mFixedShaftLength = 20; //in milimeters
    double mArrowHeadWidthRatio = 0.15;
    double mArrowHeadLengthRatio = 0.40;
};

#endif //QGSMESHRENDERERSETTINGS_H
