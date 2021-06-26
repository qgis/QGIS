/***************************************************************************
  qgspaintenginehack.cpp
  Hack paint engine flags
  -------------------
         begin                : July 2012
         copyright            : (C) Juergen E. Fischer
         email                : jef at norbit dot de

 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgspaintenginehack.h"

// Hack to workaround Qt #5114 by disabling PatternTransform
void QgsPaintEngineHack::fixFlags()
{
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
}

void QgsPaintEngineHack::fixEngineFlags( QPaintEngine *engine )
{
  if ( !engine )
    return;

  QgsPaintEngineHack *hack = static_cast<QgsPaintEngineHack *>( engine );
  hack->fixFlags();
}
