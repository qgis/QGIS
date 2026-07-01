/***************************************************************************
    qgsaivisualcontextutils.h
    ---------------------
    begin                : June 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAIVISUALCONTEXTUTILS_H
#define QGSAIVISUALCONTEXTUTILS_H

#include "qgis_app.h"

#include <QImage>
#include <QString>

class QWidget;

class APP_EXPORT QgsAiVisualContextUtils
{
  public:
    static QString visualConsentSettingKey();
    static bool hasVisualConsent();
    static bool ensureVisualContextConsent( QWidget *parent );
    static QString visualContextDirectory();
    static void cleanupOldVisualContextFiles( const QString &dirPath );

    static bool isSupportedImagePath( const QString &path );
    static QString mimeTypeForImagePath( const QString &path );
    static QString saveDroppedImage( const QImage &image, const QString &format = QString() );
};

#endif // QGSAIVISUALCONTEXTUTILS_H
