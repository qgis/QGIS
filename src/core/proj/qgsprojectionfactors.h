/***************************************************************************
               qgsprojectionfactors.h
               ------------------------
    begin                : May 2021
    copyright            : (C) 2021 Nyall Dawson
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
#ifndef QGSPROJECTIONFACTORS_H
#define QGSPROJECTIONFACTORS_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QString>

/**
 * \class QgsProjectionFactors
 * \ingroup core
 * \brief Contains various cartographic properties, such as scale factors, angular distortion and meridian convergence.
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsProjectionFactors
{
  public:

    /**
     * Returns TRUE if the factors are valid, or FALSE if they could not be calculated.
     */
    bool isValid() const { return mIsValid; }

    //! Meridional scale at coordinate (λ,ϕ).
    double meridionalScale() const { return mMeridionalScale; }

    //! Parallel scale at coordinate (λ,ϕ).
    double parallelScale() const { return mParallelScale; }

    //! Areal scale factor at coordinate (λ,ϕ).
    double arealScale() const { return mArealScale; }

    //! Angular distortion at coordinate (λ,ϕ).
    double angularDistortion() const { return mAngularDistortion; }

    //! Meridian/parallel angle (in degrees), θ′, at coordinate (λ,ϕ).
    double meridianParallelAngle() const { return mMeridianParallelAngle; }

    //! Meridian convergence (in degrees) at coordinate (λ,ϕ). Sometimes also described as grid declination.
    double meridianConvergence() const { return mMeridianConvergence; }

    //! Maximum scale factor.
    double tissotSemimajor() const { return mTissotSemimajor; }

    //! Minimum scale factor.
    double tissotSemiminor() const { return mTissotSemiminor; }

    //! Partial derivative ∂x/∂λ of coordinate (λ,ϕ).
    double dxDlam() const  { return mDxDlam; }

    //! Partial derivative ∂x/∂ϕ of coordinate (λ,ϕ).
    double dxDphi() const  { return mDxDphi; }

    //! Partial derivative ∂y/∂λ of coordinate (λ,ϕ).
    double dyDlam() const  { return mDyDlam; }

    //!Partial derivative ∂y/∂ϕ of coordinate (λ,ϕ).
    double dyDphi() const  { return mDyDphi; }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str;
    if ( !sipCpp->isValid() )
    {
      str = u"<QgsProjectionFactors: invalid>"_s;
    }
    else
    {
      str = u"<QgsProjectionFactors>"_s;
    }
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:

    bool mIsValid = false;
    double mMeridionalScale = 0;
    double mParallelScale = 0;
    double mArealScale = 0;
    double mAngularDistortion = 0;
    double mMeridianParallelAngle = 0;
    double mMeridianConvergence = 0;
    double mTissotSemimajor = 0;
    double mTissotSemiminor = 0;
    double mDxDlam = 0;
    double mDxDphi = 0;
    double mDyDlam = 0;
    double mDyDphi = 0;

    friend class QgsCoordinateReferenceSystem;
};

#endif // QGSPROJECTIONFACTORS_H
