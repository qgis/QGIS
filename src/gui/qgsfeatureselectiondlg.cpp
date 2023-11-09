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

#include "qgsvectorlayerselectionmanager.h"
#include "qgsdistancearea.h"
#include "qgsfeaturerequest.h"
#include "qgsattributeeditorcontext.h"
#include "qgsapplication.h"
#include "qgsexpressionselectiondialog.h"
#include "qgsmapcanvas.h"

#include <QWindow>

QgsFeatureSelectionDlg::QgsFeatureSelectionDlg( QgsVectorLayer *vl, const QgsAttributeEditorContext &context, QWidget *parent )
  : QDialog( parent, Qt::Window )
  , mVectorLayer( vl )
  , mContext( context )
{
  setupUi( this );

  mFeatureSelection = new QgsVectorLayerSelectionManager( vl, mDualView );

  mDualView->setFeatureSelectionManager( mFeatureSelection );

  mDualView->init( mVectorLayer, context.mapCanvas(), QgsFeatureRequest(), context );
  mDualView->setView( QgsDualView::AttributeEditor );

  mFeatureFilterWidget->init( vl, context, mDualView, mMessageBar, 5 );
  mFeatureFilterWidget->filterVisible();

  connect( mActionExpressionSelect, &QAction::triggered, this, &QgsFeatureSelectionDlg::mActionExpressionSelect_triggered );
  connect( mActionSelectAll, &QAction::triggered, this, &QgsFeatureSelectionDlg::mActionSelectAll_triggered );
  connect( mActionInvertSelection, &QAction::triggered, this, &QgsFeatureSelectionDlg::mActionInvertSelection_triggered );
  connect( mActionRemoveSelection, &QAction::triggered, this, &QgsFeatureSelectionDlg::mActionRemoveSelection_triggered );
  connect( mActionSearchForm, &QAction::toggled, mDualView, &QgsDualView::toggleSearchMode );
  connect( mActionSelectedToTop, &QAction::toggled, this, [this]( bool checked ) { mDualView->setSelectedOnTop( checked ); } );
  connect( mActionZoomMapToSelectedRows, &QAction::triggered, this, &QgsFeatureSelectionDlg::mActionZoomMapToSelectedRows_triggered );
  connect( mActionPanMapToSelectedRows, &QAction::triggered, this, &QgsFeatureSelectionDlg::mActionPanMapToSelectedRows_triggered );

  connect( mDualView, &QgsDualView::filterExpressionSet, this, &QgsFeatureSelectionDlg::setFilterExpression );
  connect( mDualView, &QgsDualView::formModeChanged, this, &QgsFeatureSelectionDlg::viewModeChanged );
}

void QgsFeatureSelectionDlg::keyPressEvent( QKeyEvent *evt )
{
  // don't accept Enter key to accept the dialog, user could be editing a text field
  if ( evt->key() == Qt::Key_Enter || evt->key() == Qt::Key_Return )
    return;
  QDialog::keyPressEvent( evt );
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
  for ( const auto &w : QgsApplication::topLevelWindows() )
  {
    if ( w->objectName() == QLatin1String( "QgisAppWindow" ) )
    {
      mainWindow = w;
      break;
    }
  }

  if ( mainWindow )
  {
    const QSize margins( size() - scrollAreaWidgetContents->size() );
    const QSize innerWinSize( mainWindow->width(), mainWindow->height() );
    setMaximumSize( innerWinSize );
    const QSize minSize( scrollAreaWidgetContents->sizeHint() );
    setMinimumSize( std::min( minSize.width() + margins.width( ), innerWinSize.width() ),
                    std::min( minSize.height() + margins.width( ), innerWinSize.height() ) );
  }

  QDialog::showEvent( event );
}

void QgsFeatureSelectionDlg::mActionExpressionSelect_triggered()
{
  QgsExpressionSelectionDialog *dlg = new QgsExpressionSelectionDialog( mVectorLayer );
  dlg->setMessageBar( mMessageBar );
  dlg->setAttribute( Qt::WA_DeleteOnClose );
  dlg->show();
}

void QgsFeatureSelectionDlg::mActionInvertSelection_triggered()
{
  mVectorLayer->invertSelection();
}

void QgsFeatureSelectionDlg::mActionRemoveSelection_triggered()
{
  mVectorLayer->removeSelection();
}

void QgsFeatureSelectionDlg::mActionSelectAll_triggered()
{
  mVectorLayer->selectAll();
}

void QgsFeatureSelectionDlg::mActionZoomMapToSelectedRows_triggered()
{
  mContext.mapCanvas()->zoomToSelected( mVectorLayer );
}

void QgsFeatureSelectionDlg::mActionPanMapToSelectedRows_triggered()
{
  mContext.mapCanvas()->panToSelected( mVectorLayer );
}

void QgsFeatureSelectionDlg::setFilterExpression( const QString &filter, QgsAttributeForm::FilterType type )
{
  mFeatureFilterWidget->setFilterExpression( filter, type, true );
}

void QgsFeatureSelectionDlg::viewModeChanged( QgsAttributeEditorContext::Mode mode )
{
  mActionSearchForm->setChecked( mode == QgsAttributeEditorContext::SearchMode );
}
