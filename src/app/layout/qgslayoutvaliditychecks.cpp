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
#include "qgslayoutitemmap.h"
#include "qgslayoutitempicture.h"
#include "qgslayout.h"

//
// QgsLayoutScaleBarValidityCheck
//

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
      res.title = QObject::tr( "Scalebar is not linked to a map" );
      const QString name = bar->displayName().toHtmlEscaped();
      res.detailedDescription = QObject::tr( "The scalebar “%1” is not linked to a map item. This scale will be misleading." ).arg( name );
      mResults.append( res );
    }
  }

  return true;
}

QList<QgsValidityCheckResult> QgsLayoutScaleBarValidityCheck::runCheck( const QgsValidityCheckContext *, QgsFeedback * )
{
  return mResults;
}


//
// QgsLayoutOverviewValidityCheck
//

QgsLayoutOverviewValidityCheck *QgsLayoutOverviewValidityCheck::create() const
{
  return new QgsLayoutOverviewValidityCheck();
}

QString QgsLayoutOverviewValidityCheck::id() const
{
  return QStringLiteral( "layout_overview_check" );
}

int QgsLayoutOverviewValidityCheck::checkType() const
{
  return QgsAbstractValidityCheck::TypeLayoutCheck;
}

bool QgsLayoutOverviewValidityCheck::prepareCheck( const QgsValidityCheckContext *context, QgsFeedback * )
{
  if ( context->type() != QgsValidityCheckContext::TypeLayoutContext )
    return false;

  const QgsLayoutValidityCheckContext *layoutContext = static_cast< const QgsLayoutValidityCheckContext * >( context );
  if ( !layoutContext )
    return false;

  QList< QgsLayoutItemMap * > mapItems;
  layoutContext->layout->layoutItems( mapItems );
  for ( QgsLayoutItemMap *map : qgis::as_const( mapItems ) )
  {
    for ( int i = 0; i < map->overviews()->size(); ++i )
    {
      QgsLayoutItemMapOverview *overview = map->overviews()->overview( i );
      if ( overview && overview->enabled() && !overview->linkedMap() )
      {
        QgsValidityCheckResult res;
        res.type = QgsValidityCheckResult::Warning;
        res.title = QObject::tr( "Overview is not linked to a map" );
        const QString name = map->displayName().toHtmlEscaped();
        res.detailedDescription = QObject::tr( "The map “%1” includes an overview (“%2”) which is not linked to a map item." ).arg( name, overview->name() );
        mResults.append( res );
      }
    }
  }

  return true;
}

QList<QgsValidityCheckResult> QgsLayoutOverviewValidityCheck::runCheck( const QgsValidityCheckContext *, QgsFeedback * )
{
  return mResults;
}



//
// QgsLayoutPictureSourceValidityCheck
//

QgsLayoutPictureSourceValidityCheck *QgsLayoutPictureSourceValidityCheck::create() const
{
  return new QgsLayoutPictureSourceValidityCheck();
}

QString QgsLayoutPictureSourceValidityCheck::id() const
{
  return QStringLiteral( "layout_picture_source_check" );
}

int QgsLayoutPictureSourceValidityCheck::checkType() const
{
  return QgsAbstractValidityCheck::TypeLayoutCheck;
}

bool QgsLayoutPictureSourceValidityCheck::prepareCheck( const QgsValidityCheckContext *context, QgsFeedback * )
{
  if ( context->type() != QgsValidityCheckContext::TypeLayoutContext )
    return false;

  const QgsLayoutValidityCheckContext *layoutContext = static_cast< const QgsLayoutValidityCheckContext * >( context );
  if ( !layoutContext )
    return false;

  QList< QgsLayoutItemPicture * > pictureItems;
  layoutContext->layout->layoutItems( pictureItems );
  for ( QgsLayoutItemPicture *picture : qgis::as_const( pictureItems ) )
  {
    if ( picture->isMissingImage() )
    {
      QgsValidityCheckResult res;
      res.type = QgsValidityCheckResult::Warning;
      res.title = QObject::tr( "Picture source is missing or corrupt" );
      const QString name = picture->displayName().toHtmlEscaped();

      const QUrl picUrl = QUrl::fromUserInput( picture->evaluatedPath() );
      const bool isLocalFile = picUrl.isLocalFile();

      res.detailedDescription = QObject::tr( "The source for picture “%1” could not be loaded or is corrupt:<p>%2" ).arg( name,
                                isLocalFile ? QDir::toNativeSeparators( picture->evaluatedPath() ) : picture->evaluatedPath() );
      mResults.append( res );
    }
  }

  return true;
}

QList<QgsValidityCheckResult> QgsLayoutPictureSourceValidityCheck::runCheck( const QgsValidityCheckContext *, QgsFeedback * )
{
  return mResults;
}
