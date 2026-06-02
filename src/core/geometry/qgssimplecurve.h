/***************************************************************************
                       qgssimplecurve.h
                         ------------
    begin                : May 2026
    copyright            : (C) 2026 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSIMPLECURVE_H
#define QGSSIMPLECURVE_H

#include "qgscurve.h"

/**
 * \ingroup core
 * \class QgsSimpleCurve
 * \brief Abstract base class for simple curved geometry type.
 *
 * This class is not part of the SQL/MM standard and only exists for implementation convenience.
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsSimpleCurve : public QgsCurve SIP_ABSTRACT
{
  public:
    QgsSimpleCurve() = default;

#ifndef SIP_RUN

    /**
   * Returns the specified point from inside the simple curve.
   * \param i index of point, starting at 0 for the first point
   */
    QgsPoint pointN( int i ) const;
#else
    // clang-format off

  /**
   * Returns the point at the specified index.
   *
   * - Indexes can be less than 0, in which case they correspond to positions from the end of the simple curve. E.g. an index of -1
   *
   * corresponds to the last point in the simple curve.
   *
   * - \throws IndexError if no point with the specified index exists.
   */
  SIP_PYOBJECT pointN( int i ) const SIP_TYPEHINT( QgsPoint );
  % MethodCode
      const int count = sipCpp->numPoints();
  if ( a0 < -count || a0 >= count )
  {
    PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
    sipIsErr = 1;
  }
  else
  {
    std::unique_ptr< QgsPoint > p;
    if ( a0 >= 0 )
      p = std::make_unique< QgsPoint >( sipCpp->pointN( a0 ) );
    else // negative index, count backwards from end
      p = std::make_unique< QgsPoint >( sipCpp->pointN( count + a0 ) );
    sipRes = sipConvertFromType( p.release(), sipType_QgsPoint, Py_None );
  }
  % End
// clang-format on
#endif

    /**
   * Returns a const pointer to the x vertex data.
   * \note Not available in Python bindings
   * \see yData()
   */
    const double *xData() const SIP_SKIP { return mX.constData(); }

    /**
   * Returns a const pointer to the y vertex data.
   * \note Not available in Python bindings
   * \see xData()
   */
    const double *yData() const SIP_SKIP { return mY.constData(); }

    /**
   * Returns a const pointer to the z vertex data, or NULLPTR if the simple curve does
   * not have z values.
   * \note Not available in Python bindings
   * \see xData()
   * \see yData()
   */
    const double *zData() const SIP_SKIP
    {
      if ( mZ.empty() )
        return nullptr;
      else
        return mZ.constData();
    }

    /**
   * Returns a const pointer to the m vertex data, or NULLPTR if the simple curve does
   * not have m values.
   * \note Not available in Python bindings
   * \see xData()
   * \see yData()
   */
    const double *mData() const SIP_SKIP
    {
      if ( mM.empty() )
        return nullptr;
      else
        return mM.constData();
    }

    /**
   * Returns the x vertex values as a vector.
   * \note Not available in Python bindings
   */
    QVector< double > xVector() const SIP_SKIP { return mX; }

    /**
   * Returns the y vertex values as a vector.
   * \note Not available in Python bindings
   */
    QVector< double > yVector() const SIP_SKIP { return mY; }

    /**
   * Returns the z vertex values as a vector.
   * \note Not available in Python bindings
   */
    QVector< double > zVector() const SIP_SKIP { return mZ; }

    /**
   * Returns the m vertex values as a vector.
   * \note Not available in Python bindings
   */
    QVector< double > mVector() const SIP_SKIP { return mM; }

    // Overrides
    int numPoints() const override SIP_HOLDGIL;
    int nCoordinates() const override SIP_HOLDGIL;
    int dimension() const override SIP_HOLDGIL;

    bool addMValue( double mValue = 0 ) override;
    bool addZValue( double zValue = 0 ) override;
    bool dropMValue() override;
    bool dropZValue() override;

protected:
  QVector<double> mX;
  QVector<double> mY;
  QVector<double> mZ;
  QVector<double> mM;
};

#endif // QGSSIMPLECURVE_H
