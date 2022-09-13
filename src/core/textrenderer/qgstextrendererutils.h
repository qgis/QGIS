/***************************************************************************
  qgstextrendererutils.h
  -----------------
   begin                : May 2020
   copyright            : (C) Nyall Dawson
   email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTEXTRENDERERUTILS_H
#define QGSTEXTRENDERERUTILS_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgstextbackgroundsettings.h"
#include "qgstextshadowsettings.h"
#include "qgstextmetrics.h"

/**
 * \class QgsTextRendererUtils
  * \ingroup core
  * \brief Utility functions for text rendering.
  * \since QGIS 3.10
 */
class CORE_EXPORT QgsTextRendererUtils
{
  public:

    /**
     * Decodes a string representation of a background shape type to a type.
     */
    static QgsTextBackgroundSettings::ShapeType decodeShapeType( const QString &string );

    /**
     * Decodes a string representation of a background size type to a type.
     */
    static QgsTextBackgroundSettings::SizeType decodeBackgroundSizeType( const QString &string );

    /**
     * Decodes a string representation of a background rotation type to a type.
     */
    static QgsTextBackgroundSettings::RotationType decodeBackgroundRotationType( const QString &string );

    /**
     * Decodes a string representation of a shadow placement type to a type.
     */
    static QgsTextShadowSettings::ShadowPlacement decodeShadowPlacementType( const QString &string );

    /**
     * Encodes a text \a orientation.
     * \returns encoded string
     * \see decodeTextOrientation()
     */
    static QString encodeTextOrientation( Qgis::TextOrientation orientation );

    /**
     * Attempts to decode a string representation of a text orientation.
     * \param name encoded text orientation name
     * \param ok if specified, will be set to TRUE if the name was successfully decoded
     * \returns decoded text orientation
     * \see encodeTextOrientation()
     */
    static Qgis::TextOrientation decodeTextOrientation( const QString &name, bool *ok = nullptr );

    /**
     * Converts a unit from an old (pre 3.0) label unit.
     *
     * \note Not available in Python bindings.
     * \since QGIS 3.14
     */
    static QgsUnitTypes::RenderUnit convertFromOldLabelUnit( int val ) SIP_SKIP;

    /**
     * Converts an encoded color value from a \a layer \a property.
     *
     * \note Not available in Python bindings.
     * \since QGIS 3.14
     */
    static QColor readColor( QgsVectorLayer *layer, const QString &property, const QColor &defaultColor = Qt::black, bool withAlpha = true ) SIP_SKIP;

#ifndef SIP_RUN

    /**
    * \class CurvedGraphemePlacement
    * \ingroup core
    * \brief Contains placement information for a single grapheme in a curved text layout.
    * \note Not available in Python bindings
    * \since QGIS 3.20
     */
    class CurvedGraphemePlacement
    {
      public:

        //! X coordinate of start of grapheme
        double x = 0;
        //! Y coordinate of start of grapheme
        double y = 0;
        //! Width of grapheme
        double width = 0;
        //! Height of grapheme
        double height = 0;
        //! Angle for grapheme, in radians
        double angle = 0;
        //! Index of corresponding grapheme
        int graphemeIndex = 0;
    };

    /**
    * \class CurvePlacementProperties
    * \ingroup core
    * \brief Contains placement information for a curved text layout.
    * \note Not available in Python bindings
    * \since QGIS 3.20
     */
    class CurvePlacementProperties
    {
      public:

        //! Placement information for all graphemes in text
        QVector< QgsTextRendererUtils::CurvedGraphemePlacement > graphemePlacement;
        //! Total count of upside down characters
        int upsideDownCharCount = 0;
        //! TRUE if labeled section of line is calculated to be of right-to-left orientation
        bool labeledLineSegmentIsRightToLeft = false;
        //! TRUE if the character placement had to be reversed in order to obtain upright labels on the segment
        bool flippedCharacterPlacementToGetUprightLabels = false;
    };

    /**
     * Controls behavior of curved text with respect to line directions
     */
    enum LabelLineDirection
    {
      RespectPainterOrientation, //!< Curved text will be placed respecting the painter orientation, and the actual line direction will be ignored
      FollowLineDirection //!< Curved text placement will respect the line direction and ignore painter orientation
    };

#if 0
    // TODO - refine API when used. We probably want to use QPolygonF here instead of QgsLineString!
    static CurvePlacementProperties *generateCurvedTextPlacement( const QgsPrecalculatedTextMetrics &metrics, const QgsLineString *line, double offsetAlongLine, LabelLineDirection direction = RespectPainterOrientation, double maxConcaveAngle = -1, double maxConvexAngle = -1, bool uprightOnly = true ) SIP_FACTORY;
#endif

    /**
     * Calculates curved text placement properties.
     *
     * \param metrics precalculated text metrics for text to render
     * \param x array of linestring x coordinates
     * \param y array of linestring y coordinates
     * \param numPoints number of points in \a x, \a y arrays
     * \param pathDistances vector of precalculated distances between vertices in \a x, \a y arrays
     * \param offsetAlongLine offset along line at which to start the curved text placement
     * \param direction controls placement of text with respect to painter orientation or line direction
     * \param maxConcaveAngle maximum angle between characters for concave text, or -1 if not set
     * \param maxConvexAngle maximum angle between characters for convex text, or -1 if not set
     * \param uprightOnly set to TRUE if text should be placed in an upright orientation only, or FALSE to allow upside down text placement
     *
     * \returns calculated placement properties, or NULLPTR if placement could not be calculated. Caller takes ownership of the returned placement.
     * \since QGIS 3.20
     */
    static CurvePlacementProperties *generateCurvedTextPlacement( const QgsPrecalculatedTextMetrics &metrics, const double *x, const double *y, int numPoints, const std::vector< double> &pathDistances, double offsetAlongLine, LabelLineDirection direction = RespectPainterOrientation, double maxConcaveAngle = -1, double maxConvexAngle = -1, bool uprightOnly = true ) SIP_SKIP;
#endif

  private:

    static CurvePlacementProperties *generateCurvedTextPlacementPrivate( const QgsPrecalculatedTextMetrics &metrics, const double *x, const double *y, int numPoints, const std::vector< double> &pathDistances, double offsetAlongLine, LabelLineDirection direction, double maxConcaveAngle = -1, double maxConvexAngle = -1, bool uprightOnly = true, bool isSecondAttempt = false ) SIP_SKIP;

    //! Returns TRUE if the next char position is found. The referenced parameters are updated.
    static bool nextCharPosition( double charWidth, double segmentLength, const double *x, const double *y, int numPoints, int &index, double &currentDistanceAlongSegment,
                                  double &characterStartX, double &characterStartY, double &characterEndX, double &characterEndY );

    static void findLineCircleIntersection( double cx, double cy, double radius,
                                            double x1, double y1, double x2, double y2,
                                            double &xRes, double &yRes );
};


#endif // QGSTEXTRENDERERUTILS_H
