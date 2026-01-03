/***************************************************************************
               qgsdatums.h
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
#ifndef QGSDATUMS_H
#define QGSDATUMS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrectangle.h"

#include <QList>
#include <QString>

/**
 * \ingroup core
 * \brief Contains information about a member of a datum ensemble.
 *
 * \note Only used in builds based on on PROJ 7.2 or later
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsDatumEnsembleMember
{
  public:

    /**
     * Returns the name of the member.
     */
    QString name() const { return mName; }

    /**
     * Returns the scope of operation, from EPSG registry database.
     */
    QString scope() const { return mScope; }

    /**
     * Remarks for operation, from EPSG registry database.
     */
    QString remarks() const { return mRemarks; }

    /**
     * Authority name, e.g. EPSG.
     */
    QString authority() const { return mAuthority; }

    /**
     * Authority code, e.g. "8447" (for EPSG:8447).
     */
    QString code() const { return mCode; }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString id;
    if ( !sipCpp->code().isEmpty() )
      id = u"%1 (%2:%3)"_s.arg( sipCpp->name(), sipCpp->authority(), sipCpp->code() );
    else
      id = sipCpp->name();
    QString str = u"<QgsDatumEnsembleMember: %1>"_s.arg( id );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:

    QString mName;
    QString mScope;
    QString mRemarks;
    QString mAuthority;
    QString mCode;

    friend class QgsCoordinateReferenceSystem;
};

/**
 * \ingroup core
 * \brief Contains information about a datum ensemble.
 *
 * \note Only used in builds based on on PROJ 7.2 or later
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsDatumEnsemble
{
  public:

    /**
     * Returns TRUE if the datum ensemble is a valid object, or FALSE if it is a null/invalid
     * object.
     */
    bool isValid() const { return mValid; }

    /**
     * Display name of datum ensemble.
     */
    QString name() const { return mName; }

    /**
     * Positional accuracy (in meters).
     */
    double accuracy() const { return mAccuracy; }

    /**
     * Authority name, e.g. EPSG.
     */
    QString authority() const { return mAuthority; }

    /**
     * Identification code, e.g. "8447" (For EPSG:8447).
     */
    QString code() const { return mCode; }

    /**
     * Scope of ensemble, from EPSG registry database.
     */
    QString scope() const { return mScope; }

    /**
    * Remarks for ensemble, from EPSG registry database.
    */
    QString remarks() const { return mRemarks; }

    /**
     * Contains a list of members of the ensemble.
     */
    QList< QgsDatumEnsembleMember > members() const { return mMembers; }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str;
    if ( !sipCpp->isValid() )
    {
      str = u"<QgsDatumEnsemble: invalid>"_s;
    }
    else
    {
      QString id;
      if ( !sipCpp->code().isEmpty() )
        id = u"%1 (%2:%3)"_s.arg( sipCpp->name(), sipCpp->authority(), sipCpp->code() );
      else
        id = sipCpp->name();
      str = u"<QgsDatumEnsemble: %1>"_s.arg( id );
    }
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:

    bool mValid = false;
    QString mName;
    double mAccuracy = 0;
    QString mAuthority;
    QString mCode;
    QString mScope;
    QString mRemarks;
    QList< QgsDatumEnsembleMember > mMembers;

    friend class QgsCoordinateReferenceSystem;
};

#endif // QGSDATUMS_H
