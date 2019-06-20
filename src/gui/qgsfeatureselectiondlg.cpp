/***************************************************************************
    qgsfeatureselectiondlg.cpp
     --------------------------------------
    Date                 : 11.6.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfeatureselectiondlg.h"

#include "qgsgenericfeatureselectionmanager.h"
#include "qgsdistancearea.h"
#include "qgsfeaturerequest.h"
#include "qgsattributeeditorcontext.h"
#include "qgsapplication.h"

#include <QWindow>

QgsFeatureSelectionDlg::QgsFeatureSelectionDlg( QgsVectorLayer *vl, QgsAttributeEditorContext &context, QWidget *parent )
  : QDialog( parent )
  , mVectorLayer( vl )
{
  setupUi( this );

  mFeatureSelection = new QgsGenericFeatureSelectionManager( mDualView );

  mDualView->setFeatureSelectionManager( mFeatureSelection );

  // TODO: Proper QgsDistanceArea, proper mapcanvas
  mDualView->init( mVectorLayer, nullptr, QgsFeatureRequest(), context );
}

const QgsFeatureIds &QgsFeatureSelectionDlg::selectedFeatures()
{
  return mFeatureSelection->selectedFeatureIds();
}

void QgsFeatureSelectionDlg::setSelectedFeatures( const QgsFeatureIds &ids )
{
  mFeatureSelection->setSelectedFeatures( ids );
}

void QgsFeatureSelectionDlg::showEvent( QShowEvent *event )
{

  QWindow *mainWindow = nullptr;
  for ( const auto &w : QgsApplication::instance()->topLevelWindows() )
  {
    if ( w->objectName() == QStringLiteral( "QgisAppWindow" ) )
    {
      mainWindow = w;
      break;
    }
  }

  if ( mainWindow )
  {
    QSize margins( size() - scrollAreaWidgetContents->size() );
    QSize innerWinSize( mainWindow->width(), mainWindow->height() );
    setMaximumSize( innerWinSize );
    QSize minSize( scrollAreaWidgetContents->sizeHint() );
    setMinimumSize( std::min( minSize.width() + margins.width( ), innerWinSize.width() ),
                    std::min( minSize.height() + margins.width( ), innerWinSize.height() ) );
  }

  QDialog::showEvent( event );
}


