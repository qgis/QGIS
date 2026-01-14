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
#include "qgsgoochmaterialsettings.h"
#include "qgsline3dsymbol.h"
#include "qgslogger.h"
#include "qgsmetalroughmaterialsettings.h"
#include "qgsphongmaterialsettings.h"
#include "qgsphongtexturedmaterialsettings.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"
#include "qgssimplelinematerialsettings.h"

#include <QString>

using namespace Qt::StringLiterals;

QColor Qgs3DSymbolUtils::vectorSymbolAverageColor( const QgsAbstract3DSymbol *symbol )
{
  QColor color = Qt::black;

  QgsAbstractMaterialSettings *materialSettings = nullptr;
  if ( symbol->type() == "line"_L1 )
  {
    const QgsLine3DSymbol *lineSymbol = dynamic_cast<const QgsLine3DSymbol *>( symbol );
    materialSettings = lineSymbol->materialSettings();
  }
  else if ( symbol->type() == "point"_L1 )
  {
    const QgsPoint3DSymbol *pointSymbol = dynamic_cast<const QgsPoint3DSymbol *>( symbol );
    materialSettings = pointSymbol->materialSettings();
  }
  else if ( symbol->type() == "polygon"_L1 )
  {
    const QgsPolygon3DSymbol *polygonSymbol = dynamic_cast<const QgsPolygon3DSymbol *>( symbol );
    materialSettings = polygonSymbol->materialSettings();
  }

  if ( materialSettings )
  {
    color = materialSettings->averageColor();
  }
  else
  {
    QgsDebugError( u"Qgs3DSymbolUtils::vectorMaterialAverageColor: unable to retrieve material from symbol"_s );
    color = Qt::black;
  }

  return color;
}
