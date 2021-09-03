/***************************************************************************
                             qgscommandlineutils.h
                             ---------------------------
    begin                : June 2021
    copyright            : (C) 2021 by Etienne Trimaille
    email                : etienne dot trimaille at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMMANDLINEUTILS_H
#define QGSCOMMANDLINEUTILS_H

#include "qgis.h"

/**
 * \ingroup core
 * \class QgsCommandLineUtils
 * \brief Utils class for QGIS command line tools.
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsCommandLineUtils
{
  public:

    /**
     * Display all versions in the standard output stream
     * \since QGIS 3.22
     */
    static QString allVersions( );

};

#endif // QGSCOMMANDLINEUTILS_H
