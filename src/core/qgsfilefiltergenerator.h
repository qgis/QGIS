/***************************************************************************
                         qgsfilefiltergenerator.h
                         ------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#ifndef QGSFILEFILTERGENERATOR_H
#define QGSFILEFILTERGENERATOR_H

#include "qgis_core.h"
#include <QString>

/**
 * \ingroup core
 * \brief Abstract interface for classes which generate a file filter string.
 *
 * This interface can be inherited by classes which can generate a file filter
 * string, for use in file open or file save dialogs.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsFileFilterGenerator
{
  public:
    virtual ~QgsFileFilterGenerator() = default;

    /**
     * This method needs to be reimplemented in all classes which implement this interface
     * and return a file filter.
     */
    virtual QString createFileFilter() const = 0;

};

#endif // QGSPROCESSINGUTILS_H


