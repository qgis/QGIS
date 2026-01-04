/***************************************************************************
               qgsprojoperation.h
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
#ifndef QGSPROJOPERATION_H
#define QGSPROJOPERATION_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QString>

/**
 * \ingroup core
 * \brief Contains information about a PROJ operation.
 *
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsProjOperation
{
  public:

    /**
     * Returns TRUE if the body is a valid object, or FALSE if it is a null/invalid
     * object.
     */
    bool isValid() const { return mValid; }

    /**
     * ID of operation.
     */
    QString id() const { return mId; }

    /**
     * Description.
     */
    QString description() const { return mDescription; }

    /**
     * Additional details.
     */
    QString details() const { return mDetails; }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str;
    if ( !sipCpp->isValid() )
    {
      str = u"<QgsProjOperation: invalid>"_s;
    }
    else
    {
      str = u"<QgsProjOperation: %1>"_s.arg( sipCpp->id() );
    }
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:

    bool mValid = false;
    QString mId;
    QString mDescription;
    QString mDetails;

    friend class QgsCoordinateReferenceSystemRegistry;
    friend class QgsCoordinateReferenceSystem;
};

#endif // QGSCELESTIALBODY_H
