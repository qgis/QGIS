/***************************************************************************
  qgslabelposition.h
  -------------------
   begin                : February 2021
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

#ifndef QGSLABELPOSITION_H
#define QGSLABELPOSITION_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include "qgspointxy.h"
#include "qgsrectangle.h"
#include "qgsgeometry.h"

#include <QFont>

/**
 * \ingroup core
 * \class QgsLabelPosition
 * \brief Represents the calculated placement of a map label.
 */
class CORE_EXPORT QgsLabelPosition
{
  public:

    /**
     * Constructor for QgsLabelPosition.
     * \param id associated feature ID
     * \param r label rotation in degrees clockwise
     * \param corners corner points of label bounding box, in map units
     * \param rect label bounding box, in map units
     * \param w width of label, in map units
     * \param h height of label, in map units
     * \param layer ID of associated map layer
     * \param labeltext text rendered for label
     * \param labelfont font used to render label
     * \param upside_down TRUE if label is upside down
     * \param diagram TRUE if label is a diagram
     * \param pinned TRUE if label has pinned placement
     * \param providerId ID of associated label provider
     * \param labelGeometry polygon geometry of label boundary
     * \param isUnplaced set to TRUE if label was unplaced (e.g. due to collisions with other labels)
     */
    QgsLabelPosition( QgsFeatureId id, double r, const QVector< QgsPointXY > &corners, const QgsRectangle &rect, double w, double h, const QString &layer, const QString &labeltext, const QFont &labelfont, bool upside_down, bool diagram = false, bool pinned = false, const QString &providerId = QString(),
                      const QgsGeometry &labelGeometry = QgsGeometry(), bool isUnplaced = false )
      : featureId( id )
      , rotation( r )
      , cornerPoints( corners )
      , labelRect( rect )
      , labelGeometry( labelGeometry )
      , width( w )
      , height( h )
      , layerID( layer )
      , labelText( labeltext )
      , labelFont( labelfont )
      , upsideDown( upside_down )
      , isDiagram( diagram )
      , isPinned( pinned )
      , providerID( providerId )
      , isUnplaced( isUnplaced )
    {}

    //! Constructor for QgsLabelPosition
    QgsLabelPosition() = default;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    const QString text = sipCpp->labelText;
    QString str = QStringLiteral( "<QgsLabelPosition: \"%1\"%2>" ).arg( text, sipCpp->isUnplaced ? QStringLiteral( " (unplaced)" ) : QString() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * ID of feature associated with this label.
     */
    QgsFeatureId featureId = FID_NULL;

    /**
     * Rotation of label, in degrees clockwise.
     */
    double rotation = 0;

    QVector< QgsPointXY > cornerPoints;
    QgsRectangle labelRect;

    /**
     * A polygon geometry representing the label's bounds in map coordinates.
     * \since QGIS 3.4.9
     */
    QgsGeometry labelGeometry;

    /**
     * Width of label bounding box, in map units.
     */
    double width = 0;

    /**
     * Heeght of label bounding box, in map units.
     */
    double height = 0;

    /**
     * ID of associated map layer.
     */
    QString layerID;

    /**
     * String shown in label.
     */
    QString labelText;

    /**
     * Font which the label is rendered using.
     */
    QFont labelFont;

    /**
     * TRUE if label is upside down.
     */
    bool upsideDown = false;

    /**
     * TRUE if label is a diagram.
     */
    bool isDiagram = false;

    /**
     * TRUE if label position has been pinned.
     */
    bool isPinned = false;

    /**
     * ID of the associated label provider.
     * \since QGIS 2.14
     */
    QString providerID;

    /**
     * TRUE if label position corresponds to an unplaced label.
     * \since QGIS 3.10
     */
    bool isUnplaced = false;

    /**
     * If non zero, indicates that the label position is part of a group of label positions (i.e. a character in a curved label).
     *
     * All other linked positions will share the same groupedLabelId.
     */
    long long groupedLabelId = 0;
};

#endif // QGSLABELPOSITION_H
