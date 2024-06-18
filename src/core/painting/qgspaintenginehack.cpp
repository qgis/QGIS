/***************************************************************************
  qgspaintenginehack.cpp
  Hack paint engine flags
  -------------------
         begin                : July 2012
         copyright            : (C) Juergen E. Fischer
         email                : jef at norbit dot de

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspaintenginehack.h"
#include "qgsconfig.h"

// Hack to workaround Qt #5114 by disabling PatternTransform
void QgsPaintEngineHack::fixFlags()
{
#if defined(HAS_KDE_QT5_PDF_TRANSFORM_FIX) || QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
  // not required, fixed upstream
#else
  gccaps = PaintEngineFeatures();
  gccaps |= ( QPaintEngine::PrimitiveTransform
              // | QPaintEngine::PatternTransform
              | QPaintEngine::PixmapTransform
              | QPaintEngine::PatternBrush
              // | QPaintEngine::LinearGradientFill
              // | QPaintEngine::RadialGradientFill
              // | QPaintEngine::ConicalGradientFill
              | QPaintEngine::AlphaBlend
              // | QPaintEngine::PorterDuff
              | QPaintEngine::PainterPaths
              | QPaintEngine::Antialiasing
              | QPaintEngine::BrushStroke
              | QPaintEngine::ConstantOpacity
              | QPaintEngine::MaskedBrush
              // | QPaintEngine::PerspectiveTransform
              | QPaintEngine::BlendModes
              // | QPaintEngine::ObjectBoundingModeGradients
              | QPaintEngine::RasterOpModes
              | QPaintEngine::PaintOutsidePaintEvent
            );
#endif
}

void QgsPaintEngineHack::fixEngineFlags( QPaintEngine *engine )
{
#if defined(HAS_KDE_QT5_PDF_TRANSFORM_FIX) || QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
  // not required, fixed upstream
  ( void )engine;
#else
  if ( !engine )
    return;

  QgsPaintEngineHack *hack = static_cast<QgsPaintEngineHack *>( engine );
  hack->fixFlags();
#endif
}
