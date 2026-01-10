/***************************************************************************
               qgscelestialbody.h
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
#ifndef QGSCELESTIALBODY_H
#define QGSCELESTIALBODY_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QString>

/**
 * \ingroup core
 * \brief Contains information about a celestial body.
 *
 * \note Only used in builds based on on PROJ 8.1 or later
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsCelestialBody
{
  public:

    /**
     * Returns TRUE if the body is a valid object, or FALSE if it is a null/invalid
     * object.
     */
    bool isValid() const { return mValid; }

    /**
     * Name of celestial body.
     */
    QString name() const { return mName; }

    /**
     * Authority name, e.g. EPSG.
     */
    QString authority() const { return mAuthority; }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str;
    if ( !sipCpp->isValid() )
    {
      str = u"<QgsCelestialBody: invalid>"_s;
    }
    else
    {
      QString id;
      if ( !sipCpp->authority().isEmpty() )
        id = u"%1 (%2)"_s.arg( sipCpp->name(), sipCpp->authority() );
      else
        id = sipCpp->name();
      str = u"<QgsCelestialBody: %1>"_s.arg( id );
    }
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:

    bool mValid = false;
    QString mName;
    QString mAuthority;

    friend class QgsCoordinateReferenceSystemRegistry;
};

#endif // QGSCELESTIALBODY_H
