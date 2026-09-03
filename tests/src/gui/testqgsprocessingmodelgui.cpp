/***************************************************************************
                         testqgsprocessingmodelgui.cpp
                         ---------------------------
    begin                : August 2026
    copyright            : What's the point? It means nothing in 2026...
    email                : nyall
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsconfig.h"

#include "processing/models/qgsprocessingmodelalgorithm.h"
#include "qgsapplication.h"
#include "qgscolorbutton.h"
#include "qgsmodelchildalgorithmwidgets.h"
#include "qgsmodeldesignerdialog.h"
#include "qgsnativealgorithms.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingmodelchildalgorithm.h"
#include "qgsprocessingmodelerparameterwidget.h"
#include "qgsprocessingmodeloutput.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingwidgetwrapper.h"
#include "qgstest.h"

#include <QLineEdit>
#include <QSignalSpy>
#include <QString>

using namespace Qt::StringLiterals;

class TestQgsProcessingModelGui : public QgsTest
{
    Q_OBJECT
  public:
    TestQgsProcessingModelGui()
      : QgsTest( "Processing Model GUI" )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testModelerParametersPanelWidgetConstructAndAlgorithm();
    void testModelerParametersPanelWidgetCreateAlgorithm();
    void testModelerParametersPanelWidgetSetStateFromChildAlgorithm();
    void testModelerParametersPanelWidgetWidgetChangedSignal();
    void testModelerParametersPanelWidgetDependencies();
    void testModelerParametersPanelWidgetSetDependencies();
    void testModelerParametersPanelWidgetParameterSourcesLists();

    void testModelerParametersWidgetConstructAndAlgorithm();
    void testModelerParametersWidgetCommentsAndColor();
    void testModelerParametersWidgetCreateAlgorithm();
    void testModelerParametersWidgetSetStateFromChildAlgorithm();
    void testModelerParametersWidgetWidgetChangedSignal();
};

void TestQgsProcessingModelGui::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );
}

void TestQgsProcessingModelGui::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsProcessingModelGui::init()
{}

void TestQgsProcessingModelGui::cleanup()
{}

void TestQgsProcessingModelGui::testModelerParametersPanelWidgetConstructAndAlgorithm()
{
  QgsProcessingModelAlgorithm model( u"test_model"_s, u"Test Group"_s );
  QgsProcessingContext context;
  const QgsProcessingAlgorithm *bufferAlg = QgsApplication::processingRegistry()->algorithmById( u"native:buffer"_s );
  QVERIFY( bufferAlg );

  QgsProcessingModelerParametersPanelWidget widget( bufferAlg, &model, context );
  QCOMPARE( widget.algorithm()->id(), u"native:buffer"_s );

  QgsProcessingModelerParametersPanelWidget nullAlgWidget( nullptr, &model, context );
  QVERIFY( !nullAlgWidget.algorithm() );
}

void TestQgsProcessingModelGui::testModelerParametersPanelWidgetCreateAlgorithm()
{
  QgsProcessingModelAlgorithm model( u"test_model"_s, u"Test Group"_s );
  QgsProcessingContext context;

  const QgsProcessingAlgorithm *bufferAlg = QgsApplication::processingRegistry()->algorithmById( u"native:buffer"_s );
  QVERIFY( bufferAlg );

  QgsProcessingModelerParametersPanelWidget widget( bufferAlg, &model, context );

  // test creating child algorithm instance
  std::unique_ptr< QgsProcessingModelChildAlgorithm > childAlg = widget.createAlgorithm();
  QVERIFY( childAlg );
  QCOMPARE( childAlg->algorithmId(), u"native:buffer"_s );
  QCOMPARE( childAlg->description(), u"Buffer"_s );
  QCOMPARE( childAlg->childId(), u"native:buffer_1"_s );

  // verify default parameter values are created
  QCOMPARE( childAlg->parameterSources().value( u"DISTANCE"_s ).at( 0 ).staticValue().toDouble(), 10.0 );
}

void TestQgsProcessingModelGui::testModelerParametersPanelWidgetSetStateFromChildAlgorithm()
{
  QgsProcessingModelAlgorithm model( u"test_model"_s, u"Test Group"_s );
  QgsProcessingContext context;

  const QgsProcessingAlgorithm *bufferAlg = QgsApplication::processingRegistry()->algorithmById( u"native:buffer"_s );
  QVERIFY( bufferAlg );

  // create child algorithm and add it to model
  QgsProcessingModelChildAlgorithm child( u"native:buffer"_s );
  child.setChildId( u"buffer_1"_s );
  child.setDescription( u"Custom Buffer Name"_s );
  child.addParameterSources( u"DISTANCE"_s, QList< QgsProcessingModelChildParameterSource >() << QgsProcessingModelChildParameterSource::fromStaticValue( 25.0 ) );

  QgsProcessingModelOutput modelOutput( u"buffered_output"_s, u"Buffered Output"_s );
  modelOutput.setChildId( u"buffer_1"_s );
  modelOutput.setChildOutputName( u"OUTPUT"_s );

  QMap< QString, QgsProcessingModelOutput > outputs;
  outputs.insert( u"buffered_output"_s, modelOutput );
  child.setModelOutputs( outputs );

  model.addChildAlgorithm( child );

  // instantiate widget with existing child
  QgsProcessingModelerParametersPanelWidget widget( bufferAlg, &model, context, u"buffer_1"_s );

  QCOMPARE( widget.mWrappers[u"DISTANCE"_s]->value().value< QgsProcessingModelChildParameterSource >(), QgsProcessingModelChildParameterSource::fromStaticValue( 25.0 ) );
  widget.mWrappers[u"DISTANCE"_s]->setWidgetValue( QgsProcessingModelChildParameterSource::fromStaticValue( 15.0 ) );

  std::unique_ptr< QgsProcessingModelChildAlgorithm > recreatedAlg = widget.createAlgorithm();
  QVERIFY( recreatedAlg );
  QCOMPARE( recreatedAlg->childId(), u"buffer_1"_s );
  // properties for existing child should be retained, eg description
  QCOMPARE( recreatedAlg->description(), u"Custom Buffer Name"_s );

  // verify loaded parameter values
  const QList< QgsProcessingModelChildParameterSource > sources = recreatedAlg->parameterSources().value( u"DISTANCE"_s );
  QCOMPARE( sources.size(), 1 );
  QCOMPARE( sources.at( 0 ).staticValue().toDouble(), 15.0 );

  // verify loaded model outputs
  QCOMPARE( recreatedAlg->modelOutputs().size(), 1 );
  QVERIFY( recreatedAlg->modelOutputs().contains( u"buffered_output"_s ) );
  QCOMPARE( recreatedAlg->modelOutputs().value( u"buffered_output"_s ).childOutputName(), u"OUTPUT"_s );
}

void TestQgsProcessingModelGui::testModelerParametersPanelWidgetWidgetChangedSignal()
{
  QgsProcessingModelAlgorithm model( u"test_model"_s, u"Test Group"_s );
  QgsProcessingContext context;

  const QgsProcessingAlgorithm *bufferAlg = QgsApplication::processingRegistry()->algorithmById( u"native:buffer"_s );
  QVERIFY( bufferAlg );

  QgsProcessingModelerParametersPanelWidget widget( bufferAlg, &model, context );
  QSignalSpy spy( &widget, &QgsProcessingModelerParametersPanelWidget::widgetChanged );

  QLineEdit *descriptionBox = widget.findChild< QLineEdit * >( u"mDescriptionBox"_s );
  QVERIFY( descriptionBox );

  // test signal emission on description change
  descriptionBox->setText( u"New Description"_s );
  QCOMPARE( spy.count(), 1 );

  // change a parameter value
  widget.mWrappers[u"DISTANCE"_s]->setWidgetValue( QgsProcessingModelChildParameterSource::fromStaticValue( 15.0 ) );
  QCOMPARE( spy.count(), 2 );
}

void TestQgsProcessingModelGui::testModelerParametersPanelWidgetDependencies()
{
  QgsProcessingModelAlgorithm model( u"test_model"_s, u"Test Group"_s );
  QgsProcessingContext context;

  QgsProcessingModelChildAlgorithm parentChild( u"native:buffer"_s );
  parentChild.setChildId( u"parent_alg"_s );
  parentChild.setDescription( u"Parent Alg"_s );
  model.addChildAlgorithm( parentChild );

  const QgsProcessingAlgorithm *bufferAlg = QgsApplication::processingRegistry()->algorithmById( u"native:buffer"_s );
  QVERIFY( bufferAlg );

  QgsProcessingModelChildAlgorithm child( u"native:buffer"_s );
  child.setChildId( u"child_alg"_s );
  child.setDependencies( QList< QgsProcessingModelChildDependency >() << QgsProcessingModelChildDependency( u"parent_alg"_s ) );
  model.addChildAlgorithm( child );

  QgsProcessingModelerParametersPanelWidget widget( bufferAlg, &model, context, u"child_alg"_s );

  // ensure existing dependencies are retained
  std::unique_ptr< QgsProcessingModelChildAlgorithm > recreatedAlg = widget.createAlgorithm();
  QVERIFY( recreatedAlg );
  QCOMPARE( recreatedAlg->dependencies().size(), 1 );
  QCOMPARE( recreatedAlg->dependencies().at( 0 ).childId, u"parent_alg"_s );
}

void TestQgsProcessingModelGui::testModelerParametersPanelWidgetSetDependencies()
{
  QgsProcessingModelAlgorithm model( u"test_model"_s, u"Test Group"_s );
  QgsProcessingContext context;

  QgsProcessingModelChildAlgorithm parentChild1( u"native:buffer"_s );
  parentChild1.setChildId( u"parent_alg_1"_s );
  parentChild1.setDescription( u"Parent Alg 1"_s );
  model.addChildAlgorithm( parentChild1 );

  QgsProcessingModelChildAlgorithm parentChild2( u"native:buffer"_s );
  parentChild2.setChildId( u"parent_alg_2"_s );
  parentChild2.setDescription( u"Parent Alg 2"_s );
  model.addChildAlgorithm( parentChild2 );

  const QgsProcessingAlgorithm *bufferAlg = QgsApplication::processingRegistry()->algorithmById( u"native:buffer"_s );
  QVERIFY( bufferAlg );

  QgsProcessingModelChildAlgorithm child( u"native:buffer"_s );
  child.setChildId( u"child_alg"_s );
  model.addChildAlgorithm( child );

  QgsProcessingModelerParametersPanelWidget widget( bufferAlg, &model, context, u"child_alg"_s );
  QSignalSpy spy( &widget, &QgsProcessingModelerParametersPanelWidget::widgetChanged );

  const QList< QgsProcessingModelChildDependency > newDeps { QgsProcessingModelChildDependency( u"parent_alg_1"_s ), QgsProcessingModelChildDependency( u"parent_alg_2"_s ) };

  widget.mDependenciesPanel->setValue( newDeps );
  // make sure changing the dependencies triggers a changed signal
  QCOMPARE( spy.count(), 1 );

  std::unique_ptr< QgsProcessingModelChildAlgorithm > recreatedAlg = widget.createAlgorithm();
  QVERIFY( recreatedAlg );
  QCOMPARE( recreatedAlg->dependencies().size(), 2 );
  QCOMPARE( recreatedAlg->dependencies().at( 0 ).childId, u"parent_alg_1"_s );
  QCOMPARE( recreatedAlg->dependencies().at( 1 ).childId, u"parent_alg_2"_s );

  widget.mDependenciesPanel->setValue( { QgsProcessingModelChildDependency( u"parent_alg_1"_s ) } );
  QCOMPARE( spy.count(), 2 );

  recreatedAlg = widget.createAlgorithm();
  QVERIFY( recreatedAlg );
  QCOMPARE( recreatedAlg->dependencies().size(), 1 );
  QCOMPARE( recreatedAlg->dependencies().at( 0 ).childId, u"parent_alg_1"_s );
}

void TestQgsProcessingModelGui::testModelerParametersPanelWidgetParameterSourcesLists()
{
  QgsProcessingModelAlgorithm model( u"test_model"_s, u"Test Group"_s );
  model.addModelParameter( new QgsProcessingParameterVectorLayer( u"input_layer_1"_s, u"Input Layer 1"_s ), QgsProcessingModelParameter( u"input_layer_1"_s ) );

  QgsProcessingContext context;

  const QgsProcessingAlgorithm *mergeAlg = QgsApplication::processingRegistry()->algorithmById( u"native:mergevectorlayers"_s );
  QVERIFY( mergeAlg );

  // test empty source list
  QgsProcessingModelerParametersPanelWidget widgetEmpty( mergeAlg, &model, context );
  widgetEmpty.mWrappers[u"LAYERS"_s]->setWidgetValue( QList<QgsProcessingModelChildParameterSource>() );

  std::unique_ptr< QgsProcessingModelChildAlgorithm > childAlgEmpty = widgetEmpty.createAlgorithm();
  QVERIFY( childAlgEmpty );
  QVERIFY( childAlgEmpty->parameterSources().value( u"LAYERS"_s ).isEmpty() );

  // test list with single source
  QgsProcessingModelerParametersPanelWidget widgetSize1( mergeAlg, &model, context );
  const QList< QgsProcessingModelChildParameterSource > sources1 { QgsProcessingModelChildParameterSource::fromModelParameter( u"input_layer_1"_s ) };
  widgetSize1.mWrappers[u"LAYERS"_s]->setWidgetValue( sources1 );

  std::unique_ptr< QgsProcessingModelChildAlgorithm > childAlgSize1 = widgetSize1.createAlgorithm();
  QVERIFY( childAlgSize1 );
  const QList< QgsProcessingModelChildParameterSource > createdSources1 = childAlgSize1->parameterSources().value( u"LAYERS"_s );
  QCOMPARE( createdSources1.size(), 1 );
  QCOMPARE( createdSources1.at( 0 ).source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( createdSources1.at( 0 ).parameterName(), u"input_layer_1"_s );

  // test loading existing child algorithm with single item list
  QgsProcessingModelChildAlgorithm child1( u"native:mergevectorlayers"_s );
  child1.setChildId( u"merge_1"_s );
  child1.addParameterSources( u"LAYERS"_s, sources1 );
  model.addChildAlgorithm( child1 );

  QgsProcessingModelerParametersPanelWidget widgetLoadedSize1( mergeAlg, &model, context, u"merge_1"_s );
  std::unique_ptr< QgsProcessingModelChildAlgorithm > recreatedSize1 = widgetLoadedSize1.createAlgorithm();
  QVERIFY( recreatedSize1 );
  const QList< QgsProcessingModelChildParameterSource > loadedSources1 = recreatedSize1->parameterSources().value( u"LAYERS"_s );
  QCOMPARE( loadedSources1.size(), 1 );
  QCOMPARE( loadedSources1.at( 0 ).source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( loadedSources1.at( 0 ).parameterName(), u"input_layer_1"_s );

  // test list with size 3 containing differing source types
  QgsProcessingModelerParametersPanelWidget widgetSize3( mergeAlg, &model, context );
  const QList< QgsProcessingModelChildParameterSource >
    sources3 { QgsProcessingModelChildParameterSource::fromModelParameter( u"input_layer_1"_s ), QgsProcessingModelChildParameterSource::fromChildOutput( u"child_alg_1"_s, u"OUTPUT"_s ), QgsProcessingModelChildParameterSource::fromStaticValue( u"static_layer_path.gpkg"_s ) };
  widgetSize3.mWrappers[u"LAYERS"_s]->setWidgetValue( sources3 );

  std::unique_ptr< QgsProcessingModelChildAlgorithm > childAlgSize3 = widgetSize3.createAlgorithm();
  QVERIFY( childAlgSize3 );
  const QList< QgsProcessingModelChildParameterSource > createdSources3 = childAlgSize3->parameterSources().value( u"LAYERS"_s );
  QCOMPARE( createdSources3.size(), 3 );
  QCOMPARE( createdSources3.at( 0 ).source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( createdSources3.at( 0 ).parameterName(), u"input_layer_1"_s );
  QCOMPARE( createdSources3.at( 1 ).source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( createdSources3.at( 1 ).outputChildId(), u"child_alg_1"_s );
  QCOMPARE( createdSources3.at( 1 ).outputName(), u"OUTPUT"_s );
  QCOMPARE( createdSources3.at( 2 ).source(), Qgis::ProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( createdSources3.at( 2 ).staticValue().toString(), u"static_layer_path.gpkg"_s );

  // test loading existing child algorithm with list size 3
  QgsProcessingModelChildAlgorithm child3( u"native:mergevectorlayers"_s );
  child3.setChildId( u"merge_3"_s );
  child3.addParameterSources( u"LAYERS"_s, sources3 );
  model.addChildAlgorithm( child3 );

  QgsProcessingModelerParametersPanelWidget widgetLoadedSize3( mergeAlg, &model, context, u"merge_3"_s );
  std::unique_ptr< QgsProcessingModelChildAlgorithm > recreatedSize3 = widgetLoadedSize3.createAlgorithm();
  QVERIFY( recreatedSize3 );
  const QList< QgsProcessingModelChildParameterSource > loadedSources3 = recreatedSize3->parameterSources().value( u"LAYERS"_s );
  QCOMPARE( loadedSources3.size(), 3 );
  QCOMPARE( loadedSources3.at( 0 ).source(), Qgis::ProcessingModelChildParameterSource::ModelParameter );
  QCOMPARE( loadedSources3.at( 0 ).parameterName(), u"input_layer_1"_s );
  QCOMPARE( loadedSources3.at( 1 ).source(), Qgis::ProcessingModelChildParameterSource::ChildOutput );
  QCOMPARE( loadedSources3.at( 1 ).outputChildId(), u"child_alg_1"_s );
  QCOMPARE( loadedSources3.at( 1 ).outputName(), u"OUTPUT"_s );
  QCOMPARE( loadedSources3.at( 2 ).source(), Qgis::ProcessingModelChildParameterSource::StaticValue );
  QCOMPARE( loadedSources3.at( 2 ).staticValue().toString(), u"static_layer_path.gpkg"_s );
}

void TestQgsProcessingModelGui::testModelerParametersWidgetConstructAndAlgorithm()
{
  QgsProcessingModelAlgorithm model( u"test_model"_s, u"Test Group"_s );
  QgsProcessingContext context;
  const QgsProcessingAlgorithm *bufferAlg = QgsApplication::processingRegistry()->algorithmById( u"native:buffer"_s );
  QVERIFY( bufferAlg );

  QgsProcessingModelerParametersWidget widget( bufferAlg, &model, context );
  QCOMPARE( widget.algorithm()->id(), u"native:buffer"_s );
  QVERIFY( widget.comments().isEmpty() );
  QVERIFY( !widget.commentColor().isValid() );
}

void TestQgsProcessingModelGui::testModelerParametersWidgetCommentsAndColor()
{
  QgsProcessingModelAlgorithm model( u"test_model"_s, u"Test Group"_s );
  QgsProcessingContext context;
  const QgsProcessingAlgorithm *bufferAlg = QgsApplication::processingRegistry()->algorithmById( u"native:buffer"_s );
  QVERIFY( bufferAlg );

  QgsProcessingModelerParametersWidget widget( bufferAlg, &model, context );

  widget.setComments( u"comment text"_s );
  QCOMPARE( widget.comments(), u"comment text"_s );

  const QColor redColor( 255, 0, 0 );
  widget.setCommentColor( redColor );
  QCOMPARE( widget.commentColor(), redColor );

  widget.setCommentColor( QColor() );
  QVERIFY( !widget.commentColor().isValid() );
}

void TestQgsProcessingModelGui::testModelerParametersWidgetCreateAlgorithm()
{
  QgsProcessingModelAlgorithm model( u"test_model"_s, u"Test Group"_s );
  QgsProcessingContext context;
  const QgsProcessingAlgorithm *bufferAlg = QgsApplication::processingRegistry()->algorithmById( u"native:buffer"_s );
  QVERIFY( bufferAlg );

  QgsProcessingModelerParametersWidget widget( bufferAlg, &model, context );
  widget.setComments( u"algorithm comment"_s );
  const QColor blueColor( 0, 0, 255 );
  widget.setCommentColor( blueColor );

  std::unique_ptr< QgsProcessingModelChildAlgorithm > childAlg = widget.createAlgorithm();
  QVERIFY( childAlg );
  QCOMPARE( childAlg->algorithmId(), u"native:buffer"_s );
  QCOMPARE( childAlg->comment()->description(), u"algorithm comment"_s );
  QCOMPARE( childAlg->comment()->color(), blueColor );
}

void TestQgsProcessingModelGui::testModelerParametersWidgetSetStateFromChildAlgorithm()
{
  QgsProcessingModelAlgorithm model( u"test_model"_s, u"Test Group"_s );
  QgsProcessingContext context;

  const QgsProcessingAlgorithm *bufferAlg = QgsApplication::processingRegistry()->algorithmById( u"native:buffer"_s );
  QVERIFY( bufferAlg );

  QgsProcessingModelChildAlgorithm child( u"native:buffer"_s );
  child.setChildId( u"buffer_1"_s );
  child.setDescription( u"Buffered Layer"_s );
  child.comment()->setDescription( u"existing comment"_s );
  const QColor greenColor( 0, 255, 0 );
  child.comment()->setColor( greenColor );
  child.addParameterSources( u"DISTANCE"_s, QList< QgsProcessingModelChildParameterSource >() << QgsProcessingModelChildParameterSource::fromStaticValue( 50.0 ) );

  model.addChildAlgorithm( child );

  QgsProcessingModelerParametersWidget widget( bufferAlg, &model, context, u"buffer_1"_s );
  widget.setComments( child.comment()->description() );
  widget.setCommentColor( child.comment()->color() );
  widget.setStateFromChildAlgorithm();

  QCOMPARE( widget.comments(), u"existing comment"_s );
  QCOMPARE( widget.commentColor(), greenColor );

  std::unique_ptr< QgsProcessingModelChildAlgorithm > recreatedAlg = widget.createAlgorithm();
  QVERIFY( recreatedAlg );
  QCOMPARE( recreatedAlg->childId(), u"buffer_1"_s );
  QCOMPARE( recreatedAlg->description(), u"Buffered Layer"_s );
  QCOMPARE( recreatedAlg->comment()->description(), u"existing comment"_s );
  QCOMPARE( recreatedAlg->comment()->color(), greenColor );
  QCOMPARE( recreatedAlg->parameterSources().value( u"DISTANCE"_s ).at( 0 ).staticValue().toDouble(), 50.0 );
}

void TestQgsProcessingModelGui::testModelerParametersWidgetWidgetChangedSignal()
{
  QgsProcessingModelAlgorithm model( u"test_model"_s, u"Test Group"_s );
  QgsProcessingContext context;

  const QgsProcessingAlgorithm *bufferAlg = QgsApplication::processingRegistry()->algorithmById( u"native:buffer"_s );
  QVERIFY( bufferAlg );

  QgsProcessingModelerParametersWidget widget( bufferAlg, &model, context );
  QSignalSpy spy( &widget, &QgsProcessingModelerParametersWidget::widgetChanged );

  widget.mCommentEdit->setText( u"Updated Description"_s );
  QCOMPARE( spy.count(), 1 );

  widget.mCommentColorButton->setColor( QColor( 255, 0, 255 ) );
  QCOMPARE( spy.count(), 2 );
}

QGSTEST_MAIN( TestQgsProcessingModelGui )
#include "testqgsprocessingmodelgui.moc"
