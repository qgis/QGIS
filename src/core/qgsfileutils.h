/***************************************************************************
                             qgsfileutils.h
                             ---------------------------
    begin                : November 2017
    copyright            : (C) 2017 by Etienne Trimaille
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

#ifndef QGSFILEUTILS_H
#define QGSFILEUTILS_H

#include "qgis.h"

/**
 * \ingroup core
 * \class QgsFileUtils
 * \brief Class for file utilities.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsFileUtils
{
  public:

    /**
     * Return the human size from bytes
     */
    static QString representFileSize( qint64 bytes );

};

#endif // QGSFILEUTILS_H
