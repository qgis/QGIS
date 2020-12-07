/***************************************************************************
    qgscolorbrewerpalette.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOLORBREWERPALETTE_H
#define QGSCOLORBREWERPALETTE_H

#include "qgis_core.h"
#include <QList>
#include <QColor>

#include "qgssymbollayerutils.h"

/**
 * \ingroup core
 * \class QgsColorBrewerPalette
 */
class CORE_EXPORT QgsColorBrewerPalette
{
  public:
    static QList<QColor> listSchemeColors( const QString &schemeName, int colors );

    static QStringList listSchemes();

    static QList<int> listSchemeVariants( const QString &schemeName );

    // extracted ColorBrewer data
    static const char *BREWER_STRING;
};

// clazy:excludeall=qstring-allocations

#endif // QGSCOLORBREWERPALETTE_H
