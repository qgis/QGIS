/***************************************************************************
                         qgsalgorithmfilter.cpp
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

#include "qgsalgorithmfilter.h"
#include "qgsapplication.h"

#include <QLabel>
#include <QGridLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QToolButton>

///@cond PRIVATE

QString QgsFilterAlgorithm::name() const
{
  return QStringLiteral( "filter" );
}

QString QgsFilterAlgorithm::displayName() const
{
  return QObject::tr( "Filter" );
}

QStringList QgsFilterAlgorithm::tags() const
{
  return QObject::tr( "filter,proxy,redirect,route" ).split( ',' );
}

QString QgsFilterAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsFilterAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QgsProcessingAlgorithm::Flags QgsFilterAlgorithm::flags() const
{
  return FlagHideFromToolbox;
}

QString QgsFilterAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm filters features from the input layer and redirects them to one or several outputs." );
}

QgsFilterAlgorithm *QgsFilterAlgorithm::createInstance() const
{
  return new QgsFilterAlgorithm();
}

QgsProcessingAlgorithmConfigurationWidget *QgsFilterAlgorithm::createModelerWidget() const
{
  return new QgsFilterAlgorithmConfigurationWidget();
}

QgsFilterAlgorithm::~QgsFilterAlgorithm()
{
  qDeleteAll( mOutputs );
}

void QgsFilterAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );

  const QVariantList outputs = configuration.value( QStringLiteral( "outputs" ) ).toList();
  for ( const QVariant &output : outputs )
  {
    const QVariantMap outputDef = output.toMap();
    const QString name = QStringLiteral( "OUTPUT_%1" ).arg( outputDef.value( QStringLiteral( "name" ) ).toString() );
    QgsProcessingParameterFeatureSink *outputParam = new QgsProcessingParameterFeatureSink( name, outputDef.value( QStringLiteral( "name" ) ).toString() );
    outputParam->setFlags( QgsProcessingParameterDefinition::FlagHidden );
    addParameter( outputParam );
    mOutputs.append( new Output( name, outputDef.value( QStringLiteral( "expression" ) ).toString() ) );
  }
}


QgsFilterAlgorithmConfigurationWidget::QgsFilterAlgorithmConfigurationWidget( QWidget *parent )
  : QgsProcessingAlgorithmConfigurationWidget( parent )
{
  setContentsMargins( 0, 0, 0, 0 );

  mOutputExpressionWidget = new QTableWidget();
  mOutputExpressionWidget->setColumnCount( 2 );
  mOutputExpressionWidget->setHorizontalHeaderItem( 0, new QTableWidgetItem( tr( "Output Name" ) ) );
  mOutputExpressionWidget->setHorizontalHeaderItem( 1, new QTableWidgetItem( tr( "Filter Expression" ) ) );
  mOutputExpressionWidget->horizontalHeader()->setStretchLastSection( true );
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

  connect( mOutputExpressionWidget->selectionModel(), &QItemSelectionModel::selectionChanged, [removeOutputButton, this]
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
    output.insert( QStringLiteral( "expression" ), mOutputExpressionWidget->item( i, 1 )->text() );
    outputs.append( output );
  }

  QVariantMap map;
  map.insert( "outputs", outputs );

  return map;
}


QVariantMap QgsFilterAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QgsExpressionContext expressionContext = context.expressionContext();
  for ( Output *output : qgis::as_const( mOutputs ) )
  {
    output->sink.reset( parameterAsSink( parameters, output->name, context, output->destinationIdentifier, source->fields(), source->wkbType(), source->sourceCrs() ) );
    output->expression.prepare( &expressionContext );
  }

  long count = source->featureCount();

  QgsFeature f;
  QgsFeatureIterator it = source->getFeatures();

  double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    expressionContext.setFeature( f );

    for ( Output *output : qgis::as_const( mOutputs ) )
    {
      if ( output->expression.evaluate( &expressionContext ).toBool() )
        output->sink->addFeature( f, QgsFeatureSink::FastInsert );
    }

    feedback->setProgress( current * step );
    current++;
  }

  QVariantMap outputs;
  for ( const Output *output : qgis::as_const( mOutputs ) )
  {
    outputs.insert( output->name, output->destinationIdentifier );
  }
  return outputs;
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
    mOutputExpressionWidget->setItem( currentRow, 1, new QTableWidgetItem( output.value( "expression" ).toString() ) );

    currentRow++;
  }

  if ( outputs.isEmpty() )
    addOutput();
}

void QgsFilterAlgorithmConfigurationWidget::removeSelectedOutputs()
{
  QItemSelection selection( mOutputExpressionWidget->selectionModel()->selection() );

  QList<int> rows;
  const QModelIndexList indexes = selection.indexes();
  for ( const QModelIndex &index : indexes )
  {
    rows.append( index.row() );
  }

  qSort( rows );

  int prev = -1;
  for ( int i = rows.count() - 1; i >= 0; i -= 1 )
  {
    int current = rows[i];
    if ( current != prev )
    {
      mOutputExpressionWidget->removeRow( current );
      prev = current;
    }
  }
}

void QgsFilterAlgorithmConfigurationWidget::addOutput()
{
  mOutputExpressionWidget->setRowCount( mOutputExpressionWidget->rowCount() + 1 );
}
