/***************************************************************************
                         qgslabel.h - render vector labels
                             -------------------
    begin                : August 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGSLABEL_H
#define QGSLABEL_H

#include <vector>

#include <QColor>
#include <QList>
#include <QMap>

class QDomNode;
class QString;
class QPainter;
class QPaintDevice;

class QgsPoint;
class QgsFeature;
class QgsField;
class QgsLabelAttributes;
class QgsRect;
class QgsMapToPixel;
class QgsCoordinateTransform;

#include "qgsfield.h"

typedef QList<int> QgsAttributeList;

typedef QMap<int, QgsField> QgsFieldMap;

/** Render class to display labels */
class CORE_EXPORT QgsLabel
{
public:
    QgsLabel ( const QgsFieldMap & fields  );

    ~QgsLabel();

    /* Fields */
    enum LabelField {
	Text = 0,
	Family,
	Size,
	SizeType,
	Bold,
	Italic,
	Underline,
	Color,
	XCoordinate,
	YCoordinate,
	XOffset,
	YOffset,
	Angle,
	Alignment,
        BufferEnabled,
	BufferSize,
	BufferColor,
	BufferBrush,
	BorderWidth,
	BorderColor,
	BorderStyle,
	LabelFieldCount
    };

    /** \brief render label
     *  \param sizeScale global scale factor for size in pixels, labels in map units are not scaled
     */
    void renderLabel ( QPainter* painter, const QgsRect& viewExtent, 
                       const QgsCoordinateTransform* coordTransform,
                       const QgsMapToPixel *transform,
		       QgsFeature &feature, bool selected, QgsLabelAttributes *classAttributes=0, double sizeScale = 1.);

    /** Reads the renderer configuration from an XML file
     @param rnode the DOM node to read 
    */
    void readXML(const QDomNode& node);

    /** Writes the contents of the renderer to a configuration file */
    void writeXML(std::ostream& xml);

    //! add vector of required fields to existing list of fields
    void addRequiredFields ( QgsAttributeList& fields );

    //! Set available fields
    void setFields( const QgsFieldMap & fields  );

    //! Available vector fields
    QgsFieldMap & fields ( void );

    //! Pointer to default attributes
    QgsLabelAttributes *layerAttributes ( void );

    //! Set label field
    void setLabelField ( int attr, int fieldIndex );

    //! label field
    QString labelField ( int attr );

    /** Get field value if : 1) field name is not empty
     *                       2) field exists
     *                       3) value is defined
     *  otherwise returns empty string
    */
    QString fieldValue ( int attr, QgsFeature& feature );

private:
    /** Does the actual rendering of a label at the given point
     * 
     */
    void renderLabel(QPainter* painter, QgsPoint point, 
                     const QgsCoordinateTransform* coordTransform,
                     const QgsMapToPixel* transform,
                     QString text, QFont font, QPen pen,
                     int dx, int dy,
                     double xoffset, double yoffset,
                     double ang);

    /** Get label point for simple feature in map units */
    void labelPoint ( std::vector<QgsPoint>&, QgsFeature & feature );

    /** Get label point for the given feature in wkb format. */
    unsigned char* labelPoint( QgsPoint& point, unsigned char* wkb, size_t wkblen);

    /** Color to draw selected features */
    QColor mSelectionColor;
    
    //! Default layer attributes
    QgsLabelAttributes *mLabelAttributes;
    
    //! Available layer fields
    QgsFieldMap mField;

    //! Label fields
    std::vector<QString> mLabelField;

    //! Label field indexes
    std::vector<int> mLabelFieldIdx;
};

#endif
