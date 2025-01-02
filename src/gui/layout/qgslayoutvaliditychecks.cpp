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
#ifndef WITH_QTWEBKIT
#include "qgslayoutmultiframe.h"
#endif
#include "qgslayout.h"
#include "qgssettings.h"
#include <QUrl>

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
  return static_cast<int>( QgsAbstractValidityCheck::Type::LayoutCheck );
}

bool QgsLayoutScaleBarValidityCheck::prepareCheck( const QgsValidityCheckContext *context, QgsFeedback * )
{
  if ( context->type() != QgsValidityCheckContext::TypeLayoutContext )
    return false;

  const QgsLayoutValidityCheckContext *layoutContext = static_cast<const QgsLayoutValidityCheckContext *>( context );
  if ( !layoutContext )
    return false;

  QList<QgsLayoutItemScaleBar *> barItems;
  layoutContext->layout->layoutItems( barItems );
  for ( QgsLayoutItemScaleBar *bar : std::as_const( barItems ) )
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
// QgsLayoutNorthArrowValidityCheck
//

QgsLayoutNorthArrowValidityCheck *QgsLayoutNorthArrowValidityCheck::create() const
{
  return new QgsLayoutNorthArrowValidityCheck();
}

QString QgsLayoutNorthArrowValidityCheck::id() const
{
  return QStringLiteral( "layout_northarrow_check" );
}

int QgsLayoutNorthArrowValidityCheck::checkType() const
{
  return static_cast<int>( QgsAbstractValidityCheck::Type::LayoutCheck );
}

bool QgsLayoutNorthArrowValidityCheck::prepareCheck( const QgsValidityCheckContext *context, QgsFeedback * )
{
  if ( context->type() != QgsValidityCheckContext::TypeLayoutContext )
    return false;

  const QgsLayoutValidityCheckContext *layoutContext = static_cast<const QgsLayoutValidityCheckContext *>( context );
  if ( !layoutContext )
    return false;

  QgsSettings settings;
  const QString defaultPath = settings.value( QStringLiteral( "LayoutDesigner/defaultNorthArrow" ), QStringLiteral( ":/images/north_arrows/layout_default_north_arrow.svg" ), QgsSettings::Gui ).toString();

  QList<QgsLayoutItemPicture *> pictureItems;
  layoutContext->layout->layoutItems( pictureItems );
  for ( QgsLayoutItemPicture *picture : std::as_const( pictureItems ) )
  {
    // look for pictures which use the default north arrow svg, but aren't actually linked to maps.
    // alternatively identify them by looking for the default "North Arrow" string in their id
    if ( !picture->linkedMap() && ( picture->picturePath() == defaultPath || picture->id().contains( QObject::tr( "North Arrow" ), Qt::CaseInsensitive ) ) )
    {
      QgsValidityCheckResult res;
      res.type = QgsValidityCheckResult::Warning;
      res.title = QObject::tr( "North arrow is not linked to a map" );
      const QString name = picture->displayName().toHtmlEscaped();
      res.detailedDescription = QObject::tr( "The north arrow “%1” is not linked to a map item. The arrow orientation may be misleading." ).arg( name );
      mResults.append( res );
    }
  }

  return true;
}

QList<QgsValidityCheckResult> QgsLayoutNorthArrowValidityCheck::runCheck( const QgsValidityCheckContext *, QgsFeedback * )
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
  return static_cast<int>( QgsAbstractValidityCheck::Type::LayoutCheck );
}

bool QgsLayoutOverviewValidityCheck::prepareCheck( const QgsValidityCheckContext *context, QgsFeedback * )
{
  if ( context->type() != QgsValidityCheckContext::TypeLayoutContext )
    return false;

  const QgsLayoutValidityCheckContext *layoutContext = static_cast<const QgsLayoutValidityCheckContext *>( context );
  if ( !layoutContext )
    return false;

  QList<QgsLayoutItemMap *> mapItems;
  layoutContext->layout->layoutItems( mapItems );
  for ( QgsLayoutItemMap *map : std::as_const( mapItems ) )
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
  return static_cast<int>( QgsAbstractValidityCheck::Type::LayoutCheck );
}

bool QgsLayoutPictureSourceValidityCheck::prepareCheck( const QgsValidityCheckContext *context, QgsFeedback * )
{
  if ( context->type() != QgsValidityCheckContext::TypeLayoutContext )
    return false;

  const QgsLayoutValidityCheckContext *layoutContext = static_cast<const QgsLayoutValidityCheckContext *>( context );
  if ( !layoutContext )
    return false;

  QList<QgsLayoutItemPicture *> pictureItems;
  layoutContext->layout->layoutItems( pictureItems );
  for ( QgsLayoutItemPicture *picture : std::as_const( pictureItems ) )
  {
    if ( picture->isMissingImage() )
    {
      QgsValidityCheckResult res;
      res.type = QgsValidityCheckResult::Warning;
      res.title = QObject::tr( "Picture source is missing or corrupt" );
      const QString name = picture->displayName().toHtmlEscaped();

      const QUrl picUrl = QUrl::fromUserInput( picture->evaluatedPath() );
      const bool isLocalFile = picUrl.isLocalFile();

      res.detailedDescription = QObject::tr( "The source for picture “%1” could not be loaded or is corrupt:<p>%2" ).arg( name, isLocalFile ? QDir::toNativeSeparators( picture->evaluatedPath() ) : picture->evaluatedPath() );
      mResults.append( res );
    }
  }

  return true;
}

QList<QgsValidityCheckResult> QgsLayoutPictureSourceValidityCheck::runCheck( const QgsValidityCheckContext *, QgsFeedback * )
{
  return mResults;
}

#ifndef WITH_QTWEBKIT
//
// QgsLayoutHtmlItemValidityCheck
//

QgsLayoutHtmlItemValidityCheck *QgsLayoutHtmlItemValidityCheck::create() const
{
  return new QgsLayoutHtmlItemValidityCheck();
}

QString QgsLayoutHtmlItemValidityCheck::id() const
{
  return QStringLiteral( "layout_html_item_check" );
}

int QgsLayoutHtmlItemValidityCheck::checkType() const
{
  return static_cast<int>( QgsAbstractValidityCheck::Type::LayoutCheck );
}

bool QgsLayoutHtmlItemValidityCheck::prepareCheck( const QgsValidityCheckContext *context, QgsFeedback * )
{
  if ( context->type() != QgsValidityCheckContext::TypeLayoutContext )
    return false;

  const QgsLayoutValidityCheckContext *layoutContext = static_cast<const QgsLayoutValidityCheckContext *>( context );
  if ( !layoutContext )
    return false;

  const QList<QgsLayoutMultiFrame *> multiFrames = layoutContext->layout->multiFrames();
  for ( QgsLayoutMultiFrame *multiFrame : std::as_const( multiFrames ) )
  {
    if ( multiFrame->type() == QgsLayoutItemRegistry::LayoutHtml && multiFrame->frameCount() > 0 )
    {
      QgsValidityCheckResult res;
      res.type = QgsValidityCheckResult::Warning;
      res.title = QObject::tr( "HTML item cannot be rendered" );
      res.detailedDescription = QObject::tr( "HTML items cannot be rendered because this QGIS install was built without WebKit support. These items will be missing from the export." );
      mResults.append( res );
      break;
    }
  }

  return true;
}

QList<QgsValidityCheckResult> QgsLayoutHtmlItemValidityCheck::runCheck( const QgsValidityCheckContext *, QgsFeedback * )
{
  return mResults;
}
#endif
