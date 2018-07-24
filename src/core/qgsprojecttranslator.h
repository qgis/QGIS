/***************************************************************************
  qgsprojecttranslator.h

 ---------------------
 begin                : 24.7.2018
 copyright            : (C) 2018 by david signer
 email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROJECTTRANSLATOR_H
#define QGSPROJECTTRANSLATOR_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include <QString>

/**
 * \ingroup core
 * This abstract class is to call translate() for project data from wherever QgsReadWriteContext is available.
 *
 * \since QGIS 3.4
 */

class CORE_EXPORT QgsProjectTranslator
{
  public:

    /**
     * This method needs to be reimplemented in all classes which implement this interface
     *
     * \since QGIS 3.4
     */
    virtual QString translate( const QString &context, const QString &sourceText, const char *disambiguation = nullptr, int n = -1 ) const = 0;

    virtual ~QgsProjectTranslator() = default;
};

#endif // QGSPROJECTTRANSLATOR_H
