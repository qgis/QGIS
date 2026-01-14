/***************************************************************************
  qgs3dsymbolutils.cpp
  --------------------------------------
  Date                 : January 2026
  Copyright            : (C) 2026 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dsymbolutils.h"

#include "qgsabstract3dsymbol.h"
#include "qgsabstractmaterialsettings.h"
#include "qgsline3dsymbol.h"
#include "qgslogger.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"

#include <QColor>
#include <QString>

using namespace Qt::StringLiterals;

QColor Qgs3DSymbolUtils::vectorSymbolAverageColor( const QgsAbstract3DSymbol *symbol )
{
  QColor color = QColor();

  if ( !symbol )
  {
    return color;
  }

  QgsAbstractMaterialSettings *materialSettings = symbol->materialSettings();
  if ( materialSettings )
  {
    color = materialSettings->averageColor();
  }
  else
  {
    QgsDebugError( u"Qgs3DSymbolUtils::vectorMaterialAverageColor: unable to retrieve material from symbol"_s );
  }

  return color;
}
