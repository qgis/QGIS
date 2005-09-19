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
#include "qgsmaptopixel.h"
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


QgsLabel::QgsLabel( std::vector<QgsField> const & fields )
{
#ifdef QGISDEBUG
    std::cerr << "QgsLabel::QgsLabel()" << std::endl;
#endif

    mField = fields;
    mLabelField.resize ( LabelFieldCount );
    mLabelFieldIdx.resize ( LabelFieldCount );
    for ( int i = 0; i < LabelFieldCount; i++ )
    {
        mLabelField[i] = "";
        mLabelFieldIdx[i] = -1;
    }
    mLabelAttributes = new QgsLabelAttributes ( true );
}

QgsLabel::~QgsLabel()
{}

QString QgsLabel::fieldValue ( int attr, QgsFeature *feature )
{
    if ( mLabelField[attr].isEmpty() )
        return QString();

    std::vector<QgsFeatureAttribute> fields =  feature->attributeMap();

    for ( unsigned int i = 0; i < fields.size(); i++ )
    {
        if ( fields[i].fieldName().lower().compare(mLabelField[attr]) == 0 )
        {
            return fields[i].fieldValue();
        }
    }
    return QString();
}

void QgsLabel::renderLabel( QPainter * painter, QgsRect *viewExtent,
                            const QgsCoordinateTransform& coordTransform,
                            bool doCoordTransform,
                            QgsMapToPixel *transform, QPaintDevice* device,
                            QgsFeature *feature, bool selected, QgsLabelAttributes *classAttributes,
       			    double sizeScale )
{
#if QGISDEBUG > 3
    std::cerr << "QgsLabel::renderLabel()" << std::endl;
#endif

    QPen pen;
    QFont font;
    QString value;
    QString text;
    double scale, x1, x2;

    /* Calc scale (not nice) */
    QgsPoint point;
    point = transform->transform ( 0, 0 );
    x1 = point.x();
    point = transform->transform ( 1000, 0 );
    x2 = point.x();
    scale = (x2-x1)/1000;

    /* Text */
    value = fieldValue ( Text, feature );
    if ( value.isEmpty() )
    {
        text = mLabelAttributes->text();
    }
    else
    {
        text = value;
    }

    /* Font */
    value = fieldValue ( Family, feature );
    if ( value.isEmpty() )
    {
        font.setFamily ( mLabelAttributes->family() );
    }
    else
    {
        font.setFamily ( value );
    }

    double size;
    value = fieldValue ( Size, feature );
    if ( value.isEmpty() )
    {
        size =  mLabelAttributes->size();
    }
    else
    {
        size =  value.toDouble();
    }
    if (  mLabelAttributes->sizeType() == QgsLabelAttributes::MapUnits )
    {
        size *= scale;
    } else {
	size *= sizeScale;
    }
    font.setPointSizeFloat ( size );

    value = fieldValue ( Color, feature );
    if ( value.isEmpty() )
    {
        pen.setColor ( mLabelAttributes->color() );
    }
    else
    {
        pen.setColor ( QColor(value) );
    }

    value = fieldValue ( Bold, feature );
    if ( value.isEmpty() )
    {
        font.setBold ( mLabelAttributes->bold() );
    }
    else
    {
        font.setBold ( (bool) value.toInt() );
    }

    value = fieldValue ( Italic, feature );
    if ( value.isEmpty() )
    {
        font.setItalic ( mLabelAttributes->italic() );
    }
    else
    {
        font.setItalic ( (bool) value.toInt() );
    }

    value = fieldValue ( Underline, feature );
    if ( value.isEmpty() )
    {
        font.setUnderline ( mLabelAttributes->underline() );
    }
    else
    {
        font.setUnderline ( (bool) value.toInt() );
    }


    /* Coordinates */
    double x, y, xoffset, yoffset, ang;

    point = labelPoint ( feature );

    value = fieldValue ( XCoordinate, feature );
    if ( !value.isEmpty() )
    {
        point.setX ( value.toDouble() );
    }
    value = fieldValue ( YCoordinate, feature );
    if ( !value.isEmpty() )
    {
        point.setY ( value.toDouble() );
    }

    // Convert point to projected units
    if (doCoordTransform)
      point = coordTransform.transform(point);

    // and then to canvas units
    transform->transform(&point);
    x = point.x();
    y = point.y();

    value = fieldValue ( XOffset, feature );
    if ( value.isEmpty() )
    {
        xoffset = mLabelAttributes->xOffset();
    }
    else
    {
        xoffset = value.toDouble();
    }
    value = fieldValue ( YOffset, feature );
    if ( value.isEmpty() )
    {
        yoffset = mLabelAttributes->yOffset();
    }
    else
    {
        yoffset = value.toDouble();
    }

    // recalc offset to points
    if (  mLabelAttributes->offsetType() == QgsLabelAttributes::MapUnits )
    {
        xoffset *= scale;
        yoffset *= scale;
    }

    value = fieldValue ( Angle, feature );
    if ( value.isEmpty() )
    {
        ang = mLabelAttributes->angle();
    }
    else
    {
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
    if ( value.isEmpty() )
    {
        alignment = mLabelAttributes->alignment();
    }
    else
    {
        value = value.lower();
        alignment = Qt::AlignCenter;
        if ( value.compare("left") == 0 )
        {
            alignment = Qt::AlignLeft | Qt::AlignVCenter;
        }
        else if ( value.compare("right") == 0 )
        {
            alignment = Qt::AlignRight | Qt::AlignVCenter;
        }
        else if ( value.compare("bottom") == 0 )
        {
            alignment = Qt::AlignBottom | Qt::AlignHCenter;
        }
        else if ( value.compare("top") == 0 )
        {
            alignment = Qt::AlignTop | Qt::AlignHCenter;
        }
    }

    if ( alignment & Qt::AlignLeft )
    {
        dx = 0;
    }
    else if ( alignment & Qt::AlignHCenter )
    {
        dx = -width/2;
    }
    else if ( alignment & Qt::AlignRight )
    {
        dx = -width;
    }

    if ( alignment & Qt::AlignBottom )
    {
        dy = 0;
    }
    else if ( alignment & Qt::AlignVCenter )
    {
        dy = height/2;
    }
    else if ( alignment & Qt::AlignTop )
    {
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
    for ( int i = 0; i < LabelFieldCount; i++ )
    {
        if ( mLabelFieldIdx[i] == -1 )
            continue;
        bool found = false;
        for (std::list<int>::iterator it = fields->
                                           begin();
                it != fields->end();
                ++it)
        {
            if ( *it == mLabelFieldIdx[i] )
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            fields->push_back(mLabelFieldIdx[i])
            ;
        }
    }
}

std::vector<QgsField> & QgsLabel::fields ( void )
{
    return mField;
}

void QgsLabel::setLabelField ( int attr, const QString str )
{
    if ( attr >= LabelFieldCount )
        return;


    mLabelField[attr] = str;

    mLabelFieldIdx[attr] = -1;
    for ( int i = 0; i < mField.size(); i++ )
    {
        if ( mField[i].name().compare(str) == 0 )
        {
            mLabelFieldIdx[attr] = i;
        }
    }
}

QString QgsLabel::labelField ( int attr )
{
    if ( attr > LabelFieldCount )
        return QString();

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

    switch (wkbType)
    {
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
        for (int i = 1; i < *nPoints; i++)
        {
            dx = ((double *)ptr)[2*i] - ((double *)ptr)[2*i-2];
            dy = ((double *)ptr)[2*i+1] - ((double *)ptr)[2*i-1];
            tl += sqrt(dx*dx + dy*dy);
        }
        tl /= 2;

        l = 0;
        for (int i = 1; i < *nPoints; i++)
        {
            double dl;

            dx = ((double *)ptr)[2*i] - ((double *)ptr)[2*i-2];
            dy = ((double *)ptr)[2*i+1] - ((double *)ptr)[2*i-1];
            dl = sqrt(dx*dx + dy*dy);

            if ( l+dl > tl )
            {
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
        for (int i = 0; i < *nPoints-1; i++)
        {
            sx += ((double *)ptr)[2*i];
            sy += ((double *)ptr)[2*i+1];
        }
        point.setX ( sx/(*nPoints-1) );
        point.setY ( sy/(*nPoints-1) );

        break;
    case QGis::WKBMultiPolygon:
        break; ///// <--------------remove this when code below needs testing..........
        {
            double sx, sy;
            unsigned char *ptr;
            int idx, jdx, kdx;
            int *numPolygons, *numRings;

            // get the number of polygons
            ptr = geom + 5;
            numPolygons = (int *) ptr;
#ifdef QGISDEBUG

            std::cout << "Finding label point for mutipolygon with "
            << *numPolygons << " parts " << std::endl;
#endif
            //for (kdx = 0; kdx < *numPolygons; kdx++)
            for (kdx = 0; kdx < 1; kdx++) //just label the first sub polygon
            {
                //skip the endian and feature type info and
                // get number of rings in the polygon
                ptr+=14;
                numRings = (int *) ptr;
                ptr += 4;
#ifdef QGISDEBUG

                std::cout << "Multipolygon part " << kdx << " ring iteration  " << std::endl;
#endif

                for (idx = 0; idx < *numRings; idx++)
                {
#ifdef QGISDEBUG
                    std::cout << "Multipolygon part " << kdx << " ring " << idx
                    << std::endl;
#endif
                    // get number of points in the ring
                    nPoints = (int *) ptr;
                    ptr += 4;
                    sx = sy = 0;

                    //loop through vertices skipping last which == first
                    for (int i = 0; i < *nPoints-1; i++)
                    {
                        x = (double *)ptr;
                        ptr += sizeof(double);
                        y = (double *)ptr;
                        ptr += sizeof(double);
                        sx += *x;
                        sy += *y;
#ifdef QGISDEBUG

                        std::cout << "\tVertex " << i << " x: " << *x << " y: " << *y  << std::endl;
#endif

                    }
                }
#ifdef QGISDEBUG
                std::cout << "Setting multipart polygon label point to" << sx/(*nPoints-1) << ", "<< sy/(*nPoints-1) << std::endl;
#endif

                point.setX ( sx/(*nPoints-1) );
                point.setY ( sy/(*nPoints-1) );
            }
            break;


        }
    }
    return QgsPoint ( point );
}

void QgsLabel::readXML( const QDomNode& node )
{
#if QGISDEBUG
    std::cout << "QgsLabel::readXML() called for layer label properties \n" << std::endl;
#endif

    qDebug( "%s:%d QgsLabel::readXML() got node %s", __FILE__, __LINE__, node.nodeName().ascii() );

    QDomNode scratchNode;       // DOM node re-used to get current QgsLabel attribute
    QDomElement el;
    
    int red, green, blue;
    int type;

    /* Text */
    scratchNode = node.namedItem("label");

    if ( scratchNode.isNull() )
    {
        qDebug( "%s:%d couldn't find QgsLabel ``label'' attribute", __FILE__, __LINE__ );
    }
    else
    {
        el = scratchNode.toElement();
        mLabelAttributes->setText ( el.attribute("text","") );
        setLabelField ( Text, el.attribute("field","") );
    }

    /* Family */
    scratchNode = node.namedItem("family");

    if ( scratchNode.isNull() )
    {
        qDebug( "%s:%d couldn't find QgsLabel ``family'' attribute", __FILE__, __LINE__  );
    }
    else
    {
        el = scratchNode.toElement();
        mLabelAttributes->setFamily ( el.attribute("name","") );
        setLabelField ( Family, el.attribute("field","") );
    }

    /* Size */
    scratchNode = node.namedItem("size");

    if ( scratchNode.isNull() )
    {
        qDebug( "%s:%d couldn't find QgsLabel ``size'' attribute", __FILE__, __LINE__  );
    }
    else
    {
        el = scratchNode.toElement();
        type = QgsLabelAttributes::unitsCode( el.attribute("units","") );
        mLabelAttributes->setSize ( el.attribute("value", "0.0").toDouble(), type );
        setLabelField ( Size, el.attribute("field","") );
    }

    /* Bold */
    scratchNode = node.namedItem("bold");

    if ( scratchNode.isNull() )
    {
        qDebug( "%s:%d couldn't find QgsLabel ``bold'' attribute", __FILE__, __LINE__  );
    }
    else
    {
        el = scratchNode.toElement();
        mLabelAttributes->setBold ( (bool)el.attribute("on","0").toInt() );
        setLabelField ( Bold, el.attribute("field","") );
    }

    /* Italic */
    scratchNode = node.namedItem("italic");

    if ( scratchNode.isNull() )
    {
        qDebug( "%s:%d couldn't find QgsLabel ``italic'' attribute", __FILE__, __LINE__  );
    }
    else
    {
        el = scratchNode.toElement();
        mLabelAttributes->setItalic ( (bool)el.attribute("on","0").toInt() );
        setLabelField ( Italic, el.attribute("field","") );
    }

    /* Underline */
    scratchNode = node.namedItem("underline");

    if ( scratchNode.isNull() )
    {
        qDebug( "%s:%d couldn't find QgsLabel ``underline'' attribute", __FILE__, __LINE__  );
    }
    else
    {
        el = scratchNode.toElement();
        mLabelAttributes->setUnderline ( (bool)el.attribute("on","0").toInt() );
        setLabelField ( Underline, el.attribute("field","") );
    }

    /* Color */
    scratchNode = node.namedItem("color");

    if ( scratchNode.isNull() )
    {
        qDebug( "%s:%d couldn't find QgsLabel ``color'' attribute", __FILE__, __LINE__  );
    }
    else
    {
        el = scratchNode.toElement();

        red = el.attribute("red","0").toInt();
        green = el.attribute("green","0").toInt();
        blue = el.attribute("blue","0").toInt();

        mLabelAttributes->setColor( QColor(red, green, blue) );

        setLabelField ( Color, el.attribute("field","") );
    }

    /* X */
    scratchNode = node.namedItem("x");

    if ( scratchNode.isNull() )
    {
        qDebug( "%s:%d couldn't find QgsLabel ``x'' attribute", __FILE__, __LINE__  );
    }
    else
    {
        el = scratchNode.toElement();
        setLabelField ( XCoordinate, el.attribute("field","") );
    }

    /* Y */
    scratchNode = node.namedItem("y");

    if ( scratchNode.isNull() )
    {
        qDebug( "%s:%d couldn't find QgsLabel ``y'' attribute", __FILE__, __LINE__  );
    }
    else
    {
        el = scratchNode.toElement();
        setLabelField ( YCoordinate, el.attribute("field","") );
    }


    /* X,Y offset */
    scratchNode = node.namedItem("offset");

    if ( scratchNode.isNull() )
    {
        qDebug( "%s:%d couldn't find QgsLabel ``offset'' attribute", __FILE__, __LINE__  );
    }
    else
    {
        double xoffset, yoffset;

        el = scratchNode.toElement();

        type = QgsLabelAttributes::unitsCode( el.attribute("units","") );
        xoffset = el.attribute("x","0.0").toDouble();
        yoffset = el.attribute("y","0.0").toDouble();

        mLabelAttributes->setOffset ( xoffset, yoffset, type );
        setLabelField ( XOffset, el.attribute("xfield","0") );
        setLabelField ( YOffset, el.attribute("yfield","0") );
    }

    /* Angle */
    scratchNode = node.namedItem("angle");

    if ( scratchNode.isNull() )
    {
        qDebug( "%s:%d couldn't find QgsLabel ``angle'' attribute", __FILE__, __LINE__  );
    }
    else
    {
        el = scratchNode.toElement();
        mLabelAttributes->setAngle ( el.attribute("value","0.0").toDouble() );
        setLabelField ( Angle, el.attribute("field","0.0") );
    }

    /* Alignment */
    scratchNode = node.namedItem("alignment");

    if ( scratchNode.isNull() )
    {
        qDebug( "%s:%d couldn't find QgsLabel ``alignment'' attribute", __FILE__, __LINE__  );
    }
    else
    {
        el = scratchNode.toElement();
        mLabelAttributes->setAlignment ( QgsLabelAttributes::alignmentCode(el.attribute("value","")) );
        setLabelField ( Alignment, el.attribute("field","") );
    }


    // Buffer
    scratchNode = node.namedItem("buffercolor");

    if ( scratchNode.isNull() )
    {
        qDebug( "%s:%d couldn't find QgsLabel ``buffercolor'' attribute", __FILE__, __LINE__  );
    }
    else
    {
        el = scratchNode.toElement();

        red = el.attribute("red","0").toInt();
        green = el.attribute("green","0").toInt();
        blue = el.attribute("blue","0").toInt();

        mLabelAttributes->setBufferColor( QColor(red, green, blue) );
        setLabelField ( BufferColor, el.attribute("field","") );
    }

    scratchNode = node.namedItem("buffersize");

    if ( scratchNode.isNull() )
    {
        qDebug( "%s:%d couldn't find QgsLabel ``buffersize'' attribute", __FILE__, __LINE__  );
    }
    else
    {
        el = scratchNode.toElement();

        type = QgsLabelAttributes::unitsCode( el.attribute("units","") );
        mLabelAttributes->setBufferSize ( el.attribute("value","0.0").toDouble(), type );
        setLabelField ( BufferSize, el.attribute("field","") );
    }

    scratchNode = node.namedItem("bufferenabled");

    if ( scratchNode.isNull() )
    {
        qDebug( "%s:%d couldn't find QgsLabel ``bufferenabled'' attribute", __FILE__, __LINE__  );
    }
    else
    {
        el = scratchNode.toElement();

        mLabelAttributes->setBufferEnabled ( (bool)el.attribute("on","0").toInt() );
        setLabelField ( BufferEnabled, el.attribute("field","") );
    }

} // QgsLabel::readXML()



void QgsLabel::writeXML(std::ostream& xml)
{

    xml << "\t\t<labelattributes>\n";

    // else
    if ( mLabelAttributes->textIsSet() && !mLabelField[Text].isEmpty() )
    {
        xml << "\t\t\t<label text=\"" << mLabelAttributes->text().local8Bit() << "\" field=\"" << mLabelField[Text].local8Bit() << "\" />\n";
    }
    else if ( mLabelAttributes->textIsSet() )
    {
        xml << "\t\t\t<label text=\"" << mLabelAttributes->text().local8Bit() << "\" field=\"\" />\n";
    }
    else
    {
        xml << "\t\t\t<label text=\"" << mLabelAttributes->text().local8Bit() << "\" field=\"" << mLabelField[Text].local8Bit() << "\" />\n";
    }

    if ( mLabelAttributes->familyIsSet() && ! mLabelAttributes->family().isNull() && mLabelField[Family].isNull())
    {
        xml << "\t\t\t<family name=\"" << mLabelAttributes->family().local8Bit() << "\" field=\"" << mLabelField[Family].local8Bit() << "\" />\n";
    }
    else if ( mLabelAttributes->familyIsSet() && ! mLabelAttributes->family().isNull() )
    {
        xml << "\t\t\t<family name=\"" << mLabelAttributes->family().local8Bit() << "\" field=\"\" />\n";
    }
    else
    {
        xml << "\t\t\t<family name=\"Arial\" field=\"\" />\n";
    }

    //size and units
    if ( mLabelAttributes->sizeIsSet() && !mLabelField[Size].isEmpty())
    {
        xml << "\t\t\t<size value=\"" << mLabelAttributes->size() << "\" units=\""
            << (const char *)QgsLabelAttributes::unitsName(mLabelAttributes->sizeType()) << "\" field=\"" << mLabelField[Size].local8Bit() << "\" />\n";
    }
    else if ( mLabelAttributes->sizeIsSet() )
    {
        xml << "\t\t\t<size value=\"" << mLabelAttributes->size() << "\" units=\""
            << (const char *)QgsLabelAttributes::unitsName(mLabelAttributes->sizeType()) << "\" field=\"\" />\n";
    }
    else
    {
        xml << "\t\t\t<size value=\"12\" units=\"Points\" field=\"\" />\n";
    }



    //bold
    if ( mLabelAttributes->boldIsSet() && !mLabelField[Bold].isEmpty() )
    {
        xml << "\t\t\t<bold on=\"" << mLabelAttributes->bold() << "\" field=\"" << mLabelField[Bold].local8Bit() << "\" />\n";
    }
    else if ( mLabelAttributes->boldIsSet() )
    {
        xml << "\t\t\t<bold on=\"" << mLabelAttributes->bold() << "\" field=\"\" />\n";
    }
    else
    {
        xml << "\t\t\t<bold on=\"0\" field=\"0\" />\n";
    }

    //italics
    if ( mLabelAttributes->italicIsSet() && ! mLabelField[Italic].isEmpty())
    {
        xml << "\t\t\t<italic on=\"" << mLabelAttributes->italic() << "\" field=\"" << mLabelField[Italic].local8Bit() << "\" />\n";
    }
    else if ( mLabelAttributes->italicIsSet() )
    {
        xml << "\t\t\t<italic on=\"" << mLabelAttributes->italic() << "\" field=\"\" />\n";
    }
    else
    {
        xml << "\t\t\t<italic on=\"0\" field=\"\" />\n";
    }
    //underline
    if ( mLabelAttributes->underlineIsSet() && !mLabelField[Underline].isEmpty())
    {
        xml << "\t\t\t<underline on=\"" << mLabelAttributes->underline() << "\" field=\"" << mLabelField[Underline].local8Bit() << "\" />\n";
    }
    else if ( mLabelAttributes->underlineIsSet() )
    {
        xml << "\t\t\t<underline on=\"" << mLabelAttributes->underline() << "\" field=\"\" />\n";
    }
    else
    {
        xml << "\t\t\t<underline on=\"0\" field=\"\" />\n";
    }

    if ( mLabelAttributes->colorIsSet() && ! mLabelField[Color].isNull() )
    {
        xml << "\t\t\t<color red=\"" << mLabelAttributes->color().red() << "\" green=\"" << mLabelAttributes->color().green()
            << "\" blue=\"" << mLabelAttributes->color().blue() << "\" field=\"" << mLabelField[Color].local8Bit() << "\" />\n";
    }
    else if ( mLabelAttributes->colorIsSet() )
    {
        xml << "\t\t\t<color red=\"" << mLabelAttributes->color().red() << "\" green=\"" << mLabelAttributes->color().green()
            << "\" blue=\"" << mLabelAttributes->color().blue() << "\" field=\"\" />\n";
    }
    else
    {
        xml << "\t\t\t<color red=\"0\" green=\"0\" blue=\"0\" field=\"\" />\n";
    }


    /* X */
    if (! mLabelField[XCoordinate].isEmpty() )
    {
      xml << "\t\t\t<x field=\"" << mLabelField[XCoordinate].local8Bit() << "\" />\n";
    }
    else
    {
     xml << "\t\t\t<x field=\"" << "\" />\n";
    }

    /* Y */
    if (! mLabelField[YCoordinate].isEmpty() )
    {
      xml << "\t\t\t<y field=\"" << mLabelField[YCoordinate].local8Bit() << "\" />\n";
    }
    else
    {
     xml << "\t\t\t<y field=\"" << "\" />\n";
    }

    // offset
    if ( mLabelAttributes->offsetIsSet() )
    {
        xml << "\t\t\t<offset  units=\"" << QgsLabelAttributes::unitsName(mLabelAttributes->offsetType()).local8Bit()
            << "\" x=\"" << mLabelAttributes->xOffset() << "\" xfield=\"" << mLabelField[XOffset].local8Bit()
            << "\" y=\"" << mLabelAttributes->yOffset() << "\" yfield=\"" << mLabelField[YOffset].local8Bit()
            << "\" />\n";
    }

    // Angle
    if ( mLabelAttributes->angleIsSet() )
    {
        xml << "\t\t\t<angle value=\"" << mLabelAttributes->angle() << "\" field=\"" << mLabelField[Angle].local8Bit() << "\" />\n";
    }
    else if ( mLabelAttributes->angleIsSet() )
    {
        xml << "\t\t\t<angle value=\"" << mLabelAttributes->angle() << "\" field=\"" << "\" />\n";
    }
    else
    {
        xml << "\t\t\t<angle value=\"" << "\" field=\"" << "\" />\n";
    }

    // alignment
    if ( mLabelAttributes->alignmentIsSet() )
    {
        xml << "\t\t\t<alignment value=\"" << QgsLabelAttributes::alignmentName(mLabelAttributes->alignment()).local8Bit()
            << "\" field=\"" << mLabelField[Alignment].local8Bit() << "\" />\n";
    }


    if ( mLabelAttributes->bufferColorIsSet() && ! mLabelField[BufferColor].isNull() )
    {
        xml << "\t\t\t<buffercolor red=\"" << mLabelAttributes->bufferColor().red() << "\" green=\"" << mLabelAttributes->bufferColor().green()
            << "\" blue=\"" << mLabelAttributes->bufferColor().blue() << "\" field=\"" << mLabelField[BufferColor].local8Bit() << "\" />\n";
    }
    else if ( mLabelAttributes->bufferColorIsSet() )
    {
        xml << "\t\t\t<buffercolor red=\"" << mLabelAttributes->bufferColor().red() << "\" green=\"" << mLabelAttributes->bufferColor().green()
            << "\" blue=\"" << mLabelAttributes->bufferColor().blue() << "\" field=\"" <<  "\" />\n";
    }
    else
    {
        xml << "\t\t\t<buffercolor red=\""  << "\" green=\""
            << "\" blue=\""  << "\" field=\"" <<  "\" />\n";
    }


    if ( mLabelAttributes->bufferSizeIsSet() && ! mLabelField[BufferSize].isNull() )
    {
        xml << "\t\t\t<buffersize value=\"" << mLabelAttributes->bufferSize() << "\" units=\""
            << (const char *)QgsLabelAttributes::unitsName(mLabelAttributes->bufferSizeType()) << "\" field=\"" << mLabelField[BufferSize].local8Bit() << "\" />\n";
    }
    else if ( mLabelAttributes->bufferSizeIsSet() )
    {
        xml << "\t\t\t<buffersize value=\"" << mLabelAttributes->bufferSize() << "\" units=\""
            << (const char *)QgsLabelAttributes::unitsName(mLabelAttributes->bufferSizeType()) << "\" field=\"" << "\" />\n";
    }
    else
    {
        xml << "\t\t\t<buffersize value=\"" << "\" units=\""
            <<  "\" field=\"" <<  "\" />\n";
    }


    if ( mLabelAttributes->bufferEnabled() && ! mLabelField[BufferEnabled].isNull() )
    {
        xml << "\t\t\t<bufferenabled on=\"" << mLabelAttributes->bufferEnabled() << "\" field=\"" << mLabelField[BufferEnabled].local8Bit() << "\" />\n";
    }
    else if ( mLabelAttributes->bufferEnabled())
    {
        xml << "\t\t\t<bufferenabled on=\"" << mLabelAttributes->bufferEnabled() << "\" field=\"" << "\" />\n";
    }
    else
    {
        xml << "\t\t\t<bufferenabled on=\"" << "\" field=\"" << "\" />\n";
    }
    xml << "\t\t</labelattributes>\n";
}

