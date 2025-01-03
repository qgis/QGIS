/***************************************************************************
    qgsrenderedlayerstatistics.h
    ----------------
    copyright            : (C) 2024 by Jean Felder
    email                : jean dot felder at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRENDEREDLAYERSTATISTICS_H
#define QGSRENDEREDLAYERSTATISTICS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrendereditemdetails.h"

#include <limits>


/**
 * \ingroup core
 * \brief Contains computed statistics for a layer render.
 * \since QGIS 3.42
 */
class CORE_EXPORT QgsRenderedLayerStatistics: public QgsRenderedItemDetails
{
  public:

    /**
     * \brief Constructor for QgsRenderedLayerStatistics from a list of \a minimum and \a maximum.
     *
     * This is used to store the minimum and maximum values of a layer. Many values may be stored.
     * For example, a raster layer may have multiple bands. In that case, \a minimum will contain the minimum
     * value of each band and \a maximum will contain the maximum value of each band.
     */
    QgsRenderedLayerStatistics( const QString &layerId, const QList<double> &minimum, const QList<double> &maximum );

    /**
     * Constructor for QgsRenderedLayerStatistics with only one value for \a minimum and \a maximum.
     */
#ifndef SIP_RUN
    QgsRenderedLayerStatistics( const QString &layerId, double minimum = std::numeric_limits<double>::quiet_NaN(), double maximum = std::numeric_limits<double>::quiet_NaN() );
#else
    QgsRenderedLayerStatistics( const QString &layerId, SIP_PYOBJECT minimum SIP_TYPEHINT( Optional[float] ) = Py_None, SIP_PYOBJECT maximum SIP_TYPEHINT( Optional[float] ) = Py_None ) [( const QString & layerId, double minimum = 0.0, double maximum = 0.0 )];
    % MethodCode
    double minP = a1 == Py_None ? std::numeric_limits<double>::quiet_NaN() : PyFloat_AsDouble( a1 );
    double maxP = a2 == Py_None ? std::numeric_limits<double>::quiet_NaN() : PyFloat_AsDouble( a2 );
    QList<double> minL = {minP};
    QList<double> maxL = {maxP};
    sipCpp = new sipQgsRenderedLayerStatistics( *a0, minL, maxL );
    % End
#endif

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QStringList minimums;
    minimums.reserve( sipCpp->minimum().size() );
    for ( double min : sipCpp->minimum() )
    {
      minimums.append( QString::number( min ) );
    }
    QStringList maximums;
    maximums.reserve( sipCpp->maximum().size() );
    for ( double max : sipCpp->maximum() )
    {
      maximums.append( QString::number( max ) );
    }
    QString str = QStringLiteral( "<QgsRenderedLayerStatistics: %1 (min: %2 - max: %3)>" ).arg( sipCpp->layerId(), minimums.join( ',' ), maximums.join( ',' ) );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * Returns the minimum values of the computed statistics.
     * \see setMinimum()
     */
    QList<double> minimum() const;

    /**
     * Returns the minimum value of the computed statistics at \a index.
     * \see setMinimum()
     */
    double minimum( int index ) const;

    /**
     * Returns the maximum values of the computed statistics.
     * \see setMaximum()
     */
    QList<double> maximum() const;

    /**
      * Returns the maximum values of the computed statistics at \a index.
      * \see setMaximum()
      */
    double maximum( int index ) const;

    /**
     * Sets the minimum values of the computed statistics.
     * \see minimum()
     */
    void setMinimum( QList<double> &minimum );

    /**
     * Sets the minimum value of the computed statistics at \a index.
     * \returns TRUE if value has been set.
     * \see minimum()
     */
    bool setMinimum( int index, double minimum );

    /**
     * Sets the maximum values of the computed statistics.
     * \see maximum()
     */
    void setMaximum( QList<double> &maximum );

    /**
     * Sets the maximum value of the computed statistics at \a index.
     * \returns TRUE if value has been set.
     * \see maximum()
     */
    bool setMaximum( int index, double maximum );

  private:

    QList<double> mMin;
    QList<double> mMax;
};

#endif // QGSRENDEREDLAYERSTATISTICS_H
