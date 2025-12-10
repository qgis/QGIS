/***************************************************************************
                             qgsmagneticmodel.h
                             ---------------------------
    begin                : December 2025
    copyright            : (C) 2025 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAGNETICMODEL_H
#define QGSMAGNETICMODEL_H

#include "qgsconfig.h"

#include "qgis.h"
#include "qgis_core.h"
#include "qgis_sip.h"

#ifndef SIP_RUN
#ifdef WITH_GEOGRAPHICLIB
namespace GeographicLib
{
  class MagneticModel;
}
#endif
#endif

/**
 * \ingroup core
 * \class QgsMagneticModel
 * \brief Represents a model of the Earth's magnetic field.
 *
 * \warning This class is only usable if QGIS was built with GeographicLib support. If not,
 * then QgsNotSupportedException exceptions will be raised when calling the
 * methods for this class.
 *
 * The Qgis::hasGeographicLib() method can be used to determine if QGIS
 * was built with GeographicLib support.
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsMagneticModel
{
  public:

    /**
     * Returns the default path used by GeographicLib to search for magnetic models.
     *
     * \see defaultModelName()
    */
    static QString defaultFilePath() SIP_THROW( QgsNotSupportedException );

    /**
     * Returns the name of the default magnetic model used by GeographicLib.
     *
     * \see defaultFilePath()
     */
    static QString defaultModelName() SIP_THROW( QgsNotSupportedException );

    /**
     * Constructor for QgsMagneticModel.
     *
     * A filename is formed by appending ".wmm" (World Magnetic Model) to the \a name argument.
     *
     * If \a path is specified (and is non-empty), then the associated file is loaded from this path. Otherwise the file path is given by the defaultFilePath().
     *
     * The associated file must exist and point to a valid magnetic model file. If not, then isValid() will return FALSE and a descriptive
     * error can be retrieved by error().
     *
     * If \a nMax ≥ 0 and \a mMax < 0, then \a mMax is set to \a nMax. After the model is loaded, the maximum degree and
     * order of the model can be found by the degree() and order() methods.
     *
     * \param name the name of the model.
     * \param path optional directory for data file. If not specified then defaultFilePath() is used.
     * \param nMax if nonnegative, truncate the degree of the model this value
     * \param mMax if nonnegative, truncate the order of the model this value
     */
    QgsMagneticModel( const QString &name, const QString &path = QString(), int nMax = -1, int mMax = -1 );
    ~QgsMagneticModel();

    QgsMagneticModel( const QgsMagneticModel &other ) = delete;
    QgsMagneticModel &operator=( const QgsMagneticModel &other ) = delete;

    /**
     * Returns TRUE if the model is valid and can be used.
     *
     * \throws QgsNotSupportedException if GeographicLib is not available
     *
     * \see error()
     */
    bool isValid() const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns the description of the magnetic model, if available.
     *
     * \throws QgsNotSupportedException if GeographicLib is not available
     */
    QString description() const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns the date of the magnetic model, if available.
     *
     * \throws QgsNotSupportedException if GeographicLib is not available
     */
    QDateTime dateTime() const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns the full file name for the magnetic model.
     *
     * \throws QgsNotSupportedException if GeographicLib is not available
     */
    QString file() const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns the full directory name containing the magnetic model file.
     *
     * \throws QgsNotSupportedException if GeographicLib is not available
     */
    QString directory() const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns the name of the magnetic model.
     *
     * \throws QgsNotSupportedException if GeographicLib is not available
     */
    QString name() const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns a string for the last error encountered by the model.
     */
    QString error() const { return mError; }

    /**
     * Returns the minimum height above the ellipsoid (in meters) for which the model should be used.
     *
     * \throws QgsNotSupportedException if GeographicLib is not available
     * \see maximumHeight()
     */
    double minimumHeight() const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns the maximum height above the ellipsoid (in meters) for which the model should be used.
     *
     * \throws QgsNotSupportedException if GeographicLib is not available
     * \see minimumHeight()
     */
    double maximumHeight() const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns the minimum time (in decimal years) for which the model should be used.
     *
     * \throws QgsNotSupportedException if GeographicLib is not available
     * \see maximumYear()
     */
    double minimumYear() const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns the maximum time (in decimal years) for which the model should be used.
     *
     * \throws QgsNotSupportedException if GeographicLib is not available
     * \see minimumYear()
     */
    double maximumYear() const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns the equatorial radius of the associated ellipsoid (in meters).
     *
     * \throws QgsNotSupportedException if GeographicLib is not available
     * \see flattening()
     */
    double equatorialRadius() const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns the flattening of the associated ellipsoid.
     *
     * \throws QgsNotSupportedException if GeographicLib is not available
     * \see equatorialRadius()
     */
    double flattening() const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns the maximum degree of the components of the model.
     *
     * \throws QgsNotSupportedException if GeographicLib is not available
     * \see order()
     */
    int degree() const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns the maximum order of the components of the model.
     *
     * \throws QgsNotSupportedException if GeographicLib is not available
     * \see degree()
     */
    int order() const SIP_THROW( QgsNotSupportedException );

    /**
     * Evaluates the components of the geomagnetic field at a point.
     *
     * \param years the time (in fractional years)
     * \param latitude latitude of the point (in decimal degrees)
     * \param longitude longitude of the point (in decimal degrees)
     * \param height height of the point above the ellipsoid (in meters)
     * \param Bx the easterly component of the magnetic field (in nanotesla).
     * \param By the northerly component of the magnetic field (in nanotesla)
     * \param Bz the vertical (up) component of the magnetic field (in nanotesla)
     *
     * \returns TRUE if the components were successfully calculated
     *
     * \see getComponentsWithTimeDerivatives()
     */
    bool getComponents( double years, double latitude, double longitude, double height, double &Bx SIP_OUT, double &By SIP_OUT, double &Bz SIP_OUT ) const SIP_THROW( QgsNotSupportedException );

    /**
     * Evaluates the components of the geomagnetic field at a point, and their time derivatives.
     *
     * \param years the time (in fractional years)
     * \param latitude latitude of the point (in decimal degrees)
     * \param longitude longitude of the point (in decimal degrees)
     * \param height height of the point above the ellipsoid (in meters)
     * \param Bx the easterly component of the magnetic field (in nanotesla).
     * \param By the northerly component of the magnetic field (in nanotesla)
     * \param Bz the vertical (up) component of the magnetic field (in nanotesla)
     * \param Bxt the rate of change of Bx (nT/yr)
     * \param Byt the rate of change of By (nT/yr)
     * \param Bzt the rate of change of Bz (nT/yr)
     *
     * \returns TRUE if the components were successfully calculated
     *
     * \see getComponents()
     */
    bool getComponentsWithTimeDerivatives( double years, double latitude, double longitude, double height, double &Bx SIP_OUT, double &By SIP_OUT, double &Bz SIP_OUT, double &Bxt SIP_OUT, double &Byt SIP_OUT, double &Bzt SIP_OUT ) const SIP_THROW( QgsNotSupportedException );

    /**
     * Compute various quantities dependent on a magnetic field.
     *
     * \param Bx the easterly component of the magnetic field (in nanotesla)
     * \param By the northerly component of the magnetic field (in nanotesla)
     * \param Bz the vertical (up) component of the magnetic field (in nanotesla)
     * \param H the horizontal magnetic field (in nanotesla)
     * \param F the total magnetic field (in nanotesla)
     * \param D the declination of the field (degrees east of north)
     * \param I the inclination of the field (degrees down from horizontal)
     *
     * \returns TRUE if the components were successfully calculated
     *
     * \see fieldComponentsWithTimeDerivatives()
     */
    static bool fieldComponents( double Bx, double By, double Bz, double &H SIP_OUT, double &F SIP_OUT, double &D SIP_OUT, double &I SIP_OUT ) SIP_THROW( QgsNotSupportedException );

    /**
     * Compute various quantities dependent on a magnetic field and their rates of change.
     *
     * \param Bx the easterly component of the magnetic field (in nanotesla)
     * \param By the northerly component of the magnetic field (in nanotesla)
     * \param Bz the vertical (up) component of the magnetic field (in nanotesla)
     * \param Bxt the rate of change of Bx (nT/yr)
     * \param Byt the rate of change of By (nT/yr)
     * \param Bzt the rate of change of Bz (nT/yr)
     * \param H the horizontal magnetic field (in nanotesla)
     * \param F the total magnetic field (in nanotesla)
     * \param D the declination of the field (degrees east of north)
     * \param I the inclination of the field (degrees down from horizontal)
     * \param Ht the rate of change of H (nT/yr)
     * \param Ft the rate of change of F (nT/yr)
     * \param Dt the rate of change of D (degrees/yr)
     * \param It the rate of change of I (degrees/yr)
     *
     * \returns TRUE if the components were successfully calculated
     *
     * \see fieldComponentsWithTimeDerivatives()
     */
    static bool fieldComponentsWithTimeDerivatives( double Bx, double By, double Bz, double Bxt, double Byt, double Bzt, double &H SIP_OUT, double &F SIP_OUT, double &D SIP_OUT, double &I SIP_OUT, double &Ht SIP_OUT, double &Ft SIP_OUT, double &Dt SIP_OUT, double &It SIP_OUT ) SIP_THROW( QgsNotSupportedException );

  private:

#ifdef SIP_RUN
    QgsMagneticModel( const QgsMagneticModel &other );
#endif

    QString mName;
    QString mPath;

#ifdef WITH_GEOGRAPHICLIB
    std::unique_ptr< GeographicLib::MagneticModel > mModel;
#endif
    QString mError;

};

#endif // QGSMAGNETICMODEL_H
