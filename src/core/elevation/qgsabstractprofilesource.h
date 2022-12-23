/***************************************************************************
                         qgsabstractprofilesource.h
                         ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
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
#ifndef QGSABSTRACTPROFILESOURCE_H
#define QGSABSTRACTPROFILESOURCE_H

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsProfileRequest;
class QgsAbstractProfileGenerator;


/**
 * \brief Interface for classes which can generate elevation profiles.
 *
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsAbstractProfileSource
{

  public:

    virtual ~QgsAbstractProfileSource();

    /**
     * Given a profile \a request, returns a new profile generator ready for generating
     * elevation profiles.
     *
     * The caller takes ownership of the returned generator.
     *
     * May return NULLPTR if the source cannot generate a profile at this time.
     */
    virtual QgsAbstractProfileGenerator *createProfileGenerator( const QgsProfileRequest &request ) = 0 SIP_FACTORY;

};

#endif // QGSABSTRACTPROFILESOURCE_H
