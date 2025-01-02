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
 * \brief An interface for objects which can translate project strings.
 *
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsProjectTranslator
{
  public:

    virtual ~QgsProjectTranslator() = default;

    /**
     * Translates a string using the Qt QTranslator mechanism.
     *
     * This function can be called from wherever the QgsReadWriteContext is available.
     *
     * \param context describes the context of the translation, eg the corresponding map layer
     * \param sourceText the identifier of the text to translate (usually the original untranslated string)
     * \param disambiguation optional string providing additional context for the translation
     * \param n optional "object count", which can alter the translation to account for plural forms
     *
     * \returns the translated string. In the case that there is no QTranslator available, the \a sourceText will be returned unchanged.
     *
     * \since QGIS 3.4
     */
    virtual QString translate( const QString &context, const QString &sourceText, const char *disambiguation = nullptr, int n = -1 ) const = 0;

};

#endif // QGSPROJECTTRANSLATOR_H
