/***************************************************************************
                         qgsprocessingconfigurationwidgets.cpp
                         ---------------------
    begin                : April 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsprocessingconfigurationwidgets.h"
#include "qgsprocessingalgorithm.h"
#include "qgsexpressionlineedit.h"
#include "qgsapplication.h"
#include "qgsgui.h"
#include "qgsprocessingguiregistry.h"

#include <QTableWidget>
#include <QGridLayout>
#include <QToolButton>
#include <QLabel>
#include <QCheckBox>
#include <QHeaderView>

///@cond PRIVATE

QgsFilterAlgorithmConfigurationWidget::QgsFilterAlgorithmConfigurationWidget( QWidget *parent )
  : QgsProcessingAlgorithmConfigurationWidget( parent )
{
  setContentsMargins( 0, 0, 0, 0 );

  mOutputExpressionWidget = new QTableWidget();
  mOutputExpressionWidget->setColumnCount( 3 );
  mOutputExpressionWidget->setHorizontalHeaderItem( 0, new QTableWidgetItem( tr( "Output Name" ) ) );
  mOutputExpressionWidget->setHorizontalHeaderItem( 1, new QTableWidgetItem( tr( "Filter Expression" ) ) );
  mOutputExpressionWidget->setHorizontalHeaderItem( 2, new QTableWidgetItem( tr( "Final Output" ) ) );
  mOutputExpressionWidget->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::Stretch );
  QGridLayout *layout = new QGridLayout();
  setLayout( layout );

  layout->addWidget( new QLabel( tr( "Outputs and filters" ) ), 0, 0, 1, 2 );
  layout->addWidget( mOutputExpressionWidget, 1, 0, 4, 1 );
  QToolButton *addOutputButton = new QToolButton();
  addOutputButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddLayer.svg" ) ) );
  addOutputButton->setText( tr( "Add Output" ) );

  QToolButton *removeOutputButton = new QToolButton();
  removeOutputButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRemoveLayer.svg" ) ) );
  removeOutputButton->setToolTip( tr( "Remove Selected Outputs" ) );

  layout->addWidget( addOutputButton, 2, 1, 1, 1 );
  layout->addWidget( removeOutputButton, 3, 1, 1, 1 );

  connect( addOutputButton, &QToolButton::clicked, this, &QgsFilterAlgorithmConfigurationWidget::addOutput );
  connect( removeOutputButton, &QToolButton::clicked, this, &QgsFilterAlgorithmConfigurationWidget::removeSelectedOutputs );

  connect( mOutputExpressionWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, [removeOutputButton, this]
  {
    removeOutputButton->setEnabled( !mOutputExpressionWidget->selectionModel()->selectedIndexes().isEmpty() );
  } );
}

QVariantMap QgsFilterAlgorithmConfigurationWidget::configuration() const
{
  QVariantList outputs;

  for ( int i = 0; i < mOutputExpressionWidget->rowCount(); ++i )
  {
    QVariantMap output;
    output.insert( QStringLiteral( "name" ), mOutputExpressionWidget->item( i, 0 )->text() );
    output.insert( QStringLiteral( "expression" ), qobject_cast<QgsExpressionLineEdit *>( mOutputExpressionWidget->cellWidget( i, 1 ) )->expression() );
    output.insert( QStringLiteral( "isModelOutput" ), qobject_cast<QCheckBox *>( mOutputExpressionWidget->cellWidget( i, 2 ) )->isChecked() );
    outputs.append( output );
  }

  QVariantMap map;
  map.insert( "outputs", outputs );

  return map;
}


void QgsFilterAlgorithmConfigurationWidget::setConfiguration( const QVariantMap &configuration )
{
  mOutputExpressionWidget->setRowCount( 0 );
  int currentRow = 0;
  const QVariantList outputs = configuration.value( "outputs" ).toList();

  for ( const QVariant &outputvar : outputs )
  {
    const QVariantMap output = outputvar.toMap();
    mOutputExpressionWidget->insertRow( currentRow );
    mOutputExpressionWidget->setItem( currentRow, 0, new QTableWidgetItem( output.value( "name" ).toString() ) );
    QgsExpressionLineEdit *expressionBuilder = new QgsExpressionLineEdit();
    expressionBuilder->registerExpressionContextGenerator( this );
    expressionBuilder->setExpression( output.value( "expression" ).toString() );
    mOutputExpressionWidget->setCellWidget( currentRow, 1, expressionBuilder );
    QCheckBox *isModelOutput = new QCheckBox();
    isModelOutput->setChecked( output.value( "isModelOutput" ).toBool() );
    mOutputExpressionWidget->setCellWidget( currentRow, 2, isModelOutput );

    currentRow++;
  }

  if ( outputs.isEmpty() )
    addOutput();
}

void QgsFilterAlgorithmConfigurationWidget::removeSelectedOutputs()
{
  const QItemSelection selection( mOutputExpressionWidget->selectionModel()->selection() );

  QList<int> rows;
  const QModelIndexList indexes = selection.indexes();
  for ( const QModelIndex &index : indexes )
  {
    rows.append( index.row() );
  }

  std::sort( rows.begin(), rows.end() );

  int prev = -1;
  for ( int i = rows.count() - 1; i >= 0; i -= 1 )
  {
    const int current = rows[i];
    if ( current != prev )
    {
      mOutputExpressionWidget->removeRow( current );
      prev = current;
    }
  }
}

void QgsFilterAlgorithmConfigurationWidget::addOutput()
{
  const int rowIndex = mOutputExpressionWidget->rowCount();
  mOutputExpressionWidget->setRowCount( rowIndex + 1 );
  QgsExpressionLineEdit *expressionBuilder = new QgsExpressionLineEdit();
  expressionBuilder->registerExpressionContextGenerator( this );
  mOutputExpressionWidget->setItem( rowIndex, 0, new QTableWidgetItem( QString() ) );
  mOutputExpressionWidget->setCellWidget( rowIndex, 1, expressionBuilder );
  mOutputExpressionWidget->setCellWidget( rowIndex, 2, new QCheckBox() );
}

QgsProcessingAlgorithmConfigurationWidget *QgsFilterAlgorithmConfigurationWidgetFactory::create( const QgsProcessingAlgorithm *algorithm ) const
{
  if ( algorithm->name() == QLatin1String( "filter" ) )
    return new QgsFilterAlgorithmConfigurationWidget();
  else
    return nullptr;
}

bool QgsFilterAlgorithmConfigurationWidgetFactory::canCreateFor( const QgsProcessingAlgorithm *algorithm ) const
{
  return algorithm->name() == QLatin1String( "filter" );
}


//
// QgsConditionalBranchAlgorithmConfigurationWidget
//

QgsConditionalBranchAlgorithmConfigurationWidget::QgsConditionalBranchAlgorithmConfigurationWidget( QWidget *parent )
  : QgsProcessingAlgorithmConfigurationWidget( parent )
{
  setContentsMargins( 0, 0, 0, 0 );

  mConditionExpressionWidget = new QTableWidget();
  mConditionExpressionWidget->setColumnCount( 2 );
  mConditionExpressionWidget->setHorizontalHeaderItem( 0, new QTableWidgetItem( tr( "Branch Name" ) ) );
  mConditionExpressionWidget->setHorizontalHeaderItem( 1, new QTableWidgetItem( tr( "Condition" ) ) );
  mConditionExpressionWidget->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::Stretch );
  QGridLayout *layout = new QGridLayout();
  setLayout( layout );

  layout->addWidget( new QLabel( tr( "Conditions" ) ), 0, 0, 1, 2 );
  layout->addWidget( mConditionExpressionWidget, 1, 0, 4, 1 );
  QToolButton *addConditionButton = new QToolButton();
  addConditionButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddLayer.svg" ) ) );
  addConditionButton->setText( tr( "Add Condition" ) );

  QToolButton *removeConditionButton = new QToolButton();
  removeConditionButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRemoveLayer.svg" ) ) );
  removeConditionButton->setToolTip( tr( "Remove Selected Conditions" ) );

  layout->addWidget( addConditionButton, 2, 1, 1, 1 );
  layout->addWidget( removeConditionButton, 3, 1, 1, 1 );

  connect( addConditionButton, &QToolButton::clicked, this, &QgsConditionalBranchAlgorithmConfigurationWidget::addCondition );
  connect( removeConditionButton, &QToolButton::clicked, this, &QgsConditionalBranchAlgorithmConfigurationWidget::removeSelectedConditions );

  connect( mConditionExpressionWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, [removeConditionButton, this]
  {
    removeConditionButton->setEnabled( !mConditionExpressionWidget->selectionModel()->selectedIndexes().isEmpty() );
  } );
}

QVariantMap QgsConditionalBranchAlgorithmConfigurationWidget::configuration() const
{
  QVariantList outputs;

  for ( int i = 0; i < mConditionExpressionWidget->rowCount(); ++i )
  {
    QVariantMap output;
    output.insert( QStringLiteral( "name" ), mConditionExpressionWidget->item( i, 0 )->text() );
    output.insert( QStringLiteral( "expression" ), qobject_cast<QgsExpressionLineEdit *>( mConditionExpressionWidget->cellWidget( i, 1 ) )->expression() );
    outputs.append( output );
  }

  QVariantMap map;
  map.insert( "conditions", outputs );

  return map;
}


void QgsConditionalBranchAlgorithmConfigurationWidget::setConfiguration( const QVariantMap &configuration )
{
  mConditionExpressionWidget->setRowCount( 0 );
  int currentRow = 0;
  const QVariantList conditions = configuration.value( "conditions" ).toList();

  for ( const QVariant &conditionvar : conditions )
  {
    const QVariantMap output = conditionvar.toMap();
    mConditionExpressionWidget->insertRow( currentRow );
    mConditionExpressionWidget->setItem( currentRow, 0, new QTableWidgetItem( output.value( "name" ).toString() ) );
    QgsExpressionLineEdit *expressionBuilder = new QgsExpressionLineEdit();
    expressionBuilder->registerExpressionContextGenerator( this );
    expressionBuilder->setExpression( output.value( "expression" ).toString() );
    mConditionExpressionWidget->setCellWidget( currentRow, 1, expressionBuilder );
    currentRow++;
  }

  if ( conditions .isEmpty() )
    addCondition();
}

void QgsConditionalBranchAlgorithmConfigurationWidget::removeSelectedConditions()
{
  const QItemSelection selection( mConditionExpressionWidget->selectionModel()->selection() );

  QList<int> rows;
  const QModelIndexList indexes = selection.indexes();
  for ( const QModelIndex &index : indexes )
  {
    rows.append( index.row() );
  }

  std::sort( rows.begin(), rows.end() );

  int prev = -1;
  for ( int i = rows.count() - 1; i >= 0; i -= 1 )
  {
    const int current = rows[i];
    if ( current != prev )
    {
      mConditionExpressionWidget->removeRow( current );
      prev = current;
    }
  }
}

void QgsConditionalBranchAlgorithmConfigurationWidget::addCondition()
{
  const int rowIndex = mConditionExpressionWidget->rowCount();
  mConditionExpressionWidget->setRowCount( rowIndex + 1 );
  QgsExpressionLineEdit *expressionBuilder = new QgsExpressionLineEdit();
  expressionBuilder->registerExpressionContextGenerator( this );
  mConditionExpressionWidget->setItem( rowIndex, 0, new QTableWidgetItem( QString() ) );
  mConditionExpressionWidget->setCellWidget( rowIndex, 1, expressionBuilder );
}

QgsConditionalBranchAlgorithmConfigurationWidget *QgsConditionalBranchAlgorithmConfigurationWidgetFactory::create( const QgsProcessingAlgorithm *algorithm ) const
{
  if ( algorithm->name() == QLatin1String( "condition" ) )
    return new QgsConditionalBranchAlgorithmConfigurationWidget();
  else
    return nullptr;
}

bool QgsConditionalBranchAlgorithmConfigurationWidgetFactory::canCreateFor( const QgsProcessingAlgorithm *algorithm ) const
{
  return algorithm->name() == QLatin1String( "condition" );
}


///@endcond PRIVATE
