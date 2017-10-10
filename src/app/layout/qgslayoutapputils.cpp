/***************************************************************************
                             qgslayoutapputils.cpp
                             ---------------------
    Date                 : October 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutapputils.h"
#include "qgsgui.h"
#include "qgslayoutitemguiregistry.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutviewrubberband.h"

void QgsLayoutAppUtils::registerGuiForKnownItemTypes()
{
  QgsLayoutItemGuiRegistry *registry = QgsGui::layoutItemGuiRegistry();

  registry->addItemGroup( QgsLayoutItemGuiGroup( QStringLiteral( "shapes" ), QObject::tr( "Shape" ), QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicShape.svg" ) ) ) );

  auto createRubberBand = ( []( QgsLayoutView * view )->QgsLayoutViewRubberBand *
  {
    return new QgsLayoutViewRectangularRubberBand( view );
  } );
  auto createEllipseBand = ( []( QgsLayoutView * view )->QgsLayoutViewRubberBand *
  {
    return new QgsLayoutViewEllipticalRubberBand( view );
  } );
  auto createTriangleBand = ( []( QgsLayoutView * view )->QgsLayoutViewRubberBand *
  {
    return new QgsLayoutViewTriangleRubberBand( view );
  } );

  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutItem + 1002, QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddLabel.svg" ) ), nullptr, createRubberBand ) );

  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutMap, QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddMap.svg" ) ), nullptr, createRubberBand ) );

  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutRectangle, QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicRectangle.svg" ) ), nullptr, createRubberBand, QStringLiteral( "shapes" ) ) );
  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutEllipse, QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicCircle.svg" ) ), nullptr, createEllipseBand, QStringLiteral( "shapes" ) ) );
  registry->addLayoutItemGuiMetadata( new QgsLayoutItemGuiMetadata( QgsLayoutItemRegistry::LayoutTriangle, QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicTriangle.svg" ) ), nullptr, createTriangleBand, QStringLiteral( "shapes" ) ) );
}
