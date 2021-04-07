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
#include "qgstextformat.h"
#include "qgstextrenderer.h"

#ifndef SIP_RUN
#include <optional>
#endif

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
    static QString encodeTextOrientation( QgsTextFormat::TextOrientation orientation );

    /**
     * Attempts to decode a string representation of a text orientation.
     * \param name encoded text orientation name
     * \param ok if specified, will be set to TRUE if the name was successfully decoded
     * \returns decoded text orientation
     * \see encodeTextOrientation()
     */
    static QgsTextFormat::TextOrientation decodeTextOrientation( const QString &name, bool *ok = nullptr );

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


    class CurvedGraphemePlacement
    {
      public:

        double x;
        double y;
        double width;
        double height;
        double angle; //in radians
        int partId;
    };

    class CurvePlacementProperties
    {
      public:
        QVector< QgsTextRendererUtils::CurvedGraphemePlacement > graphemePlacement;
        int upsideDownCharCount = 0;
        bool flip = false;
        bool reversed = false;
        int orientation = 0;

    };

    static CurvePlacementProperties *generateCurvedTextPlacement( const QgsPrecalculatedTextMetrics &metrics, const QgsLineString *line, double offsetAlongLine, int orientation = 0, double maxConcaveAngle = -1, double maxConvexAngle = -1, bool uprightOnly = true ) SIP_FACTORY;

    static CurvePlacementProperties *generateCurvedTextPlacement( const QgsPrecalculatedTextMetrics &metrics, const double *x, const double *y, int numPoints, const std::vector< double> &pathDistances, double offsetAlongLine, int orientation = 0, double maxConcaveAngle = -1, double maxConvexAngle = -1, bool uprightOnly = true ) SIP_SKIP;

  private:


    //! Returns TRUE if the next char position is found. The referenced parameters are updated.
    static bool nextCharPosition( double charWidth, double segmentLength, const double *x, const double *y, int numPoints, int &index, double &currentDistanceAlongSegment,
                                  double &characterStartX, double &characterStartY, double &characterEndX, double &characterEndY );

    static void findLineCircleIntersection( double cx, double cy, double radius,
                                            double x1, double y1, double x2, double y2,
                                            double &xRes, double &yRes );
};


#endif // QGSTEXTRENDERERUTILS_H
