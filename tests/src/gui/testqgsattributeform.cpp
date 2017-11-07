/***************************************************************************
    testqgsattributeform.cpp
     --------------------------------------
    Date                 : 13 05 2016
    Copyright            : (C) 2016 Paul Blottiere
    Email                : paul dot blottiere at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"
#include <QPushButton>

#include <editorwidgets/core/qgseditorwidgetregistry.h>
#include "qgsattributeform.h"
#include <qgsapplication.h>
#include "qgseditorwidgetwrapper.h"
#include <qgsvectorlayer.h>
#include "qgsvectordataprovider.h"
#include <qgsfeature.h>
#include <qgsvectorlayerjoininfo.h>
#include "qgsgui.h"
#include "qgsattributeformeditorwidget.h"

class TestQgsAttributeForm : public QObject
{
    Q_OBJECT
  public:
    TestQgsAttributeForm() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testFieldConstraint();
    void testFieldMultiConstraints();
    void testOKButtonStatus();
    void testDynamicForm();
    void testConstraintsOnJoinedFields();
    void testEditableJoin();
    void testUpsertOnEdit();

  private:
    QLabel *constraintsLabel( QgsAttributeForm *form, QgsEditorWidgetWrapper *ww )
    {
      QgsAttributeFormEditorWidget *formEditorWidget = form->mFormEditorWidgets.value( ww->fieldIdx() );
      return formEditorWidget->findChild<QLabel *>( QStringLiteral( "ConstraintStatus" ) );
    }
};

void TestQgsAttributeForm::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsGui::editorWidgetRegistry()->initEditors();
}

void TestQgsAttributeForm::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAttributeForm::init()
{
}

void TestQgsAttributeForm::cleanup()
{
}

void TestQgsAttributeForm::testFieldConstraint()
{
  // make a temporary vector layer
  QString def = QStringLiteral( "Point?field=col0:integer" );
  QgsVectorLayer *layer = new QgsVectorLayer( def, QStringLiteral( "test" ), QStringLiteral( "memory" ) );
  layer->setEditorWidgetSetup( 0, QgsEditorWidgetSetup( QStringLiteral( "TextEdit" ), QVariantMap() ) );

  // add a feature to the vector layer
  QgsFeature ft( layer->dataProvider()->fields(), 1 );
  ft.setAttribute( QStringLiteral( "col0" ), 0 );

  // build a form for this feature
  QgsAttributeForm form( layer );
  form.setFeature( ft );

  // testing stuff
  QString validLabel = QStringLiteral( "<font color=\"#259B24\">%1</font>" ).arg( QChar( 0x2714 ) );
  QString invalidLabel = QStringLiteral( "<font color=\"#FF9800\">%1</font>" ).arg( QChar( 0x2718 ) );
  QString warningLabel = QStringLiteral( "<font color=\"#FFC107\">%1</font>" ).arg( QChar( 0x2718 ) );

  // set constraint
  layer->setConstraintExpression( 0, QString() );

  // get wrapper
  QgsEditorWidgetWrapper *ww = nullptr;
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[0] );

  // no constraint so we expect an empty label
  QCOMPARE( constraintsLabel( &form, ww )->text(), QString() );

  // set a not null constraint
  layer->setConstraintExpression( 0, QStringLiteral( "col0 is not null" ) );
  // build a form for this feature
  QgsAttributeForm form2( layer );
  form2.setFeature( ft );
  QSignalSpy spy( &form2, SIGNAL( attributeChanged( QString, QVariant ) ) );
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form2.mWidgets[0] );

  // set value to 1
  ww->setValue( 1 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( constraintsLabel( &form2, ww )->text(), validLabel );

  // set value to null
  spy.clear();
  ww->setValue( QVariant() );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( constraintsLabel( &form2, ww )->text(), invalidLabel );

  // set value to 1
  spy.clear();
  ww->setValue( 1 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( constraintsLabel( &form2, ww )->text(), validLabel );

  // set a soft constraint
  layer->setConstraintExpression( 0, QStringLiteral( "col0 is not null" ) );
  layer->setFieldConstraint( 0, QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintStrengthSoft );
  // build a form for this feature
  QgsAttributeForm form3( layer );
  form3.setFeature( ft );
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form3.mWidgets[0] );

  // set value to 1
  ww->setValue( 1 );
  QCOMPARE( constraintsLabel( &form3, ww )->text(), validLabel );

  // set value to null
  ww->setValue( QVariant() );
  QCOMPARE( constraintsLabel( &form3, ww )->text(), warningLabel );

  // set value to 1
  ww->setValue( 1 );
  QCOMPARE( constraintsLabel( &form3, ww )->text(), validLabel );
}

void TestQgsAttributeForm::testFieldMultiConstraints()
{
  // make a temporary layer to check through
  QString def = QStringLiteral( "Point?field=col0:integer&field=col1:integer&field=col2:integer&field=col3:integer" );
  QgsVectorLayer *layer = new QgsVectorLayer( def, QStringLiteral( "test" ), QStringLiteral( "memory" ) );

  // add features to the vector layer
  QgsFeature ft( layer->dataProvider()->fields(), 1 );
  ft.setAttribute( QStringLiteral( "col0" ), 0 );
  ft.setAttribute( QStringLiteral( "col1" ), 1 );
  ft.setAttribute( QStringLiteral( "col2" ), 2 );
  ft.setAttribute( QStringLiteral( "col3" ), 3 );

  // set constraints for each field
  layer->setConstraintExpression( 0, QString() );
  layer->setConstraintExpression( 1, QString() );
  layer->setConstraintExpression( 2, QString() );
  layer->setConstraintExpression( 3, QString() );

  // build a form for this feature
  QgsAttributeForm form( layer );
  form.setFeature( ft );

  // testing stuff
  QSignalSpy spy( &form, SIGNAL( attributeChanged( QString, QVariant ) ) );
  QString val = QStringLiteral( "<font color=\"#259B24\">%1</font>" ).arg( QChar( 0x2714 ) );
  QString inv = QStringLiteral( "<font color=\"#FF9800\">%1</font>" ).arg( QChar( 0x2718 ) );

  // get wrappers for each widget
  QgsEditorWidgetWrapper *ww0, *ww1, *ww2, *ww3;
  ww0 = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[0] );
  ww1 = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[1] );
  ww2 = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[2] );
  ww3 = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[3] );

  // no constraint so we expect an empty label
  Q_ASSERT( constraintsLabel( &form, ww0 )->text().isEmpty() );
  Q_ASSERT( constraintsLabel( &form, ww1 )->text().isEmpty() );
  Q_ASSERT( constraintsLabel( &form, ww2 )->text().isEmpty() );
  Q_ASSERT( constraintsLabel( &form, ww3 )->text().isEmpty() );

  // update constraint
  layer->setConstraintExpression( 0, QStringLiteral( "col0 < (col1 * col2)" ) );
  layer->setConstraintExpression( 1, QString() );
  layer->setConstraintExpression( 2, QString() );
  layer->setConstraintExpression( 3, QStringLiteral( "col0 = 2" ) );

  QgsAttributeForm form2( layer );
  form2.setFeature( ft );
  ww0 = qobject_cast<QgsEditorWidgetWrapper *>( form2.mWidgets[0] );
  ww1 = qobject_cast<QgsEditorWidgetWrapper *>( form2.mWidgets[1] );
  ww2 = qobject_cast<QgsEditorWidgetWrapper *>( form2.mWidgets[2] );
  ww3 = qobject_cast<QgsEditorWidgetWrapper *>( form2.mWidgets[3] );
  QSignalSpy spy2( &form2, SIGNAL( attributeChanged( QString, QVariant ) ) );

  // change value
  ww0->setValue( 2 ); // update col0
  QCOMPARE( spy2.count(), 2 );

  QCOMPARE( constraintsLabel( &form2, ww0 )->text(), inv ); // 2 < ( 1 + 2 )
  QCOMPARE( constraintsLabel( &form2, ww1 )->text(), QString() );
  QCOMPARE( constraintsLabel( &form2, ww2 )->text(), QString() );
  QCOMPARE( constraintsLabel( &form2, ww3 )->text(), val ); // 2 = 2

  // change value
  spy2.clear();
  ww0->setValue( 1 ); // update col0
  QCOMPARE( spy2.count(), 2 );

  QCOMPARE( constraintsLabel( &form2, ww0 )->text(), val ); // 1 < ( 1 + 2 )
  QCOMPARE( constraintsLabel( &form2, ww1 )->text(), QString() );
  QCOMPARE( constraintsLabel( &form2, ww2 )->text(), QString() );
  QCOMPARE( constraintsLabel( &form2, ww3 )->text(), inv ); // 2 = 1
}

void TestQgsAttributeForm::testOKButtonStatus()
{
  // make a temporary vector layer
  QString def = QStringLiteral( "Point?field=col0:integer" );
  QgsVectorLayer *layer = new QgsVectorLayer( def, QStringLiteral( "test" ), QStringLiteral( "memory" ) );

  // add a feature to the vector layer
  QgsFeature ft( layer->dataProvider()->fields(), 1 );
  ft.setAttribute( QStringLiteral( "col0" ), 0 );
  ft.setValid( true );

  // set constraint
  layer->setConstraintExpression( 0, QString() );

  // build a form for this feature
  QgsAttributeForm form( layer );
  form.setFeature( ft );

  QPushButton *okButton = form.mButtonBox->button( QDialogButtonBox::Ok );

  // get wrapper
  QgsEditorWidgetWrapper *ww = nullptr;
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[0] );

  // testing stuff
  QSignalSpy spy1( &form, SIGNAL( attributeChanged( QString, QVariant ) ) );
  QSignalSpy spy2( layer, SIGNAL( editingStarted() ) );
  QSignalSpy spy3( layer, SIGNAL( editingStopped() ) );

  // no constraint but layer not editable : OK button disabled
  QCOMPARE( layer->isEditable(), false );
  QCOMPARE( okButton->isEnabled(), false );

  // no constraint and editable layer : OK button enabled
  layer->startEditing();
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( layer->isEditable(), true );
  QCOMPARE( okButton->isEnabled(), true );

  // invalid constraint and editable layer : OK button disabled
  layer->setConstraintExpression( 0, QStringLiteral( "col0 = 0" ) );
  QgsAttributeForm form2( layer );
  form2.setFeature( ft );
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form2.mWidgets[0] );
  okButton = form2.mButtonBox->button( QDialogButtonBox::Ok );
  ww->setValue( 1 );
  QCOMPARE( okButton->isEnabled(), false );

  // valid constraint and editable layer : OK button enabled
  layer->setConstraintExpression( 0, QStringLiteral( "col0 = 2" ) );
  QgsAttributeForm form3( layer );
  form3.setFeature( ft );
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form3.mWidgets[0] );
  okButton = form3.mButtonBox->button( QDialogButtonBox::Ok );

  ww->setValue( 2 );
  QCOMPARE( okButton->isEnabled(), true );

  // valid constraint and not editable layer : OK button disabled
  layer->rollBack();
  QCOMPARE( spy3.count(), 1 );
  QCOMPARE( layer->isEditable(), false );
  QCOMPARE( okButton->isEnabled(), false );

  // set soft constraint
  layer->setFieldConstraint( 0, QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintStrengthSoft );
  QgsAttributeForm form4( layer );
  form4.setFeature( ft );
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form4.mWidgets[0] );
  okButton = form4.mButtonBox->button( QDialogButtonBox::Ok );
  ww->setValue( 1 );
  QVERIFY( !okButton->isEnabled() );
  layer->startEditing();
  // just a soft constraint, so OK should be enabled
  QVERIFY( okButton->isEnabled() );
  layer->rollBack();
  QVERIFY( !okButton->isEnabled() );
}

void TestQgsAttributeForm::testDynamicForm()
{
  // make temporary layers
  QString defA = QStringLiteral( "Point?field=id_a:integer" );
  QgsVectorLayer *layerA = new QgsVectorLayer( defA, QStringLiteral( "layerA" ), QStringLiteral( "memory" ) );

  QString defB = QStringLiteral( "Point?field=id_b:integer&field=col0:integer" );
  QgsVectorLayer *layerB = new QgsVectorLayer( defB, QStringLiteral( "layerB" ), QStringLiteral( "memory" ) );

  QString defC = QStringLiteral( "Point?field=id_c:integer&field=col0:integer" );
  QgsVectorLayer *layerC = new QgsVectorLayer( defC, QStringLiteral( "layerC" ), QStringLiteral( "memory" ) );

  // join configuration
  QgsVectorLayerJoinInfo infoJoinAB;
  infoJoinAB.setTargetFieldName( QStringLiteral( "id_a" ) );
  infoJoinAB.setJoinLayer( layerB );
  infoJoinAB.setJoinFieldName( QStringLiteral( "id_b" ) );
  infoJoinAB.setDynamicFormEnabled( true );

  layerA->addJoin( infoJoinAB );

  QgsVectorLayerJoinInfo infoJoinAC;
  infoJoinAC.setTargetFieldName( QStringLiteral( "id_a" ) );
  infoJoinAC.setJoinLayer( layerC );
  infoJoinAC.setJoinFieldName( QStringLiteral( "id_c" ) );
  infoJoinAC.setDynamicFormEnabled( true );

  layerA->addJoin( infoJoinAC );

  // add features for main layer
  QgsFeature ftA( layerA->fields() );
  ftA.setAttribute( QStringLiteral( "id_a" ), 0 );
  layerA->startEditing();
  layerA->addFeature( ftA );
  layerA->commitChanges();

  // add features for joined layers
  QgsFeature ft0B( layerB->fields() );
  ft0B.setAttribute( QStringLiteral( "id_b" ), 30 );
  ft0B.setAttribute( QStringLiteral( "col0" ), 10 );
  layerB->startEditing();
  layerB->addFeature( ft0B );
  layerB->commitChanges();

  QgsFeature ft1B( layerB->fields() );
  ft1B.setAttribute( QStringLiteral( "id_b" ), 31 );
  ft1B.setAttribute( QStringLiteral( "col0" ), 11 );
  layerB->startEditing();
  layerB->addFeature( ft1B );
  layerB->commitChanges();

  QgsFeature ft0C( layerC->fields() );
  ft0C.setAttribute( QStringLiteral( "id_c" ), 32 );
  ft0C.setAttribute( QStringLiteral( "col0" ), 12 );
  layerC->startEditing();
  layerC->addFeature( ft0C );
  layerC->commitChanges();

  QgsFeature ft1C( layerC->fields() );
  ft1C.setAttribute( QStringLiteral( "id_c" ), 31 );
  ft1C.setAttribute( QStringLiteral( "col0" ), 13 );
  layerC->startEditing();
  layerC->addFeature( ft1C );
  layerC->commitChanges();

  // build a form with feature A
  QgsAttributeForm form( layerA );
  form.setMode( QgsAttributeForm::AddFeatureMode );
  form.setFeature( ftA );

  // test that there's no joined feature by default
  QgsEditorWidgetWrapper *ww = nullptr;

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[1] );
  QCOMPARE( ww->field().name(), QString( "layerB_col0" ) );
  QCOMPARE( ww->value(), QVariant( QVariant::Int ) );

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[2] );
  QCOMPARE( ww->field().name(), QString( "layerC_col0" ) );
  QCOMPARE( ww->value(), QVariant( QVariant::Int ) );

  // change layerA join id field to join with layerB
  form.changeAttribute( QStringLiteral( "id_a" ), QVariant( 30 ) );

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[0] );
  QCOMPARE( ww->field().name(), QString( "id_a" ) );
  QCOMPARE( ww->value(), QVariant( 30 ) );

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[1] );
  QCOMPARE( ww->field().name(), QString( "layerB_col0" ) );
  QCOMPARE( ww->value(), QVariant( 10 ) );

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[2] );
  QCOMPARE( ww->field().name(), QString( "layerC_col0" ) );
  QCOMPARE( ww->value(), QVariant( QVariant::Int ) );

  // change layerA join id field to join with layerC
  form.changeAttribute( QStringLiteral( "id_a" ), QVariant( 32 ) );

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[0] );
  QCOMPARE( ww->field().name(), QString( "id_a" ) );
  QCOMPARE( ww->value(), QVariant( 32 ) );

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[1] );
  QCOMPARE( ww->field().name(), QString( "layerB_col0" ) );
  QCOMPARE( ww->value(), QVariant( QVariant::Int ) );

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[2] );
  QCOMPARE( ww->field().name(), QString( "layerC_col0" ) );
  QCOMPARE( ww->value(), QVariant( 12 ) );

  // change layerA join id field to join with layerA and layerC
  form.changeAttribute( QStringLiteral( "id_a" ), QVariant( 31 ) );

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[0] );
  QCOMPARE( ww->field().name(), QString( "id_a" ) );
  QCOMPARE( ww->value(), QVariant( 31 ) );

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[1] );
  QCOMPARE( ww->field().name(), QString( "layerB_col0" ) );
  QCOMPARE( ww->value(), QVariant( 11 ) );

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[2] );
  QCOMPARE( ww->field().name(), QString( "layerC_col0" ) );
  QCOMPARE( ww->value(), QVariant( 13 ) );

  // clean
  delete layerA;
  delete layerB;
  delete layerC;
}

void TestQgsAttributeForm::testConstraintsOnJoinedFields()
{
  QString validLabel = QStringLiteral( "<font color=\"#259B24\">%1</font>" ).arg( QChar( 0x2714 ) );
  QString warningLabel = QStringLiteral( "<font color=\"#FFC107\">%1</font>" ).arg( QChar( 0x2718 ) );

  // make temporary layers
  QString defA = QStringLiteral( "Point?field=id_a:integer" );
  QgsVectorLayer *layerA = new QgsVectorLayer( defA, QStringLiteral( "layerA" ), QStringLiteral( "memory" ) );

  QString defB = QStringLiteral( "Point?field=id_b:integer&field=col0:integer" );
  QgsVectorLayer *layerB = new QgsVectorLayer( defB, QStringLiteral( "layerB" ), QStringLiteral( "memory" ) );

  // set constraints on joined layer
  layerB->setConstraintExpression( 1, QStringLiteral( "col0 < 10" ) );
  layerB->setFieldConstraint( 1, QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintStrengthSoft );

  // join configuration
  QgsVectorLayerJoinInfo infoJoinAB;
  infoJoinAB.setTargetFieldName( QStringLiteral( "id_a" ) );
  infoJoinAB.setJoinLayer( layerB );
  infoJoinAB.setJoinFieldName( QStringLiteral( "id_b" ) );
  infoJoinAB.setDynamicFormEnabled( true );

  layerA->addJoin( infoJoinAB );

  // add features for main layer
  QgsFeature ftA( layerA->fields() );
  ftA.setAttribute( QStringLiteral( "id_a" ), 1 );
  layerA->startEditing();
  layerA->addFeature( ftA );
  layerA->commitChanges();

  // add features for joined layer
  QgsFeature ft0B( layerB->fields() );
  ft0B.setAttribute( QStringLiteral( "id_b" ), 30 );
  ft0B.setAttribute( QStringLiteral( "col0" ), 9 );
  layerB->startEditing();
  layerB->addFeature( ft0B );
  layerB->commitChanges();

  QgsFeature ft1B( layerB->fields() );
  ft1B.setAttribute( QStringLiteral( "id_b" ), 31 );
  ft1B.setAttribute( QStringLiteral( "col0" ), 11 );
  layerB->startEditing();
  layerB->addFeature( ft1B );
  layerB->commitChanges();

  // build a form for this feature
  QgsAttributeForm form( layerA );
  form.setMode( QgsAttributeForm::AddFeatureMode );
  form.setFeature( ftA );

  // change layerA join id field
  form.changeAttribute( QStringLiteral( "id_a" ), QVariant( 30 ) );

  // compare
  QgsEditorWidgetWrapper *ww = nullptr;
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[1] );
  QCOMPARE( constraintsLabel( &form, ww )->text(), validLabel );

  // change layerA join id field
  form.changeAttribute( QStringLiteral( "id_a" ), QVariant( 31 ) );

  // compare
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[1] );
  QCOMPARE( constraintsLabel( &form, ww )->text(), warningLabel );
}

void TestQgsAttributeForm::testEditableJoin()
{
  // make temporary layers
  QString defA = QStringLiteral( "Point?field=id_a:integer" );
  QgsVectorLayer *layerA = new QgsVectorLayer( defA, QStringLiteral( "layerA" ), QStringLiteral( "memory" ) );

  QString defB = QStringLiteral( "Point?field=id_b:integer&field=col0:integer" );
  QgsVectorLayer *layerB = new QgsVectorLayer( defB, QStringLiteral( "layerB" ), QStringLiteral( "memory" ) );

  QString defC = QStringLiteral( "Point?field=id_c:integer&field=col0:integer" );
  QgsVectorLayer *layerC = new QgsVectorLayer( defC, QStringLiteral( "layerC" ), QStringLiteral( "memory" ) );

  // join configuration
  QgsVectorLayerJoinInfo infoJoinAB;
  infoJoinAB.setTargetFieldName( QStringLiteral( "id_a" ) );
  infoJoinAB.setJoinLayer( layerB );
  infoJoinAB.setJoinFieldName( QStringLiteral( "id_b" ) );
  infoJoinAB.setDynamicFormEnabled( true );
  infoJoinAB.setEditable( true );

  layerA->addJoin( infoJoinAB );

  QgsVectorLayerJoinInfo infoJoinAC;
  infoJoinAC.setTargetFieldName( QStringLiteral( "id_a" ) );
  infoJoinAC.setJoinLayer( layerC );
  infoJoinAC.setJoinFieldName( QStringLiteral( "id_c" ) );
  infoJoinAC.setDynamicFormEnabled( true );
  infoJoinAC.setEditable( false );

  layerA->addJoin( infoJoinAC );

  // add features for main layer
  QgsFeature ftA( layerA->fields() );
  ftA.setAttribute( QStringLiteral( "id_a" ), 31 );
  layerA->startEditing();
  layerA->addFeature( ftA );
  layerA->commitChanges();

  // add features for joined layers
  QgsFeature ft0B( layerB->fields() );
  ft0B.setAttribute( QStringLiteral( "id_b" ), 31 );
  ft0B.setAttribute( QStringLiteral( "col0" ), 11 );
  layerB->startEditing();
  layerB->addFeature( ft0B );
  layerB->commitChanges();

  QgsFeature ft0C( layerC->fields() );
  ft0C.setAttribute( QStringLiteral( "id_c" ), 31 );
  ft0C.setAttribute( QStringLiteral( "col0" ), 13 );
  layerC->startEditing();
  layerC->addFeature( ft0C );
  layerC->commitChanges();

  // start editing layers
  layerA->startEditing();
  layerB->startEditing();
  layerC->startEditing();

  // build a form with feature A
  ftA = layerA->getFeature( 1 );

  QgsAttributeForm form( layerA );
  form.setMode( QgsAttributeForm::SingleEditMode );
  form.setFeature( ftA );

  // change layerA join id field to join with layerB and layerC
  QgsEditorWidgetWrapper *ww = nullptr;

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[0] );
  QCOMPARE( ww->field().name(), QString( "id_a" ) );
  QCOMPARE( ww->value(), QVariant( 31 ) );

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[1] );
  QCOMPARE( ww->field().name(), QString( "layerB_col0" ) );
  QCOMPARE( ww->value(), QVariant( 11 ) );

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[2] );
  QCOMPARE( ww->field().name(), QString( "layerC_col0" ) );
  QCOMPARE( ww->value(), QVariant( 13 ) );

  // test if widget is enabled for layerA
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[0] );
  QCOMPARE( ww->widget()->isEnabled(), true );

  // test if widget is enabled for layerB
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[1] );
  QCOMPARE( ww->widget()->isEnabled(), true );

  // test if widget is disabled for layerC
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[2] );
  QCOMPARE( ww->widget()->isEnabled(), false );

  // change attributes
  form.changeAttribute( QStringLiteral( "layerB_col0" ), QVariant( 333 ) );
  form.changeAttribute( QStringLiteral( "layerC_col0" ), QVariant( 444 ) );
  form.save();

  // commit changes
  layerA->commitChanges();
  layerB->commitChanges();
  layerC->commitChanges();

  // check attributes
  ft0B = layerB->getFeature( 1 );
  QCOMPARE( ft0B.attribute( "col0" ), QVariant( 333 ) );

  ft0C = layerC->getFeature( 1 );
  QCOMPARE( ft0C.attribute( "col0" ), QVariant( 13 ) );

  // clean
  delete layerA;
  delete layerB;
  delete layerC;
}

void TestQgsAttributeForm::testUpsertOnEdit()
{
  // make temporary layers
  QString defA = QStringLiteral( "Point?field=id_a:integer" );
  QgsVectorLayer *layerA = new QgsVectorLayer( defA, QStringLiteral( "layerA" ), QStringLiteral( "memory" ) );

  QString defB = QStringLiteral( "Point?field=id_b:integer&field=col0:integer" );
  QgsVectorLayer *layerB = new QgsVectorLayer( defB, QStringLiteral( "layerB" ), QStringLiteral( "memory" ) );

  QString defC = QStringLiteral( "Point?field=id_c:integer&field=col0:integer" );
  QgsVectorLayer *layerC = new QgsVectorLayer( defC, QStringLiteral( "layerC" ), QStringLiteral( "memory" ) );

  // join configuration
  QgsVectorLayerJoinInfo infoJoinAB;
  infoJoinAB.setTargetFieldName( QStringLiteral( "id_a" ) );
  infoJoinAB.setJoinLayer( layerB );
  infoJoinAB.setJoinFieldName( QStringLiteral( "id_b" ) );
  infoJoinAB.setDynamicFormEnabled( true );
  infoJoinAB.setEditable( true );
  infoJoinAB.setUpsertOnEdit( true );

  layerA->addJoin( infoJoinAB );

  QgsVectorLayerJoinInfo infoJoinAC;
  infoJoinAC.setTargetFieldName( QStringLiteral( "id_a" ) );
  infoJoinAC.setJoinLayer( layerC );
  infoJoinAC.setJoinFieldName( QStringLiteral( "id_c" ) );
  infoJoinAC.setDynamicFormEnabled( true );
  infoJoinAC.setEditable( true );
  infoJoinAC.setUpsertOnEdit( false );

  layerA->addJoin( infoJoinAC );

  // add features for main layer
  QgsFeature ft0A( layerA->fields() );
  ft0A.setAttribute( QStringLiteral( "id_a" ), 31 );
  layerA->startEditing();
  layerA->addFeature( ft0A );
  layerA->commitChanges();

  // add features for joined layers
  QgsFeature ft0B( layerB->fields() );
  ft0B.setAttribute( QStringLiteral( "id_b" ), 33 );
  ft0B.setAttribute( QStringLiteral( "col0" ), 11 );
  layerB->startEditing();
  layerB->addFeature( ft0B );
  layerB->commitChanges();

  QgsFeature ft0C( layerC->fields() );
  ft0C.setAttribute( QStringLiteral( "id_c" ), 31 );
  ft0C.setAttribute( QStringLiteral( "col0" ), 13 );
  layerC->startEditing();
  layerC->addFeature( ft0C );
  layerC->commitChanges();

  // get committed feature from layerA
  QgsFeature feature;
  QString filter = QgsExpression::createFieldEqualityExpression( QStringLiteral( "id_a" ), 31 );

  QgsFeatureRequest request;
  request.setFilterExpression( filter );
  request.setLimit( 1 );
  layerA->getFeatures( request ).nextFeature( ft0A );

  // start editing layers
  layerA->startEditing();
  layerB->startEditing();
  layerC->startEditing();

  // build a form with feature A
  QgsAttributeForm form( layerA );
  form.setMode( QgsAttributeForm::AddFeatureMode );
  form.setFeature( ft0A );

  // count features
  QCOMPARE( ( int )layerA->featureCount(), 1 );
  QCOMPARE( ( int )layerB->featureCount(), 1 );
  QCOMPARE( ( int )layerC->featureCount(), 1 );

  // add a new feature with null joined fields. Joined feature should not be
  // added
  form.changeAttribute( QStringLiteral( "id_a" ), QVariant( 32 ) );
  form.changeAttribute( QStringLiteral( "layerB_col0" ), QVariant() );
  form.changeAttribute( QStringLiteral( "layerC_col0" ), QVariant() );
  form.save();

  // commit
  layerA->commitChanges();
  layerB->commitChanges();
  layerC->commitChanges();

  // count features
  QCOMPARE( ( int )layerA->featureCount(), 2 );
  QCOMPARE( ( int )layerB->featureCount(), 1 );
  QCOMPARE( ( int )layerC->featureCount(), 1 );

  // start editing layers
  layerA->startEditing();
  layerB->startEditing();
  layerC->startEditing();

  // add a new feature with not null joined fields. Joined feature should be
  // added
  QgsAttributeForm form1( layerA );
  form1.setMode( QgsAttributeForm::AddFeatureMode );
  form1.setFeature( ft0A );

  form1.changeAttribute( QStringLiteral( "id_a" ), QVariant( 34 ) );
  form1.changeAttribute( QStringLiteral( "layerB_col0" ), QVariant( 3434 ) );
  form1.changeAttribute( QStringLiteral( "layerC_col0" ), QVariant( 343434 ) );
  form1.save();

  // commit
  layerA->commitChanges();
  layerB->commitChanges();
  layerC->commitChanges();

  // count features
  QCOMPARE( ( int )layerA->featureCount(), 3 );
  QCOMPARE( ( int )layerB->featureCount(), 2 );
  QCOMPARE( ( int )layerC->featureCount(), 1 );

  // check joined feature value
  filter = QgsExpression::createFieldEqualityExpression( QStringLiteral( "id_a" ), 34 );

  request.setFilterExpression( filter );
  request.setLimit( 1 );
  layerA->getFeatures( request ).nextFeature( feature );

  QCOMPARE( feature.attribute( "layerB_col0" ), QVariant( 3434 ) );

  // start editing layers
  layerA->startEditing();
  layerB->startEditing();
  layerC->startEditing();

  // create a target feature but update a joined feature. A new feature should
  // be added in layerA and values in layerB should be updated
  QgsAttributeForm form2( layerA );
  form2.setMode( QgsAttributeForm::AddFeatureMode );
  form2.setFeature( ft0A );
  form2.changeAttribute( QStringLiteral( "id_a" ), QVariant( 33 ) );
  form2.changeAttribute( QStringLiteral( "layerB_col0" ), QVariant( 3333 ) );
  form2.changeAttribute( QStringLiteral( "layerC_col0" ), QVariant( 323232 ) );
  form2.save();

  // commit
  layerA->commitChanges();
  layerB->commitChanges();
  layerC->commitChanges();

  // count features
  QCOMPARE( ( int )layerA->featureCount(), 4 );
  QCOMPARE( ( int )layerB->featureCount(), 2 );
  QCOMPARE( ( int )layerC->featureCount(), 1 );

  // check joined feature value
  filter = QgsExpression::createFieldEqualityExpression( QStringLiteral( "id_a" ), 33 );

  request.setFilterExpression( filter );
  request.setLimit( 1 );
  layerA->getFeatures( request ).nextFeature( feature );

  QCOMPARE( feature.attribute( "layerB_col0" ), QVariant( 3333 ) );

  // start editing layers
  layerA->startEditing();
  layerB->startEditing();
  layerC->startEditing();

  // update feature which does not exist in joined layer but with null joined
  // fields. A new feature should NOT be added in joined layer
  QgsAttributeForm form3( layerA );
  form3.setMode( QgsAttributeForm::SingleEditMode );
  form3.setFeature( ft0A );
  form3.changeAttribute( QStringLiteral( "id_a" ), QVariant( 31 ) );
  form3.changeAttribute( QStringLiteral( "layerB_col0" ), QVariant() );
  form3.changeAttribute( QStringLiteral( "layerC_col0" ), QVariant() );
  form3.save();

  // commit
  layerA->commitChanges();
  layerB->commitChanges();
  layerC->commitChanges();

  // count features
  QCOMPARE( ( int )layerA->featureCount(), 4 );
  QCOMPARE( ( int )layerB->featureCount(), 2 );
  QCOMPARE( ( int )layerC->featureCount(), 1 );

  // start editing layers
  layerA->startEditing();
  layerB->startEditing();
  layerC->startEditing();

  // update feature which does not exist in joined layer with NOT null joined
  // fields. A new feature should be added in joined layer
  QgsAttributeForm form4( layerA );
  form4.setMode( QgsAttributeForm::SingleEditMode );
  form4.setFeature( ft0A );
  form4.changeAttribute( QStringLiteral( "id_a" ), QVariant( 31 ) );
  form4.changeAttribute( QStringLiteral( "layerB_col0" ), QVariant( 1111 ) );
  form4.changeAttribute( QStringLiteral( "layerC_col0" ), QVariant( 3131 ) );
  form4.save();

  // commit
  layerA->commitChanges();
  layerB->commitChanges();
  layerC->commitChanges();

  // count features
  QCOMPARE( ( int )layerA->featureCount(), 4 );
  QCOMPARE( ( int )layerB->featureCount(), 3 );
  QCOMPARE( ( int )layerC->featureCount(), 1 );

  // check joined feature value
  filter = QgsExpression::createFieldEqualityExpression( QStringLiteral( "id_a" ), 31 );

  request.setFilterExpression( filter );
  request.setLimit( 1 );
  layerA->getFeatures( request ).nextFeature( feature );

  QCOMPARE( feature.attribute( "layerB_col0" ), QVariant( 1111 ) );

  // clean
  delete layerA;
  delete layerB;
  delete layerC;
}


QGSTEST_MAIN( TestQgsAttributeForm )
#include "testqgsattributeform.moc"
