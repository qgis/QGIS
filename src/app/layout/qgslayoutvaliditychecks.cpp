/***************************************************************************
                              qgslayoutvaliditychecks.cpp
                              ---------------------------
    begin                : November 2018
    copyright            : (C) 2018 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutvaliditychecks.h"
#include "qgsvaliditycheckcontext.h"
#include "qgslayoutitemscalebar.h"
#include "qgslayout.h"

QgsLayoutScaleBarValidityCheck *QgsLayoutScaleBarValidityCheck::create() const
{
  return new QgsLayoutScaleBarValidityCheck();
}

QString QgsLayoutScaleBarValidityCheck::id() const
{
  return QStringLiteral( "layout_scalebar_check" );
}

int QgsLayoutScaleBarValidityCheck::checkType() const
{
  return QgsAbstractValidityCheck::TypeLayoutCheck;
}

bool QgsLayoutScaleBarValidityCheck::prepareCheck( const QgsValidityCheckContext *context, QgsFeedback * )
{
  if ( context->type() != QgsValidityCheckContext::TypeLayoutContext )
    return false;

  const QgsLayoutValidityCheckContext *layoutContext = static_cast< const QgsLayoutValidityCheckContext * >( context );
  if ( !layoutContext )
    return false;

  QList< QgsLayoutItemScaleBar * > barItems;
  layoutContext->layout->layoutItems( barItems );
  for ( QgsLayoutItemScaleBar *bar : qgis::as_const( barItems ) )
  {
    if ( !bar->linkedMap() )
    {
      QgsValidityCheckResult res;
      res.type = QgsValidityCheckResult::Warning;
      res.title = tr( "Scalebar is not linked to a map" );
      const QString name = bar->displayName().toHtmlEscaped();
      res.detailedDescription = tr( "The scalebar “%1” is not linked to a map item. This scale will be misleading." ).arg( name );
      mResults.append( res );
    }
  }

  return true;
}

QList<QgsValidityCheckResult> QgsLayoutScaleBarValidityCheck::runCheck( const QgsValidityCheckContext *, QgsFeedback * )
{
  return mResults;
}
