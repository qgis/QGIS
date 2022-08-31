/***************************************************************************
    qgstextpreview.h
    ----------------
    begin                : October 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSTEXTPREVIEW_H
#define QGSTEXTPREVIEW_H

#include "qgstextformat.h"
#include "qgsrendercontext.h"

#include <QLabel>
#include "qgis_gui.h"

class QgsScreenHelper;

/**
 * \class QgsTextPreview
 * \ingroup gui
 * \brief A widget for previewing text formatting settings.
 *
 * QgsTextPreview provides a widget for previewing the appearance of text rendered
 * using QgsTextRenderer. The preview includes all settings contained within
 * a QgsTextFormat, including shadow, background and buffer.
 *
 * In order to preview the exact appearance of text which uses sizes in map units,
 * the scale and map units must be set by calling setScale() and setMapUnits().
 *
 * \since QGIS 3.0
 */

class GUI_EXPORT QgsTextPreview : public QLabel
{
    Q_OBJECT

    Q_PROPERTY( QgsTextFormat format READ format WRITE setFormat )
    Q_PROPERTY( double scale READ scale WRITE setScale )
    Q_PROPERTY( QgsUnitTypes::DistanceUnit mapUnits READ mapUnits WRITE setMapUnits )

  public:

    /**
     * Constructor for QgsTextPreview
     * \param parent parent widget
     */
    QgsTextPreview( QWidget *parent = nullptr );

    void paintEvent( QPaintEvent *e ) override;

    /**
     * Sets the text format for previewing in the widget.
     * \param format text format
     * \see format()
     */
    void setFormat( const QgsTextFormat &format );

    /**
     * Returns the text format used for previewing text in the widget.
     * \see setFormat()
     */
    QgsTextFormat format() const { return mFormat; }

    /**
     * Sets the \a scale to use for previewing format sizes in map units.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see scale()
     * \see setMapUnits()
     */
    void setScale( double scale );

    /**
     * Returns the scale used for previewing format sizes in map units.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see setScale()
     * \see mapUnits()
     */
    double scale() const { return mScale; }

    /**
     * Sets the map unit type for previewing format sizes in map units.
     * \param unit map units
     * \see mapUnits()
     * \see setScale()
     */
    void setMapUnits( QgsUnitTypes::DistanceUnit unit );

    /**
     * Returns the map unit type used for previewing format sizes in map units.
     * \see setMapUnits()
     * \see scale()
     */
    QgsUnitTypes::DistanceUnit mapUnits() const { return mMapUnits; }

  private:

    QgsScreenHelper *mScreenHelper = nullptr;

    QgsTextFormat mFormat;
    QgsRenderContext mContext;
    double mScale = -1;
    QgsUnitTypes::DistanceUnit mMapUnits = QgsUnitTypes::DistanceMeters;
    void updateContext();
};

#endif // QGSTEXTPREVIEW_H
