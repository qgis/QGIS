/***************************************************************************
    qgsvectorfieldarrowsettings.h
    ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORFIELDARROWSETTINGS_H
#define QGSVECTORFIELDARROWSETTINGS_H

#include "qgis_core.h"

#include <QDomElement>

/**
 * \ingroup core
 *
 * \brief Represents a mesh renderer settings for vector datasets displayed with arrows.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVectorFieldArrowSettings
{
  public:
    //! Algorithm how to transform vector magnitude to length of arrow on the device in pixels
    enum class ArrowScalingMethod
    {
      MinMax = 0, //!< Scale vector magnitude linearly to fit in range of vectorFilterMin() and vectorFilterMax()
      Scaled,     //!< Scale vector magnitude by factor scaleFactor()
      Fixed       //!< Use fixed length fixedShaftLength() regardless of vector's magnitude
    };

    //! Returns method used for drawing arrows
    QgsVectorFieldArrowSettings::ArrowScalingMethod shaftLengthMethod() const;
    //! Sets method used for drawing arrows
    void setShaftLengthMethod( ArrowScalingMethod shaftLengthMethod );

    /**
     * Returns mininimum shaft length (in millimeters)
     *
     * Only for QgsVectorFieldArrowSettings::ArrowScalingMethod::MinMax
     */
    double minShaftLength() const;

    /**
     * Sets mininimum shaft length (in millimeters)
     *
     * Only for QgsVectorFieldArrowSettings::ArrowScalingMethod::MinMax
     */
    void setMinShaftLength( double minShaftLength );

    /**
     * Returns maximum shaft length (in millimeters)
     *
     * Only for QgsVectorFieldArrowSettings::ArrowScalingMethod::MinMax
     */
    double maxShaftLength() const;

    /**
     * Sets maximum shaft length (in millimeters)
     *
     * Only for QgsVectorFieldArrowSettings::ArrowScalingMethod::MinMax
     */
    void setMaxShaftLength( double maxShaftLength );

    /**
     * Returns scale factor
     *
     * Only for QgsVectorFieldArrowSettings::ArrowScalingMethod::Scaled
     */
    double scaleFactor() const;

    /**
     * Sets scale factor
     *
     * Only for QgsVectorFieldArrowSettings::ArrowScalingMethod::Scaled
     */
    void setScaleFactor( double scaleFactor );

    /**
     * Returns fixed arrow length (in millimeters)
     *
     * Only for QgsVectorFieldArrowSettings::ArrowScalingMethod::Fixed
     */
    double fixedShaftLength() const;

    /**
     * Sets fixed length  (in millimeters)
     *
     * Only for QgsVectorFieldArrowSettings::ArrowScalingMethod::Fixed
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

    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc ) const;
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem );

  private:
    QgsVectorFieldArrowSettings::ArrowScalingMethod mShaftLengthMethod = QgsVectorFieldArrowSettings::ArrowScalingMethod::MinMax;
    double mMinShaftLength = 0.8; //in millimeters
    double mMaxShaftLength = 10;  //in millimeters
    double mScaleFactor = 10;
    double mFixedShaftLength = 20; //in millimeters
    double mArrowHeadWidthRatio = 0.15;
    double mArrowHeadLengthRatio = 0.40;
};

#endif // QGSVECTORFIELDARROWSETTINGS_H
