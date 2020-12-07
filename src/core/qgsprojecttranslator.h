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
#include <QString>

/**
 * \ingroup core
 * Wherever an object of this class is available, the derived translate function can be called from.
 *
 * \since QGIS 3.4
 */

class CORE_EXPORT QgsProjectTranslator
{
  public:

    /**
     * The derived translate() translates with QTranslator and qm file the sourceText.
     * It \returns the result string and in case there is no QTranslator loaded, the sourceText.
     * This function can be called from wherever the QgsReadWriteContext is available.
     *
     * \param context describing layer etc.
     * \param sourceText is the identifier of this text
     * \param disambiguation it's the disambiguation
     * \param n if -1 uses the appropriate form
     *
     * \since QGIS 3.4
     */

    virtual QString translate( const QString &context, const QString &sourceText, const char *disambiguation = nullptr, int n = -1 ) const = 0;

    virtual ~QgsProjectTranslator() = default;
};

#endif // QGSPROJECTTRANSLATOR_H
