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

#include <QList>
#include <QColor>

#include "qgssymbollayerv2utils.h"

class CORE_EXPORT QgsColorBrewerPalette
{
  public:
    static QList<QColor> listSchemeColors( QString schemeName, int colors )
    {
      QList<QColor> pal;
      QString palette( brewerString );
      QStringList list = palette.split( QChar( '\n' ) );
      Q_FOREACH ( const QString& entry, list )
      {
        QStringList items = entry.split( QChar( '-' ) );
        if ( items.count() != 3 || items[0] != schemeName || items[1].toInt() != colors )
          continue;
        QStringList colors = items[2].split( QChar( ' ' ) );
        Q_FOREACH ( const QString& clr, colors )
        {
          pal << QgsSymbolLayerV2Utils::parseColor( clr );
        }
      }
      return pal;
    }

    static QStringList listSchemes()
    {
      QStringList schemes;

      QString palette( brewerString );
      QStringList list = palette.split( QChar( '\n' ) );
      Q_FOREACH ( const QString& entry, list )
      {
        QStringList items = entry.split( QChar( '-' ) );
        if ( items.count() != 3 )
          continue;
        if ( !schemes.contains( items[0] ) )
          schemes << items[0];
      }
      return schemes;
    }

    static QList<int> listSchemeVariants( QString schemeName )
    {
      QList<int> variants;

      QString palette( brewerString );
      QStringList list = palette.split( QChar( '\n' ) );
      Q_FOREACH ( const QString& entry, list )
      {
        QStringList items = entry.split( QChar( '-' ) );
        if ( items.count() != 3 || items[0] != schemeName )
          continue;
        variants << items[1].toInt();
      }

      return variants;
    }

    // extracted ColorBrewer data
    static const char *brewerString;
};

#endif // QGSCOLORBREWERPALETTE_H
