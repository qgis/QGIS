/***************************************************************************
    qgshighlightmaterial.cpp
    ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshighlightmaterial.h"

#include "qgssettings.h"

#include <QString>

#include "moc_qgshighlightmaterial.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE

QgsHighlightMaterial::QgsHighlightMaterial( QNode *parent )
  : QgsUnlitMaterial( parent )
{
  const QgsSettings settings;
  const float alpha = settings.value( u"Map/highlight/colorAlpha"_s, Qgis::DEFAULT_HIGHLIGHT_COLOR.alpha() ).toFloat() / 255.f;
  QColor color = QColor( settings.value( u"Map/highlight/color"_s, Qgis::DEFAULT_HIGHLIGHT_COLOR.name() ).toString() );
  color.setAlphaF( alpha );
  setColor( color );
}


///@endcond PRIVATE
