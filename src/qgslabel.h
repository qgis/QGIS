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
#include <list>
#include <qdom.h>

class QString;
class QWidget;
class QPainter;
class QPaintDevice;

class QgsPoint;
class QgsFeature;
class QgsField;
class QgsMapCanvas;
class QgsLabelAttributes;
class QgsRect;
class QgsCoordinateTransform;

/** Render class to display labels */
class QgsLabel
{
public:
    QgsLabel ( std::vector<QgsField>& fields  );

    ~QgsLabel();

    /* Fields */
    enum LabelField {
	Text = 0,
	Family,
	Size,
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

    void dialog( QWidget * parent = 0 );

    void renderLabel ( QPainter* painter, QgsRect* viewExtent, 
	               QgsCoordinateTransform *transform, QPaintDevice *device,
		       QgsFeature *feature, bool selected, QgsLabelAttributes *classAttributes=0);
    
    /** Reads the renderer configuration from an XML file
     @param rnode the DOM node to read 
    */
    void readXML(const QDomNode& node);

    /** Writes the contents of the renderer to a configuration file */
    void writeXML(std::ostream& xml);

    //! add vector of required fields to existing list of fields
    void addRequiredFields ( std::list<int> *fields );

    //! Available vector fields
    std::vector<QgsField> & QgsLabel::fields ( void );

    //! Pointer to default attributes
    QgsLabelAttributes *layerAttributes ( void );

    //! Set label field
    void setLabelField ( int attr, const QString str );

    //! label field
    QString labelField ( int attr );

    /** Get field value if : 1) field name is not empty
     *                       2) field exists
     *                       3) value is defined
     *  otherwise returns empty string
    */
    QString fieldValue ( int attr, QgsFeature *feature );

private:
    /** Get label point for simple feature in map units */
    QgsPoint labelPoint ( QgsFeature *feature );
    
    /** Color to draw selected features */
    QColor mSelectionColor;
    
    //! Default layer attributes
    QgsLabelAttributes *mLabelAttributes;
    
    //! Available layer fields
    std::vector<QgsField> mField;

    //! Label fields
    std::vector<QString> mLabelField;

    //! Label field indexes
    std::vector<int> mLabelFieldIdx;
};

#endif
