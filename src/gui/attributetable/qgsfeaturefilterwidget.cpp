/***************************************************************************
    qgsfeaturefilterwidget.cpp
     --------------------------------------
    Date                 : 20.9.2019
    Copyright            : (C) 2019 Julien Cabieces
    Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "qgsfeaturefilterwidget_p.h"

#include "qgsapplication.h"
#include "qgssearchwidgetwrapper.h"
#include "qgsdualview.h"
#include "qgsstoredexpressionmanager.h"
#include "qgseditorwidgetregistry.h"
#include "qgsexpressioncontextutils.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsgui.h"
#include "qgsdialog.h"
#include "qgsexpressionlineedit.h"
#include "qgsmessagebar.h"

#include <QMenu>

QgsFeatureFilterWidget::QgsFeatureFilterWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  // Initialize filter gui elements
  mFilterColumnsMenu = new QMenu( this );
  mActionFilterColumnsMenu->setMenu( mFilterColumnsMenu );
  mStoredFilterExpressionMenu = new QMenu( this );
  mActionStoredFilterExpressions->setMenu( mStoredFilterExpressionMenu );

  // Set filter icon in a couple of places
  QIcon filterIcon = QgsApplication::getThemeIcon( "/mActionFilter2.svg" );
  mActionShowAllFilter->setIcon( filterIcon );
  mActionAdvancedFilter->setIcon( filterIcon );
  mActionSelectedFilter->setIcon( filterIcon );
  mActionVisibleFilter->setIcon( filterIcon );
  mActionEditedFilter->setIcon( filterIcon );


  // Set button to store or delete stored filter expressions
  mStoreFilterExpressionButton->setDefaultAction( mActionHandleStoreFilterExpression );
  connect( mActionSaveAsStoredFilterExpression, &QAction::triggered, this, &QgsFeatureFilterWidget::saveAsStoredFilterExpression );
  connect( mActionEditStoredFilterExpression, &QAction::triggered, this, &QgsFeatureFilterWidget::editStoredFilterExpression );
  connect( mActionHandleStoreFilterExpression, &QAction::triggered, this, &QgsFeatureFilterWidget::handleStoreFilterExpression );
  mApplyFilterButton->setDefaultAction( mActionApplyFilter );

  // Connect filter signals
  connect( mActionAdvancedFilter, &QAction::triggered, this, &QgsFeatureFilterWidget::filterExpressionBuilder );
  connect( mActionShowAllFilter, &QAction::triggered, this, &QgsFeatureFilterWidget::filterShowAll );
  connect( mActionSelectedFilter, &QAction::triggered, this, &QgsFeatureFilterWidget::filterSelected );
  connect( mActionVisibleFilter, &QAction::triggered, this, &QgsFeatureFilterWidget::filterVisible );
  connect( mActionEditedFilter, &QAction::triggered, this, &QgsFeatureFilterWidget::filterEdited );
  connect( mFilterQuery, &QLineEdit::returnPressed, this, &QgsFeatureFilterWidget::filterQueryAccepted );
  connect( mActionApplyFilter, &QAction::triggered, this, &QgsFeatureFilterWidget::filterQueryAccepted );
  connect( mFilterQuery, &QLineEdit::textChanged, this, &QgsFeatureFilterWidget::onFilterQueryTextChanged );
}

void QgsFeatureFilterWidget::init( QgsVectorLayer *layer, const QgsAttributeEditorContext &context, QgsDualView *mainView,
                                   QgsMessageBar *messageBar, int messageBarTimeout )
{
  mMainView = mainView;
  mLayer = layer;
  mEditorContext = context;
  mMessageBar = messageBar;
  mMessageBarTimeout = messageBarTimeout;

  connect( mLayer, &QgsVectorLayer::attributeAdded, this, &QgsFeatureFilterWidget::columnBoxInit );
  connect( mLayer, &QgsVectorLayer::attributeDeleted, this, &QgsFeatureFilterWidget::columnBoxInit );

  //set delay on entering text
  mFilterQueryTimer.setSingleShot( true );
  connect( &mFilterQueryTimer, &QTimer::timeout, this, &QgsFeatureFilterWidget::updateCurrentStoredFilterExpression );

  columnBoxInit();
  storedFilterExpressionBoxInit();
  storeExpressionButtonInit();
}

void QgsFeatureFilterWidget::filterShowAll()
{
  mFilterButton->setDefaultAction( mActionShowAllFilter );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  mFilterQuery->setVisible( false );
  mFilterQuery->setText( QString() );
  if ( mCurrentSearchWidgetWrapper )
  {
    mCurrentSearchWidgetWrapper->widget()->setVisible( false );
  }
  mApplyFilterButton->setVisible( false );
  mStoreFilterExpressionButton->setVisible( false );
  mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowAll );
}

void QgsFeatureFilterWidget::filterSelected()
{
  mFilterButton->setDefaultAction( mActionSelectedFilter );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  mFilterQuery->setVisible( false );
  mApplyFilterButton->setVisible( false );
  mStoreFilterExpressionButton->setVisible( false );
  mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowSelected );
}

void QgsFeatureFilterWidget::filterVisible()
{
  if ( !mLayer->isSpatial() )
  {
    filterShowAll();
    return;
  }

  mFilterButton->setDefaultAction( mActionVisibleFilter );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  mFilterQuery->setVisible( false );
  mApplyFilterButton->setVisible( false );
  mStoreFilterExpressionButton->setVisible( false );
  mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowVisible );
}

void QgsFeatureFilterWidget::filterEdited()
{
  mFilterButton->setDefaultAction( mActionEditedFilter );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  mFilterQuery->setVisible( false );
  mApplyFilterButton->setVisible( false );
  mStoreFilterExpressionButton->setVisible( false );
  mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowEdited );
}


void QgsFeatureFilterWidget::filterQueryAccepted()
{
  if ( ( mFilterQuery->isVisible() && mFilterQuery->text().isEmpty() ) ||
       ( mCurrentSearchWidgetWrapper && mCurrentSearchWidgetWrapper->widget()->isVisible()
         && mCurrentSearchWidgetWrapper->expression().isEmpty() ) )
  {
    filterShowAll();
    return;
  }
  filterQueryChanged( mFilterQuery->text() );
}

void QgsFeatureFilterWidget::filterQueryChanged( const QString &query )
{
  QString str;
  if ( mFilterButton->defaultAction() == mActionAdvancedFilter )
  {
    str = query;
  }
  else if ( mCurrentSearchWidgetWrapper )
  {
    str = mCurrentSearchWidgetWrapper->expression();
  }

  setFilterExpression( str );
}

void QgsFeatureFilterWidget::columnBoxInit()
{
  const auto constActions = mFilterColumnsMenu->actions();
  for ( QAction *a : constActions )
  {
    mFilterColumnsMenu->removeAction( a );
    mFilterButton->removeAction( a );
    delete a;
  }

  mFilterButton->addAction( mActionShowAllFilter );
  mFilterButton->addAction( mActionSelectedFilter );
  if ( mLayer->isSpatial() )
  {
    mFilterButton->addAction( mActionVisibleFilter );
  }
  mFilterButton->addAction( mActionEditedFilter );
  mFilterButton->addAction( mActionFilterColumnsMenu );
  mFilterButton->addAction( mActionAdvancedFilter );
  mFilterButton->addAction( mActionStoredFilterExpressions );

  const QList<QgsField> fields = mLayer->fields().toList();

  const auto constFields = fields;
  for ( const QgsField &field : constFields )
  {
    int idx = mLayer->fields().lookupField( field.name() );
    if ( idx < 0 )
      continue;

    if ( QgsGui::editorWidgetRegistry()->findBest( mLayer, field.name() ).type() != QLatin1String( "Hidden" ) )
    {
      QIcon icon = mLayer->fields().iconForField( idx );
      QString alias = mLayer->attributeDisplayName( idx );

      // Generate action for the filter popup button
      QAction *filterAction = new QAction( icon, alias, mFilterButton );
      filterAction->setData( field.name() );

      connect( filterAction, &QAction::triggered, this, [ = ] { filterColumnChanged( filterAction ); } );
      mFilterColumnsMenu->addAction( filterAction );
    }
  }
}

void QgsFeatureFilterWidget::handleStoreFilterExpression()
{
  if ( !mActionHandleStoreFilterExpression->isChecked() )
  {
    mLayer->storedExpressionManager()->removeStoredExpression( mActionHandleStoreFilterExpression->data().toString() );
  }
  else
  {
    mLayer->storedExpressionManager()->addStoredExpression( mFilterQuery->text(), mFilterQuery->text() );
  }

  updateCurrentStoredFilterExpression();
  storedFilterExpressionBoxInit();
}

void QgsFeatureFilterWidget::storedFilterExpressionBoxInit()
{
  const auto constActions = mStoredFilterExpressionMenu->actions();
  for ( QAction *a : constActions )
  {
    mStoredFilterExpressionMenu->removeAction( a );
    delete a;
  }

  const QList< QgsStoredExpression > storedExpressions = mLayer->storedExpressionManager()->storedExpressions();
  for ( const QgsStoredExpression &storedExpression : storedExpressions )
  {
    QAction *storedExpressionAction = new QAction( storedExpression.name, mFilterButton );
    connect( storedExpressionAction, &QAction::triggered, this, [ = ]()
    {
      setFilterExpression( storedExpression.expression, QgsAttributeForm::ReplaceFilter, true );
    } );
    mStoredFilterExpressionMenu->addAction( storedExpressionAction );
  }
}

void QgsFeatureFilterWidget::storeExpressionButtonInit()
{
  if ( mActionHandleStoreFilterExpression->isChecked() )
  {
    mActionHandleStoreFilterExpression->setToolTip( tr( "Delete stored expression" ) );
    mActionHandleStoreFilterExpression->setText( tr( "Delete Stored Expression" ) );
    mActionHandleStoreFilterExpression->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionHandleStoreFilterExpressionChecked.svg" ) ) );
    mStoreFilterExpressionButton->removeAction( mActionSaveAsStoredFilterExpression );
    mStoreFilterExpressionButton->addAction( mActionEditStoredFilterExpression );
  }
  else
  {
    mActionHandleStoreFilterExpression->setToolTip( tr( "Save expression with the text as name" ) );
    mActionHandleStoreFilterExpression->setText( tr( "Save Expression" ) );
    mActionHandleStoreFilterExpression->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionHandleStoreFilterExpressionUnchecked.svg" ) ) );
    mStoreFilterExpressionButton->addAction( mActionSaveAsStoredFilterExpression );
    mStoreFilterExpressionButton->removeAction( mActionEditStoredFilterExpression );
  }
}


void QgsFeatureFilterWidget::filterColumnChanged( QAction *filterAction )
{
  mFilterButton->setDefaultAction( filterAction );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  // replace the search line edit with a search widget that is suited to the selected field
  // delete previous widget
  if ( mCurrentSearchWidgetWrapper )
  {
    mCurrentSearchWidgetWrapper->widget()->setVisible( false );
    delete mCurrentSearchWidgetWrapper;
  }
  QString fieldName = mFilterButton->defaultAction()->data().toString();
  // get the search widget
  int fldIdx = mLayer->fields().lookupField( fieldName );
  if ( fldIdx < 0 )
    return;
  const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( mLayer, fieldName );
  mCurrentSearchWidgetWrapper = QgsGui::editorWidgetRegistry()->
                                createSearchWidget( setup.type(), mLayer, fldIdx, setup.config(), mFilterContainer, mEditorContext );
  if ( mCurrentSearchWidgetWrapper->applyDirectly() )
  {
    connect( mCurrentSearchWidgetWrapper, &QgsSearchWidgetWrapper::expressionChanged, this, &QgsFeatureFilterWidget::filterQueryChanged );
    mApplyFilterButton->setVisible( false );
    mStoreFilterExpressionButton->setVisible( false );
  }
  else
  {
    connect( mCurrentSearchWidgetWrapper, &QgsSearchWidgetWrapper::expressionChanged, this, &QgsFeatureFilterWidget::filterQueryAccepted );
    mApplyFilterButton->setVisible( true );
    mStoreFilterExpressionButton->setVisible( true );
  }

  replaceSearchWidget( mFilterQuery, mCurrentSearchWidgetWrapper->widget() );
}

void QgsFeatureFilterWidget::filterExpressionBuilder()
{
  // Show expression builder
  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );

  QgsExpressionBuilderDialog dlg( mLayer, mFilterQuery->text(), this, QStringLiteral( "generic" ), context );
  dlg.setWindowTitle( tr( "Expression Based Filter" ) );

  QgsDistanceArea myDa;
  myDa.setSourceCrs( mLayer->crs(), QgsProject::instance()->transformContext() );
  myDa.setEllipsoid( QgsProject::instance()->ellipsoid() );
  dlg.setGeomCalculator( myDa );

  if ( dlg.exec() == QDialog::Accepted )
  {
    setFilterExpression( dlg.expressionText(), QgsAttributeForm::ReplaceFilter, true );
  }
}

void QgsFeatureFilterWidget::saveAsStoredFilterExpression()
{
  QgsDialog *dlg = new QgsDialog( this, nullptr, QDialogButtonBox::Save | QDialogButtonBox::Cancel );
  dlg->setWindowTitle( tr( "Save Expression As" ) );
  QVBoxLayout *layout = dlg->layout();
  dlg->resize( std::max( 400, this->width() / 2 ), dlg->height() );

  QLabel *nameLabel = new QLabel( tr( "Name" ), dlg );
  QLineEdit *nameEdit = new QLineEdit( dlg );
  layout->addWidget( nameLabel );
  layout->addWidget( nameEdit );

  if ( dlg->exec() == QDialog::Accepted )
  {
    mLayer->storedExpressionManager()->addStoredExpression( nameEdit->text(), mFilterQuery->text() );

    updateCurrentStoredFilterExpression();
    storedFilterExpressionBoxInit();
  }
}

void QgsFeatureFilterWidget::editStoredFilterExpression()
{
  QgsDialog *dlg = new QgsDialog( this, nullptr, QDialogButtonBox::Save | QDialogButtonBox::Cancel );
  dlg->setWindowTitle( tr( "Edit expression" ) );
  QVBoxLayout *layout = dlg->layout();
  dlg->resize( std::max( 400, this->width() / 2 ), dlg->height() );

  QLabel *nameLabel = new QLabel( tr( "Name" ), dlg );
  QLineEdit *nameEdit = new QLineEdit( mLayer->storedExpressionManager()->storedExpression( mActionHandleStoreFilterExpression->data().toString() ).name, dlg );
  QLabel *expressionLabel = new QLabel( tr( "Expression" ), dlg );
  QgsExpressionLineEdit *expressionEdit = new QgsExpressionLineEdit( dlg );
  expressionEdit->setExpression( mLayer->storedExpressionManager()->storedExpression( mActionHandleStoreFilterExpression->data().toString() ).expression );

  layout->addWidget( nameLabel );
  layout->addWidget( nameEdit );
  layout->addWidget( expressionLabel );
  layout->addWidget( expressionEdit );

  if ( dlg->exec() == QDialog::Accepted )
  {
    //update stored expression
    mLayer->storedExpressionManager()->updateStoredExpression( mActionHandleStoreFilterExpression->data().toString(), nameEdit->text(), expressionEdit->expression(), QgsStoredExpression::Category::FilterExpression );

    //update text
    mFilterQuery->setValue( expressionEdit->expression() );

    storedFilterExpressionBoxInit();
  }
}

void QgsFeatureFilterWidget::updateCurrentStoredFilterExpression()
{
  QgsStoredExpression currentStoredExpression = mLayer->storedExpressionManager()->findStoredExpressionByExpression( mFilterQuery->value() );

  //set checked when it's an existing stored expression
  mActionHandleStoreFilterExpression->setChecked( !currentStoredExpression.id.isNull() );

  mActionHandleStoreFilterExpression->setData( currentStoredExpression.id );
  mActionEditStoredFilterExpression->setData( currentStoredExpression.id );

  //update bookmark button
  storeExpressionButtonInit();
}

void QgsFeatureFilterWidget::setFilterExpression( const QString &filterString, QgsAttributeForm::FilterType type, bool alwaysShowFilter )
{
  QString filter;
  if ( !mFilterQuery->text().isEmpty() && !filterString.isEmpty() )
  {
    switch ( type )
    {
      case QgsAttributeForm::ReplaceFilter:
        filter = filterString;
        break;

      case QgsAttributeForm::FilterAnd:
        filter = QStringLiteral( "(%1) AND (%2)" ).arg( mFilterQuery->text(), filterString );
        break;

      case QgsAttributeForm::FilterOr:
        filter = QStringLiteral( "(%1) OR (%2)" ).arg( mFilterQuery->text(), filterString );
        break;
    }
  }
  else if ( !filterString.isEmpty() )
  {
    filter = filterString;
  }
  else
  {
    filterShowAll();
    return;
  }

  mFilterQuery->setText( filter );

  if ( alwaysShowFilter || !mCurrentSearchWidgetWrapper || !mCurrentSearchWidgetWrapper->applyDirectly() )
  {

    mFilterButton->setDefaultAction( mActionAdvancedFilter );
    mFilterButton->setPopupMode( QToolButton::MenuButtonPopup );
    mFilterQuery->setVisible( true );
    mApplyFilterButton->setVisible( true );
    mStoreFilterExpressionButton->setVisible( true );
    if ( mCurrentSearchWidgetWrapper )
    {
      // replace search widget widget with the normal filter query line edit
      replaceSearchWidget( mCurrentSearchWidgetWrapper->widget(), mFilterQuery );
    }
  }

  QgsFeatureIds filteredFeatures;
  QgsDistanceArea myDa;

  myDa.setSourceCrs( mLayer->crs(), QgsProject::instance()->transformContext() );
  myDa.setEllipsoid( QgsProject::instance()->ellipsoid() );

  // parse search string and build parsed tree
  QgsExpression filterExpression( filter );
  if ( filterExpression.hasParserError() )
  {
    mMessageBar->pushMessage( tr( "Parsing error" ), filterExpression.parserErrorString(), Qgis::Warning, mMessageBarTimeout );
    return;
  }

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );

  if ( !filterExpression.prepare( &context ) )
  {
    mMessageBar->pushMessage( tr( "Evaluation error" ), filterExpression.evalErrorString(), Qgis::Warning, mMessageBarTimeout );
  }

  bool fetchGeom = filterExpression.needsGeometry();

  QApplication::setOverrideCursor( Qt::WaitCursor );

  filterExpression.setGeomCalculator( &myDa );
  filterExpression.setDistanceUnits( QgsProject::instance()->distanceUnits() );
  filterExpression.setAreaUnits( QgsProject::instance()->areaUnits() );
  QgsFeatureRequest request( mMainView->masterModel()->request() );
  request.setSubsetOfAttributes( filterExpression.referencedColumns(), mLayer->fields() );
  if ( !fetchGeom )
  {
    request.setFlags( QgsFeatureRequest::NoGeometry );
  }
  else
  {
    // force geometry extraction if the filter requests it
    request.setFlags( request.flags() & ~QgsFeatureRequest::NoGeometry );
  }
  QgsFeatureIterator featIt = mLayer->getFeatures( request );

  QgsFeature f;

  while ( featIt.nextFeature( f ) )
  {
    context.setFeature( f );
    if ( filterExpression.evaluate( &context ).toInt() != 0 )
      filteredFeatures << f.id();

    // check if there were errors during evaluating
    if ( filterExpression.hasEvalError() )
      break;
  }

  featIt.close();

  mMainView->setFilteredFeatures( filteredFeatures );

  QApplication::restoreOverrideCursor();

  if ( filterExpression.hasEvalError() )
  {
    mMessageBar->pushMessage( tr( "Error filtering" ), filterExpression.evalErrorString(), Qgis::Warning, mMessageBarTimeout );
    return;
  }
  mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowFilteredList );
}

void QgsFeatureFilterWidget::replaceSearchWidget( QWidget *oldw, QWidget *neww )
{
  mFilterLayout->removeWidget( oldw );
  oldw->setVisible( false );
  mFilterLayout->addWidget( neww, 0, 0, nullptr );
  neww->setVisible( true );
  neww->setFocus();
}

void QgsFeatureFilterWidget::onFilterQueryTextChanged( const QString &value )
{
  Q_UNUSED( value );
  mFilterQueryTimer.start( 300 );
}

/// @endcond
