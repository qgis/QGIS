/***************************************************************************
                         qgslabel.cpp - render vector labels
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
#include <iostream>
#include <fstream>
#include <math.h> //needed for win32 build (ts)

#include <qstring.h>
#include <qfont.h>
#include <qfontmetrics.h>

#include <qpainter.h>
#include <qpaintdevice.h>
#include <qwmatrix.h>
#include <qdom.h>

#include "qgis.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsrect.h"
#include "qgscoordinatetransform.h"

#include "qgslabelattributes.h"
#include "qgslabeldialog.h"
#include "qgslabel.h"

// use M_PI define PI 3.141592654
#ifdef WIN32
#define M_PI 4*atan(1.0)
#endif

static const char * const ident_ = 
   "$Id$";


QgsLabel::QgsLabel( std::vector<QgsField>& fields )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsLabel::QgsLabel()" << std::endl;
    #endif

    mField = fields;
    mLabelField.resize ( LabelFieldCount );
    mLabelFieldIdx.resize ( LabelFieldCount );
    for ( int i = 0; i < LabelFieldCount; i++ ) {
        mLabelField[i] = "";
        mLabelFieldIdx[i] = -1;
    }
    mLabelAttributes = new QgsLabelAttributes ( true );
}

QgsLabel::~QgsLabel()
{
}

QString QgsLabel::fieldValue ( int attr, QgsFeature *feature )
{
    if ( mLabelField[attr].isEmpty() ) return QString(); 

    std::vector<QgsFeatureAttribute> fields =  feature->attributeMap();

    for ( unsigned int i = 0; i < fields.size(); i++ ) {
	if ( fields[i].fieldName().lower().compare(mLabelField[attr]) == 0 ) {
	    return fields[i].fieldValue();
	}
    }
    return QString();
}

void QgsLabel::renderLabel( QPainter * painter, QgsRect *viewExtent,
                            QgsCoordinateTransform *transform, QPaintDevice* device,
                            QgsFeature *feature, bool selected, QgsLabelAttributes *classAttributes )
{
    #if QGISDEBUG > 3 
    std::cerr << "QgsLabel::renderLabel()" << std::endl;
    #endif
    
    QPen pen;
    QFont font;
    QString value;
    QString text;
    double scale, x1, x2;

    /* Clac scale (not nice) */    
    QgsPoint point;
    point = transform->transform ( 0, 0 );
    x1 = point.x();
    point = transform->transform ( 1000, 0 );
    x2 = point.x();
    scale = (x2-x1)/1000;

    /* Text */
    value = fieldValue ( Text, feature );
    if ( value.isEmpty() ) {
        text = mLabelAttributes->text();
    } else {
	text = value;
    }

    /* Font */
    value = fieldValue ( Family, feature );
    if ( value.isEmpty() ) {
	font.setFamily ( mLabelAttributes->family() );
    } else {
	font.setFamily ( value );
    }
    
    double size;
    value = fieldValue ( Size, feature );
    if ( value.isEmpty() ) {
        size =  mLabelAttributes->size();
    } else {
        size =  value.toDouble();
    }
    if (  mLabelAttributes->sizeType() == QgsLabelAttributes::MapUnits ) {
	size *= scale;
    }
    font.setPointSizeFloat ( size );
    
    value = fieldValue ( Color, feature );
    if ( value.isEmpty() ) {
        pen.setColor ( mLabelAttributes->color() );
    } else { 
	pen.setColor ( QColor(value) );
    }
    
    value = fieldValue ( Bold, feature );
    if ( value.isEmpty() ) {
        font.setBold ( mLabelAttributes->bold() );
    } else {
	font.setBold ( (bool) value.toInt() );
    }
    
    value = fieldValue ( Italic, feature );
    if ( value.isEmpty() ) {
        font.setItalic ( mLabelAttributes->italic() );
    } else {
	font.setItalic ( (bool) value.toInt() );
    }
    
    value = fieldValue ( Underline, feature );
    if ( value.isEmpty() ) {
        font.setUnderline ( mLabelAttributes->underline() );
    } else { 
	font.setUnderline ( (bool) value.toInt() );
    }
    

    /* Coordinates */
    double x, y, xoffset, yoffset, ang;

    point = labelPoint ( feature ); 
	
    value = fieldValue ( XCoordinate, feature );
    if ( !value.isEmpty() ) {
	point.setX ( value.toDouble() );
    }
    value = fieldValue ( YCoordinate, feature );
    if ( !value.isEmpty() ) {
	point.setY ( value.toDouble() );
    }

    transform->transform(&point);
    x = point.x();
    y = point.y();

    value = fieldValue ( XOffset, feature );
    if ( value.isEmpty() ) {
        xoffset = mLabelAttributes->xOffset();
    } else { 
	xoffset = value.toDouble();
    }
    value = fieldValue ( YOffset, feature );
    if ( value.isEmpty() ) {
        yoffset = mLabelAttributes->yOffset();
    } else { 
	yoffset = value.toDouble();
    }

    // recalc offset to points
    if (  mLabelAttributes->offsetType() == QgsLabelAttributes::MapUnits ) {
	xoffset *= scale;
	yoffset *= scale;
    }
    
    value = fieldValue ( Angle, feature );
    if ( value.isEmpty() ) {
        ang = mLabelAttributes->angle();
    } else {
        ang = value.toDouble();
    }
    double rad = ang * M_PI/180;
    
    x = x + xoffset * cos(rad) - yoffset * sin(rad);
    y = y - xoffset * sin(rad) - yoffset * cos(rad);

    /* Alignment */
    int alignment;
    QFontMetrics fm ( font );
    int width = fm.width ( text );
    int height = fm.height();
    int dx, dy;

    value = fieldValue ( Alignment, feature );
    if ( value.isEmpty() ) {
        alignment = mLabelAttributes->alignment();
    } else {
	value = value.lower();
	alignment = Qt::AlignCenter;
	if ( value.compare("left") == 0 ) {
	    alignment = Qt::AlignLeft | Qt::AlignVCenter;
	} else if ( value.compare("right") == 0 ) {
	    alignment = Qt::AlignRight | Qt::AlignVCenter;
	} else if ( value.compare("bottom") == 0 ) {
	    alignment = Qt::AlignBottom | Qt::AlignHCenter;
	} else if ( value.compare("top") == 0 ) {
	    alignment = Qt::AlignTop | Qt::AlignHCenter;
	}
    }

    if ( alignment & Qt::AlignLeft ) {
	dx = 0;
    } else if ( alignment & Qt::AlignHCenter ) {
	dx = -width/2;
    } else if ( alignment & Qt::AlignRight ) {
	dx = -width;
    }

    if ( alignment & Qt::AlignBottom ) {
	dy = 0;
    } else if ( alignment & Qt::AlignVCenter ) {
	dy = height/2;
    } else if ( alignment & Qt::AlignTop ) {
	dy = height;
    }
    
    painter->save();
    painter->setFont ( font );
    painter->translate ( x, y );
    painter->rotate ( -ang );
    //
    // Draw a buffer behind the text if one is desired
    //
    if (mLabelAttributes->bufferSizeIsSet() && mLabelAttributes->bufferEnabled())
    {
      int myBufferSize = static_cast<int>(mLabelAttributes->bufferSize());
      if (mLabelAttributes->bufferColorIsSet())
      {
        painter->setPen( mLabelAttributes->bufferColor());
      }
      else //default to a white buffer
      {
        painter->setPen( Qt::white);
      }
      for (int i = dx-myBufferSize; i <= dx+myBufferSize; i++)
      {
        for (int j = dy-myBufferSize; j <= dy+myBufferSize; j++) 
        {
          painter->drawText( i ,j, text);
        }
      }
    }
    painter->setPen ( pen );
    painter->drawText ( dx, dy, text );
    painter->restore();
}

void QgsLabel::addRequiredFields ( std::list<int> *fields )
{
    for ( int i = 0; i < LabelFieldCount; i++ ) {
       if ( mLabelFieldIdx[i] == -1 ) continue;
       bool found = false;
       for (std::list<int>::iterator it = fields->begin(); it != fields->end(); ++it) {
	   if ( *it == mLabelFieldIdx[i] ) {
	       found = true;
	       break;
	   }
       }
       if (!found) {
	   fields->push_back(mLabelFieldIdx[i]);
       }
    }
}

std::vector<QgsField> & QgsLabel::fields ( void )
{
    return mField;
}

void QgsLabel::setLabelField ( int attr, const QString str )
{
    if ( attr >= LabelFieldCount ) return;
       
    
    mLabelField[attr] = str;

    mLabelFieldIdx[attr] = -1;
    for ( int i = 0; i < mField.size(); i++ ) {
	if ( mField[i].name().compare(str) == 0 ) {
            mLabelFieldIdx[attr] = i;
	}
    }
}

QString QgsLabel::labelField ( int attr )
{
    if ( attr > LabelFieldCount ) return QString();
    
    return mLabelField[attr];
}

QgsLabelAttributes *QgsLabel::layerAttributes ( void )
{
    return mLabelAttributes;
}

QgsPoint QgsLabel::labelPoint ( QgsFeature *feature )
{
    int wkbType;
    double *x, *y;
    unsigned char *ptr;
    int *nPoints;
	    
    unsigned char *geom = feature->getGeometry();
    
    memcpy(&wkbType, (geom+1), sizeof(wkbType));

    QgsPoint point;
    
    switch (wkbType) {
	case QGis::WKBPoint:
	    x = (double *) (geom + 5);
	    y = (double *) (geom + 5 + sizeof(double));
	    point.setX(*x);
	    point.setY(*y);
	break;

	case QGis::WKBLineString: // Line center
	    double dx, dy, tl, l;
	    ptr = geom + 5;
	    nPoints = (int *)ptr;
	    ptr = geom + 1 + 2 * sizeof(int);
	    
	    tl = 0;
	    for (int i = 1; i < *nPoints; i++) {
		dx = ((double *)ptr)[2*i] - ((double *)ptr)[2*i-2];
		dy = ((double *)ptr)[2*i+1] - ((double *)ptr)[2*i-1];
	        tl += sqrt(dx*dx + dy*dy);
	    }
	    tl /= 2;

	    l = 0;
	    for (int i = 1; i < *nPoints; i++) {
		double dl;

		dx = ((double *)ptr)[2*i] - ((double *)ptr)[2*i-2];
		dy = ((double *)ptr)[2*i+1] - ((double *)ptr)[2*i-1];
	        dl = sqrt(dx*dx + dy*dy);
                
	        if ( l+dl > tl ) {
		    l = tl - l;
		    double k = l/dl;
		    
		    point.setX ( ((double *)ptr)[2*i-2] +  k * dx  );
		    point.setY ( ((double *)ptr)[2*i-1] + k * dy );
		    break;
		}
		l += dl;
	    }
	break;

	case QGis::WKBPolygon:
	    double sx, sy;
	    ptr = geom + 1 + 2 * sizeof(int); // set pointer to the first ring
	    nPoints = (int *) ptr;
	    ptr += 4;

	    sx = sy = 0;
	    for (int i = 0; i < *nPoints-1; i++) {
		sx += ((double *)ptr)[2*i];
		sy += ((double *)ptr)[2*i+1];
	    }
	    point.setX ( sx/(*nPoints-1) );
	    point.setY ( sy/(*nPoints-1) );

	    break;
    }
    return QgsPoint ( point ); 
}

void QgsLabel::readXML( const QDomNode& node )
{
#if QGISDEBUG
  std::cout << "readXML called for layer label properties \n" << std::endl;
#endif
    //QDomNode nd;
    QDomElement el;
    int red, green, blue;
    int type;
   
    /* Text */
    el = node.namedItem("label").toElement();
    mLabelAttributes->setText ( el.attribute("text") );
    setLabelField ( Text, el.attribute("field") );

    /* Family */
    el = node.namedItem("family").toElement();
    mLabelAttributes->setFamily ( el.attribute("name") );
    setLabelField ( Family, el.attribute("field") );

    /* Size */
    el = node.namedItem("size").toElement();
    type = QgsLabelAttributes::unitsCode( el.attribute("units") );
    mLabelAttributes->setSize ( el.attribute("value").toDouble(), type );
    setLabelField ( Size, el.attribute("field") );

    /* Bold */
    el = node.namedItem("bold").toElement();
    mLabelAttributes->setBold ( (bool)el.attribute("on").toInt() );
    setLabelField ( Bold, el.attribute("field") );
    
    /* Italic */
    el = node.namedItem("italic").toElement();
    mLabelAttributes->setItalic ( (bool)el.attribute("on").toInt() );
    setLabelField ( Italic, el.attribute("field") );
    
    /* Underline */
    el = node.namedItem("underline").toElement();
    mLabelAttributes->setUnderline ( (bool)el.attribute("on").toInt() );
    setLabelField ( Underline, el.attribute("field") );
    
    /* Color */
    el = node.namedItem("color").toElement();
    red = el.attribute("red").toInt();
    green = el.attribute("green").toInt();
    blue = el.attribute("blue").toInt();
    mLabelAttributes->setColor ( QColor(red, green, blue) );
    setLabelField ( Color, el.attribute("field") );

    /* X */
    el = node.namedItem("x").toElement();
    setLabelField ( XCoordinate, el.attribute("field") );

    /* Y */
    el = node.namedItem("y").toElement();
    setLabelField ( YCoordinate, el.attribute("field") );

    /* X,Y offset */
    double xoffset, yoffset;
    el = node.namedItem("offset").toElement();
    type = QgsLabelAttributes::unitsCode( el.attribute("units") );
    xoffset = el.attribute("x").toDouble();
    yoffset = el.attribute("y").toDouble();
    mLabelAttributes->setOffset ( xoffset, yoffset, type );
    setLabelField ( XOffset, el.attribute("xfield") );
    setLabelField ( YOffset, el.attribute("yfield") );

    /* Angle */
    el = node.namedItem("angle").toElement();
    mLabelAttributes->setAngle ( el.attribute("value").toDouble() );
    setLabelField ( Angle, el.attribute("field") );

    /* Alignment */
    el = node.namedItem("alignment").toElement();
    mLabelAttributes->setAlignment ( QgsLabelAttributes::alignmentCode(el.attribute("value")) );
    setLabelField ( Alignment, el.attribute("field") );

    // Buffer
    el = node.namedItem("buffercolor").toElement();
    red = el.attribute("red").toInt();
    green = el.attribute("green").toInt();
    blue = el.attribute("blue").toInt();
    mLabelAttributes->setBufferColor ( QColor(red, green, blue) );
    setLabelField ( BufferColor, el.attribute("field") );

    el = node.namedItem("buffersize").toElement();
    type = QgsLabelAttributes::unitsCode( el.attribute("units") );
    mLabelAttributes->setBufferSize ( el.attribute("value").toDouble(), type );
    setLabelField ( BufferSize, el.attribute("field") );

    el = node.namedItem("bufferenabled").toElement();
    mLabelAttributes->setBufferEnabled ( (bool)el.attribute("on").toInt() );
    setLabelField ( BufferEnabled, el.attribute("field") );
}

void QgsLabel::writeXML(std::ostream& xml)
{
    
    xml << "\t\t<labelattributes>\n";

    /* Text */
    xml << "\t\t\t<label text=\"" << mLabelAttributes->text().ascii() << "\" field=\"" << mLabelField[Text].ascii() << "\" />\n";

    /* Family */
    xml << "\t\t\t<family name=\"" << mLabelAttributes->family().ascii() << "\" field=\"" << mLabelField[Family].ascii() << "\" />\n";

    /* Size */
    xml << "\t\t\t<size value=\"" << mLabelAttributes->size() << "\" units=\"" 
	<< (const char *)QgsLabelAttributes::unitsName(mLabelAttributes->sizeType()) << "\" field=\"" << mLabelField[Size].ascii() << "\" />\n";

    /* Bold */
    xml << "\t\t\t<bold on=\"" << mLabelAttributes->bold() << "\" field=\"" << mLabelField[Bold].ascii() << "\" />\n";

    /* Italic */
    xml << "\t\t\t<italic on=\"" << mLabelAttributes->italic() << "\" field=\"" << mLabelField[Italic].ascii() << "\" />\n";

    /* Underline */
    xml << "\t\t\t<underline on=\"" << mLabelAttributes->underline() << "\" field=\"" << mLabelField[Underline].ascii() << "\" />\n";

    /* Color */
    xml << "\t\t\t<color red=\"" << mLabelAttributes->color().red() << "\" green=\"" << mLabelAttributes->color().green()
        << "\" blue=\"" << mLabelAttributes->color().blue() << "\" field=\"" << mLabelField[Color].ascii() << "\" />\n";

    /* X */
    xml << "\t\t\t<x field=\"" << mLabelField[XCoordinate].ascii() << "\" />\n";

    /* Y */
    xml << "\t\t\t<y field=\"" << mLabelField[YCoordinate].ascii() << "\" />\n";

    /* Offset */
    xml << "\t\t\t<offset  units=\"" << QgsLabelAttributes::unitsName(mLabelAttributes->offsetType()).ascii()
        << "\" x=\"" << mLabelAttributes->xOffset() << "\" xfield=\"" << mLabelField[XOffset].ascii() 
        << "\" y=\"" << mLabelAttributes->yOffset() << "\" yfield=\"" << mLabelField[YOffset].ascii()
	<< "\" />\n";

    /* Angle */
    xml << "\t\t\t<angle value=\"" << mLabelAttributes->angle() << "\" field=\"" << mLabelField[Angle].ascii() << "\" />\n";

    /* Alignment */
    xml << "\t\t\t<alignment value=\"" << QgsLabelAttributes::alignmentName(mLabelAttributes->alignment()).ascii() 
	<< "\" field=\"" << mLabelField[Alignment].ascii() << "\" />\n";


    // Buffer settings
    xml << "\t\t\t<buffercolor red=\"" << mLabelAttributes->bufferColor().red() << "\" green=\"" << mLabelAttributes->bufferColor().green()
        << "\" blue=\"" << mLabelAttributes->bufferColor().blue() << "\" field=\"" << mLabelField[BufferColor].ascii() << "\" />\n";
    xml << "\t\t\t<buffersize value=\"" << mLabelAttributes->bufferSize() << "\" units=\"" 
	<< (const char *)QgsLabelAttributes::unitsName(mLabelAttributes->bufferSizeType()) << "\" field=\"" << mLabelField[BufferSize].ascii() << "\" />\n";

    xml << "\t\t\t<bufferenabled on=\"" << mLabelAttributes->bufferEnabled() << "\" field=\"" << mLabelField[BufferEnabled].ascii() << "\" />\n";
    
    xml << "\t\t</labelattributes>\n";
}

