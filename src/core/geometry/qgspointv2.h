/***************************************************************************
                         qgspointv2.h
                         --------------
    begin                : September 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTV2_H
#define QGSPOINTV2_H

#include "qgsabstractgeometryv2.h"

/** \ingroup core
 * \class QgsPointV2
 * \brief Point geometry type.
 * \note added in QGIS 2.10
 * \note this API is not considered stable and may change for 2.12
 */
class CORE_EXPORT QgsPointV2: public QgsAbstractGeometryV2
{
  public:
    QgsPointV2( double x = 0.0, double y = 0.0 );
    QgsPointV2( QgsWKBTypes::Type type, double x = 0.0, double y = 0.0, double z = 0.0, double m = 0.0 );

    bool operator==( const QgsPointV2& pt ) const;
    bool operator!=( const QgsPointV2& pt ) const;

    virtual QgsAbstractGeometryV2* clone() const override;
    void clear() override;

    double x() const { return mX; }
    double y() const { return mY; }
    double z() const { return mZ; }
    double m() const { return mM; }

    void setX( double x ) { mX = x; }
    void setY( double y ) { mY = y; }
    void setZ( double z ) { mZ = z; }
    void setM( double m ) { mM = m; }

    virtual QString geometryType() const override { return "Point"; }

    //implementation of inherited methods
    virtual int dimension() const override { return 0; }


    virtual bool fromWkb( const unsigned char* wkb ) override;
    virtual bool fromWkt( const QString& wkt ) override;

    int wkbSize() const override;
    unsigned char* asWkb( int& binarySize ) const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const override;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const override;
    QString asJSON( int precision = 17 ) const override;

    virtual QgsRectangle calculateBoundingBox() const override { return QgsRectangle( mX, mY, mX, mY );}

    void draw( QPainter& p ) const override;
    void transform( const QgsCoordinateTransform& ct ) override;
    void transform( const QTransform& t ) override;

    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const override;

    //low-level editing
    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex ) override { Q_UNUSED( position ); Q_UNUSED( vertex ); return false; }
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos ) override;
    virtual bool deleteVertex( const QgsVertexId& position ) override { Q_UNUSED( position ); return false; }

    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const override;
    bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const override;

  private:
    double mX;
    double mY;
    double mZ;
    double mM;
};

#endif // QGSPOINTV2_H
