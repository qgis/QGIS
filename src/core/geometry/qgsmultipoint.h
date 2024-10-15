/***************************************************************************
                        qgsmultipoint.h
  -------------------------------------------------------------------
Date                 : 29 Oct 2014
Copyright            : (C) 2014 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMULTIPOINT_H
#define QGSMULTIPOINT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsgeometrycollection.h"

/**
 * \ingroup core
 * \class QgsMultiPoint
 * \brief Multi point geometry collection.
 */
class CORE_EXPORT QgsMultiPoint: public QgsGeometryCollection
{
  public:

    /**
     * Constructor for an empty multipoint geometry.
     */
    QgsMultiPoint() SIP_HOLDGIL;

#ifndef SIP_RUN

    /**
     * Construct a multipoint from a vector of points.
     * Z and M type will be set based on the type of the first point
     * in the vector.
     *
     * \since QGIS 3.34
     */
    QgsMultiPoint( const QVector<QgsPoint> &points );

    /**
     * Construct a multipoint from a vector of points.
     * Z and M type will be set based on the type of the first point
     * in the vector.
     *
     * Ownership of the \a points is transferred to the multipoint.
     *
     * \since QGIS 3.34
     */
    QgsMultiPoint( const QVector<QgsPoint *> &points );

    /**
    * Construct a multipoint from list of points.
    * This constructor is more efficient then calling addGeometry() repeatedly.
    *
    * \since QGIS 3.34
    */
    QgsMultiPoint( const QVector<QgsPointXY> &points );
#else

    /**
     * Construct a multipoint from a sequence of points (QgsPoint objects, QgsPointXY objects, or sequences of float values).
     *
     * The multipoint Z and M type will be set based on the type of the first point in the sequence.
     *
     * \since QGIS 3.34
     */
    QgsMultiPoint( SIP_PYOBJECT points SIP_TYPEHINT( Sequence[Union[QgsPoint, QgsPointXY, Sequence[float]]] ) ) SIP_HOLDGIL [( const QVector<QgsPoint> &points )];
    % MethodCode
    if ( !PySequence_Check( a0 ) )
    {
      PyErr_SetString( PyExc_TypeError, QStringLiteral( "A sequence of QgsPoint, QgsPointXY or array of floats is expected" ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else
    {
      int state;
      const int size = PySequence_Size( a0 );
      QVector< QgsPoint * > pointList;
      pointList.reserve( size );

      sipIsErr = 0;
      for ( int i = 0; i < size; ++i )
      {
        PyObject *value = PySequence_GetItem( a0, i );
        if ( !value )
        {
          qDeleteAll( pointList );
          pointList.clear();
          PyErr_SetString( PyExc_TypeError, QStringLiteral( "Invalid type at index %1." ).arg( i ) .toUtf8().constData() );
          sipIsErr = 1;
          break;
        }

        if ( PySequence_Check( value ) )
        {
          const int elementSize = PySequence_Size( value );
          if ( elementSize < 2 || elementSize > 4 )
          {
            qDeleteAll( pointList );
            pointList.clear();
            sipIsErr = 1;
            PyErr_SetString( PyExc_TypeError, QStringLiteral( "Invalid sequence size at index %1. Expected an array of 2-4 float values, got %2." ).arg( i ).arg( elementSize ).toUtf8().constData() );
            Py_DECREF( value );
            break;
          }
          else
          {
            sipIsErr = 0;

            PyObject *element = PySequence_GetItem( value, 0 );
            if ( !element )
            {
              qDeleteAll( pointList );
              pointList.clear();
              PyErr_SetString( PyExc_TypeError, QStringLiteral( "Invalid type at index %1." ).arg( i ) .toUtf8().constData() );
              sipIsErr = 1;
              break;
            }

            PyErr_Clear();
            const double x = PyFloat_AsDouble( element );
            Py_DECREF( element );
            if ( PyErr_Occurred() )
            {
              qDeleteAll( pointList );
              pointList.clear();
              Py_DECREF( value );
              sipIsErr = 1;
              break;
            }

            element = PySequence_GetItem( value, 1 );
            if ( !element )
            {
              qDeleteAll( pointList );
              pointList.clear();
              Py_DECREF( value );
              PyErr_SetString( PyExc_TypeError, QStringLiteral( "Invalid type at index %1." ).arg( i ) .toUtf8().constData() );
              sipIsErr = 1;
              break;
            }

            PyErr_Clear();
            const double y = PyFloat_AsDouble( element );
            Py_DECREF( element );
            if ( PyErr_Occurred() )
            {
              qDeleteAll( pointList );
              pointList.clear();
              Py_DECREF( value );
              sipIsErr = 1;
              break;
            }

            std::unique_ptr< QgsPoint > point = std::make_unique< QgsPoint >( x, y );
            if ( elementSize > 2 )
            {
              element = PySequence_GetItem( value, 2 );
              if ( !element )
              {
                qDeleteAll( pointList );
                pointList.clear();
                Py_DECREF( value );
                PyErr_SetString( PyExc_TypeError, QStringLiteral( "Invalid type at index %1." ).arg( i ) .toUtf8().constData() );
                sipIsErr = 1;
                break;
              }

              PyErr_Clear();
              const double z = PyFloat_AsDouble( element );
              Py_DECREF( element );
              if ( PyErr_Occurred() )
              {
                qDeleteAll( pointList );
                pointList.clear();
                Py_DECREF( value );
                sipIsErr = 1;
                break;
              }
              point->addZValue( z );
            }
            if ( elementSize > 3 )
            {
              element = PySequence_GetItem( value, 3 );
              if ( !element )
              {
                qDeleteAll( pointList );
                pointList.clear();
                Py_DECREF( value );
                PyErr_SetString( PyExc_TypeError, QStringLiteral( "Invalid type at index %1." ).arg( i ) .toUtf8().constData() );
                sipIsErr = 1;
                break;
              }

              PyErr_Clear();
              const double m = PyFloat_AsDouble( element );
              Py_DECREF( element );
              if ( PyErr_Occurred() )
              {
                qDeleteAll( pointList );
                pointList.clear();
                Py_DECREF( value );
                sipIsErr = 1;
                break;
              }
              point->addMValue( m );
            }
            pointList.append( point.release() );

            Py_DECREF( value );
            if ( sipIsErr )
            {
              qDeleteAll( pointList );
              pointList.clear();
              break;
            }
          }
        }
        else
        {
          if ( sipCanConvertToType( value, sipType_QgsPointXY, SIP_NOT_NONE ) )
          {
            sipIsErr = 0;
            QgsPointXY *p = reinterpret_cast<QgsPointXY *>( sipConvertToType( value, sipType_QgsPointXY, 0, SIP_NOT_NONE, &state, &sipIsErr ) );
            if ( !sipIsErr )
            {
              pointList.append( new QgsPoint( p->x(), p->y() ) );
            }
            sipReleaseType( p, sipType_QgsPointXY, state );
          }
          else if ( sipCanConvertToType( value, sipType_QgsPoint, SIP_NOT_NONE ) )
          {
            sipIsErr = 0;
            QgsPoint *p = reinterpret_cast<QgsPoint *>( sipConvertToType( value, sipType_QgsPoint, 0, SIP_NOT_NONE, &state, &sipIsErr ) );
            if ( !sipIsErr )
            {
              pointList.append( p->clone() );
            }
            sipReleaseType( p, sipType_QgsPoint, state );
          }
          else
          {
            sipIsErr = 1;
          }

          Py_DECREF( value );

          if ( sipIsErr )
          {
            qDeleteAll( pointList );
            pointList.clear();
            // couldn't convert the sequence value to a QgsPoint or QgsPointXY
            PyErr_SetString( PyExc_TypeError, QStringLiteral( "Invalid type at index %1. Expected QgsPoint, QgsPointXY or array of floats." ).arg( i ) .toUtf8().constData() );
            break;
          }
        }
      }
      if ( sipIsErr == 0 )
        sipCpp = new sipQgsMultiPoint( QgsMultiPoint( pointList ) );
    }
    % End
#endif

    /**
     * Construct a multipoint from arrays of coordinates. If the z or m
     * arrays are non-empty then the resultant multipoint will have
     * z and m types accordingly.
     * This constructor is more efficient then calling addGeometry() repeatedly.
     *
     * If the sizes of \a x and \a y are non-equal then the resultant multipoint
     * will be created using the minimum size of these arrays.
     *
     * \since QGIS 3.34
     */
    QgsMultiPoint( const QVector<double> &x, const QVector<double> &y,
                   const QVector<double> &z = QVector<double>(),
                   const QVector<double> &m = QVector<double>() ) SIP_HOLDGIL;

#ifndef SIP_RUN

    /**
     * Returns the point with the specified \a index.
     *
     * \since QGIS 3.16
     */
    QgsPoint *pointN( int index );
#else

    /**
     * Returns the point with the specified \a index.
     *
     * \throws IndexError if no point with the specified index exists.
     *
     * \since QGIS 3.16
     */
    SIP_PYOBJECT pointN( int index ) SIP_TYPEHINT( QgsPoint );
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->numGeometries() )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      return sipConvertFromType( sipCpp->pointN( a0 ), sipType_QgsPoint, NULL );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the point with the specified \a index.
     *
     * \note Not available in Python bindings
     *
     * \since QGIS 3.16
     */
    const QgsPoint *pointN( int index ) const;
#endif

    QString geometryType() const override;
    QgsMultiPoint *clone() const override SIP_FACTORY;
    QgsMultiPoint *toCurveType() const override SIP_FACTORY;
    bool fromWkt( const QString &wkt ) override;
    void clear() override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    json asJsonObject( int precision = 17 ) const override SIP_SKIP;
    int nCoordinates() const override SIP_HOLDGIL;
    bool addGeometry( QgsAbstractGeometry *g SIP_TRANSFER ) override;
    bool addGeometries( const QVector< QgsAbstractGeometry * > &geometries SIP_TRANSFER ) final;
    bool insertGeometry( QgsAbstractGeometry *g SIP_TRANSFER, int index ) override;
    QgsAbstractGeometry *boundary() const override SIP_FACTORY;
    int vertexNumberFromVertexId( QgsVertexId id ) const override;
    double segmentLength( QgsVertexId startVertex ) const override;
    bool isValid( QString &error SIP_OUT, Qgis::GeometryValidityFlags flags = Qgis::GeometryValidityFlags() ) const override SIP_HOLDGIL;
    QgsMultiPoint *simplifyByDistance( double tolerance ) const override SIP_FACTORY;

#ifndef SIP_RUN
    void filterVertices( const std::function< bool( const QgsPoint & ) > &filter ) override;

    /**
     * Cast the \a geom to a QgsLineString.
     * Should be used by qgsgeometry_cast<QgsLineString *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     */
    inline static const QgsMultiPoint *cast( const QgsAbstractGeometry *geom ) // cppcheck-suppress duplInheritedMember
    {
      if ( geom && QgsWkbTypes::flatType( geom->wkbType() ) == Qgis::WkbType::MultiPoint )
        return static_cast<const QgsMultiPoint *>( geom );
      return nullptr;
    }
#endif

    QgsMultiPoint *createEmptyWithSameType() const override SIP_FACTORY;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString wkt = sipCpp->asWkt();
    if ( wkt.length() > 1000 )
      wkt = wkt.left( 1000 ) + QStringLiteral( "..." );
    QString str = QStringLiteral( "<QgsMultiPoint: %1>" ).arg( wkt );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  protected:

    bool wktOmitChildType() const override;

};

// clazy:excludeall=qstring-allocations

#endif // QGSMULTIPOINT_H
