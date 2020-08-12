/***************************************************************************
                                  qgspostgresstringutils.h
                              ---------------------
    begin                : July 2019
    copyright            : (C) 2019 by David Signer
    email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOSTGRESSTRINGUTILS_H
#define QGSPOSTGRESSTRINGUTILS_H

#include "qgis_core.h"
#include "qgis.h"
#include "qlist.h"

#ifdef SIP_RUN
% ModuleHeaderCode
#include "qgspostgresstringutils.h"
% End
#endif

/**
 * \ingroup core
 * The QgsPostgresStringUtils provides functions to handle postgres array like formatted lists in strings.
 * \since QGIS 3.8
 */
class CORE_EXPORT QgsPostgresStringUtils
{

  public:

    /**
     * Returns a QVariantList created out of a string containing an array in postgres array format {1,2,3} or {"a","b","c"}
     * \param string The formatted list in a string
     * \since QGIS 3.8
     */
    static QVariantList parseArray( const QString &string );

    /**
     * Build a postgres array like formatted list in a string from a QVariantList
     * \param list The list that needs to be stored to the string
     * \since QGIS 3.8
     */
    static QString buildArray( const QVariantList &list );

  private:

    /**
     * get the string until the separator
     * \param txt the input text
     * \param i the current position
     * \param sep the separator
     * \since QGIS 3.8
     */
    static QString getNextString( const QString &txt, int &i, const QString &sep );
};

#endif //QGSPOSTGRESSTRINGUTILS_H
