/***************************************************************************
                                  qgstestutils.h
                              ---------------------
    begin                : January 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTESTUTILS_H
#define QGSTESTUTILS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsfeaturerequest.h"

class QgsVectorDataProvider;

///@cond PRIVATE

// Used only for utilities required for the QGIS Python unit tests - not stable or public API
class CORE_EXPORT QgsTestUtils
{
  public:

    /**
     * Runs a thready safety test on iterators from a vector data \a provider, by concurrently
     * requesting features from the provider.
     *
     * This method returns TRUE... or it segfaults.
     */
    static bool testProviderIteratorThreadSafety( QgsVectorDataProvider *provider, const QgsFeatureRequest &request = QgsFeatureRequest() );

    /**
     * Compares two DOM elements and returns TRUE if they are equivalent.
     *
     * Dumps useful diff information to the console if the elements differ.
     */
    static bool compareDomElements( const QDomElement &element1, const QDomElement &element2 );

};

///@endcond
#endif //QGSTESTUTILS_H
