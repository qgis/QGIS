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


#include "editorwidgets/core/qgseditorwidgetregistry.h"
#include "qgsapplication.h"
#include "qgsattributeeditorcontainer.h"
#include "qgsattributeeditorfield.h"
#include "qgsattributeform.h"
#include "qgsattributeformeditorwidget.h"
#include "qgsattributeforminterface.h"
#include "qgseditorwidgetwrapper.h"
#include "qgsfeature.h"
#include "qgsgui.h"
#include "qgsmultiedittoolbutton.h"
#include "qgsspinbox.h"
#include "qgstest.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerjoininfo.h"

#include <QLineEdit>
#include <QPushButton>
#include <QSignalSpy>

class TestQgsAttributeForm : public QObject
{
    Q_OBJECT
  public:
    TestQgsAttributeForm() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testFieldConstraint();
    void testFieldMultiConstraints();
    void testOKButtonStatus();
    void testDynamicForm();
    void testConstraintsOnJoinedFields();
    void testEditableJoin();
    void testUpsertOnEdit();
    void testFixAttributeForm();
    void testAttributeFormInterface();
    void testDefaultValueUpdate();
    void testDefaultValueUpdateRecursion();
    void testParentFeatureUpdate();
    void testSameFieldSync();
    void testZeroDoubles();
    void testMinimumWidth();
    void testFieldConstraintDuplicateField();
    void testCaseInsensitiveFieldConstraint();

  private:
    QLabel *constraintsLabel( QgsAttributeForm *form, QgsEditorWidgetWrapper *ww )
    {
      QgsAttributeFormEditorWidget *formEditorWidget = form->mFormEditorWidgets.value( ww->fieldIdx() );
      return formEditorWidget->findChild<QLabel *>( u"ConstraintStatus"_s );
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
  const QString def = u"Point?field=col0:integer"_s;
  QgsVectorLayer *layer = new QgsVectorLayer( def, u"test"_s, u"memory"_s );
  layer->setEditorWidgetSetup( 0, QgsEditorWidgetSetup( u"TextEdit"_s, QVariantMap() ) );

  // add a feature to the vector layer
  QgsFeature ft( layer->dataProvider()->fields(), 1 );
  ft.setAttribute( u"col0"_s, 0 );

  // toggle start editing to show constraint labels
  layer->startEditing();

  // build a form for this feature
  QgsAttributeForm form( layer );
  form.setFeature( ft );

  // testing stuff
  const QString validLabel = u"<font color=\"#259B24\">%1</font>"_s.arg( QChar( 0x2714 ) );
  const QString invalidLabel = u"<font color=\"#FF9800\">%1</font>"_s.arg( QChar( 0x2718 ) );
  const QString warningLabel = u"<font color=\"#FFC107\">%1</font>"_s.arg( QChar( 0x2718 ) );

  // set constraint
  layer->setConstraintExpression( 0, QString() );

  // get wrapper
  QgsEditorWidgetWrapper *ww = nullptr;
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[0] );

  // no constraint so we expect no label
  QVERIFY( !constraintsLabel( &form, ww ) );

  // set a not null constraint
  layer->setConstraintExpression( 0, u"col0 is not null"_s );
  // build a form for this feature
  QgsAttributeForm form2( layer );
  form2.setFeature( ft );
  QSignalSpy spy( &form2, SIGNAL( widgetValueChanged( QString, QVariant, bool ) ) );
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form2.mWidgets[0] );

  // set value to 1
  ww->setValues( 1, QVariantList() );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( constraintsLabel( &form2, ww )->text(), validLabel );

  // set value to null
  spy.clear();
  ww->setValues( QVariant(), QVariantList() );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( constraintsLabel( &form2, ww )->text(), invalidLabel );

  // set value to 1
  spy.clear();
  ww->setValues( 1, QVariantList() );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( constraintsLabel( &form2, ww )->text(), validLabel );

  // set a soft constraint
  layer->setConstraintExpression( 0, u"col0 is not null"_s );
  layer->setFieldConstraint( 0, QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintStrengthSoft );
  // build a form for this feature
  QgsAttributeForm form3( layer );
  form3.setFeature( ft );
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form3.mWidgets[0] );

  // set value to 1
  ww->setValues( 1, QVariantList() );
  QCOMPARE( constraintsLabel( &form3, ww )->text(), validLabel );

  // set value to null
  ww->setValues( QVariant(), QVariantList() );
  QCOMPARE( constraintsLabel( &form3, ww )->text(), warningLabel );

  // set value to 1
  ww->setValues( 1, QVariantList() );
  QCOMPARE( constraintsLabel( &form3, ww )->text(), validLabel );
}

void TestQgsAttributeForm::testFieldMultiConstraints()
{
  // make a temporary layer to check through
  const QString def = u"Point?field=col0:integer&field=col1:integer&field=col2:integer&field=col3:integer"_s;
  QgsVectorLayer *layer = new QgsVectorLayer( def, u"test"_s, u"memory"_s );

  // add features to the vector layer
  QgsFeature ft( layer->dataProvider()->fields(), 1 );
  ft.setAttribute( u"col0"_s, 0 );
  ft.setAttribute( u"col1"_s, 1 );
  ft.setAttribute( u"col2"_s, 2 );
  ft.setAttribute( u"col3"_s, 3 );

  // set constraints for each field
  layer->setConstraintExpression( 0, QString() );
  layer->setConstraintExpression( 1, QString() );
  layer->setConstraintExpression( 2, QString() );
  layer->setConstraintExpression( 3, QString() );

  // toggle start editing to show constraint labels
  layer->startEditing();

  // build a form for this feature
  QgsAttributeForm form( layer );
  form.setFeature( ft );

  // testing stuff
  const QSignalSpy spy( &form, SIGNAL( attributeChanged( QString, QVariant ) ) );
  const QString val = u"<font color=\"#259B24\">%1</font>"_s.arg( QChar( 0x2714 ) );
  const QString inv = u"<font color=\"#FF9800\">%1</font>"_s.arg( QChar( 0x2718 ) );

  // get wrappers for each widget
  QgsEditorWidgetWrapper *ww0, *ww1, *ww2, *ww3;
  ww0 = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[0] );
  ww1 = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[1] );
  ww2 = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[2] );
  ww3 = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[3] );

  // no constraint so we expect an empty label
  QVERIFY( !constraintsLabel( &form, ww0 ) );
  QVERIFY( !constraintsLabel( &form, ww1 ) );
  QVERIFY( !constraintsLabel( &form, ww2 ) );
  QVERIFY( !constraintsLabel( &form, ww3 ) );

  // update constraint
  layer->setConstraintExpression( 0, u"col0 < (col1 * col2)"_s );
  layer->setConstraintExpression( 1, QString() );
  layer->setConstraintExpression( 2, QString() );
  layer->setConstraintExpression( 3, u"col0 = 2"_s );

  QgsAttributeForm form2( layer );
  form2.setFeature( ft );
  ww0 = qobject_cast<QgsEditorWidgetWrapper *>( form2.mWidgets[0] );
  ww1 = qobject_cast<QgsEditorWidgetWrapper *>( form2.mWidgets[1] );
  ww2 = qobject_cast<QgsEditorWidgetWrapper *>( form2.mWidgets[2] );
  ww3 = qobject_cast<QgsEditorWidgetWrapper *>( form2.mWidgets[3] );
  QSignalSpy spy2( &form2, SIGNAL( widgetValueChanged( QString, QVariant, bool ) ) );

  // change value
  ww0->setValues( 2, QVariantList() ); // update col0
  QCOMPARE( spy2.count(), 1 );

  QCOMPARE( constraintsLabel( &form2, ww0 )->text(), inv ); // 2 < ( 1 + 2 )
  QVERIFY( !constraintsLabel( &form2, ww1 ) );
  QVERIFY( !constraintsLabel( &form2, ww2 ) );
  QCOMPARE( constraintsLabel( &form2, ww3 )->text(), val ); // 2 = 2

  // change value
  spy2.clear();
  ww0->setValues( 1, QVariantList() ); // update col0
  QCOMPARE( spy2.count(), 1 );

  QCOMPARE( constraintsLabel( &form2, ww0 )->text(), val ); // 1 < ( 1 + 2 )
  QVERIFY( !constraintsLabel( &form2, ww1 ) );
  QVERIFY( !constraintsLabel( &form2, ww2 ) );
  QCOMPARE( constraintsLabel( &form2, ww3 )->text(), inv ); // 2 = 1
}

void TestQgsAttributeForm::testOKButtonStatus()
{
  // make a temporary vector layer
  const QString def = u"Point?field=col0:integer"_s;
  QgsVectorLayer *layer = new QgsVectorLayer( def, u"test"_s, u"memory"_s );

  // add a feature to the vector layer
  QgsFeature ft( layer->dataProvider()->fields(), 1 );
  ft.setAttribute( u"col0"_s, 0 );
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
  const QSignalSpy spy1( &form, SIGNAL( attributeChanged( QString, QVariant ) ) );
  const QSignalSpy spy2( layer, SIGNAL( editingStarted() ) );
  const QSignalSpy spy3( layer, SIGNAL( editingStopped() ) );

  // no constraint but layer not editable : OK button disabled
  QCOMPARE( layer->isEditable(), false );
  QCOMPARE( okButton->isEnabled(), false );

  // no constraint and editable layer : OK button enabled
  layer->startEditing();
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( layer->isEditable(), true );
  QCOMPARE( okButton->isEnabled(), true );

  // invalid constraint and editable layer : OK button disabled
  layer->setConstraintExpression( 0, u"col0 = 0"_s );
  QgsAttributeForm form2( layer );
  form2.setFeature( ft );
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form2.mWidgets[0] );
  okButton = form2.mButtonBox->button( QDialogButtonBox::Ok );
  ww->setValues( 1, QVariantList() );
  QCOMPARE( okButton->isEnabled(), false );

  // valid constraint and editable layer : OK button enabled
  layer->setConstraintExpression( 0, u"col0 = 2"_s );
  QgsAttributeForm form3( layer );
  form3.setFeature( ft );
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form3.mWidgets[0] );
  okButton = form3.mButtonBox->button( QDialogButtonBox::Ok );

  ww->setValues( 2, QVariantList() );
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
  ww->setValues( 1, QVariantList() );
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
  const QString defA = u"Point?field=id_a:integer"_s;
  QgsVectorLayer *layerA = new QgsVectorLayer( defA, u"layerA"_s, u"memory"_s );

  const QString defB = u"Point?field=id_b:integer&field=col0:integer"_s;
  QgsVectorLayer *layerB = new QgsVectorLayer( defB, u"layerB"_s, u"memory"_s );

  const QString defC = u"Point?field=id_c:integer&field=col0:integer"_s;
  QgsVectorLayer *layerC = new QgsVectorLayer( defC, u"layerC"_s, u"memory"_s );

  // join configuration
  QgsVectorLayerJoinInfo infoJoinAB;
  infoJoinAB.setTargetFieldName( u"id_a"_s );
  infoJoinAB.setJoinLayer( layerB );
  infoJoinAB.setJoinFieldName( u"id_b"_s );
  infoJoinAB.setDynamicFormEnabled( true );

  layerA->addJoin( infoJoinAB );

  QgsVectorLayerJoinInfo infoJoinAC;
  infoJoinAC.setTargetFieldName( u"id_a"_s );
  infoJoinAC.setJoinLayer( layerC );
  infoJoinAC.setJoinFieldName( u"id_c"_s );
  infoJoinAC.setDynamicFormEnabled( true );

  layerA->addJoin( infoJoinAC );

  // add features for main layer
  QgsFeature ftA( layerA->fields() );
  ftA.setAttribute( u"id_a"_s, 0 );
  layerA->startEditing();
  layerA->addFeature( ftA );
  layerA->commitChanges();

  // add features for joined layers
  QgsFeature ft0B( layerB->fields() );
  ft0B.setAttribute( u"id_b"_s, 30 );
  ft0B.setAttribute( u"col0"_s, 10 );
  layerB->startEditing();
  layerB->addFeature( ft0B );
  layerB->commitChanges();

  QgsFeature ft1B( layerB->fields() );
  ft1B.setAttribute( u"id_b"_s, 31 );
  ft1B.setAttribute( u"col0"_s, 11 );
  layerB->startEditing();
  layerB->addFeature( ft1B );
  layerB->commitChanges();

  QgsFeature ft0C( layerC->fields() );
  ft0C.setAttribute( u"id_c"_s, 32 );
  ft0C.setAttribute( u"col0"_s, 12 );
  layerC->startEditing();
  layerC->addFeature( ft0C );
  layerC->commitChanges();

  QgsFeature ft1C( layerC->fields() );
  ft1C.setAttribute( u"id_c"_s, 31 );
  ft1C.setAttribute( u"col0"_s, 13 );
  layerC->startEditing();
  layerC->addFeature( ft1C );
  layerC->commitChanges();

  // build a form with feature A
  QgsAttributeForm form( layerA );
  form.setMode( QgsAttributeEditorContext::AddFeatureMode );
  form.setFeature( ftA );

  // test that there's no joined feature by default
  QgsEditorWidgetWrapper *ww = nullptr;

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[1] );
  QCOMPARE( ww->field().name(), QString( "layerB_col0" ) );
  QCOMPARE( ww->value(), QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) );

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[2] );
  QCOMPARE( ww->field().name(), QString( "layerC_col0" ) );
  QCOMPARE( ww->value(), QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) );

  // change layerA join id field to join with layerB
  form.changeAttribute( u"id_a"_s, QVariant( 30 ) );

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[0] );
  QCOMPARE( ww->field().name(), QString( "id_a" ) );
  QCOMPARE( ww->value(), QVariant( 30 ) );

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[1] );
  QCOMPARE( ww->field().name(), QString( "layerB_col0" ) );
  QCOMPARE( ww->value(), QVariant( 10 ) );

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[2] );
  QCOMPARE( ww->field().name(), QString( "layerC_col0" ) );
  QCOMPARE( ww->value(), QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) );

  // change layerA join id field to join with layerC
  form.changeAttribute( u"id_a"_s, QVariant( 32 ) );

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[0] );
  QCOMPARE( ww->field().name(), QString( "id_a" ) );
  QCOMPARE( ww->value(), QVariant( 32 ) );

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[1] );
  QCOMPARE( ww->field().name(), QString( "layerB_col0" ) );
  QCOMPARE( ww->value(), QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) );

  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[2] );
  QCOMPARE( ww->field().name(), QString( "layerC_col0" ) );
  QCOMPARE( ww->value(), QVariant( 12 ) );

  // change layerA join id field to join with layerA and layerC
  form.changeAttribute( u"id_a"_s, QVariant( 31 ) );

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
  const QString validLabel = u"<font color=\"#259B24\">%1</font>"_s.arg( QChar( 0x2714 ) );
  const QString warningLabel = u"<font color=\"#FFC107\">%1</font>"_s.arg( QChar( 0x2718 ) );

  // make temporary layers
  const QString defA = u"Point?field=id_a:integer"_s;
  QgsVectorLayer *layerA = new QgsVectorLayer( defA, u"layerA"_s, u"memory"_s );

  const QString defB = u"Point?field=id_b:integer&field=col0:integer"_s;
  QgsVectorLayer *layerB = new QgsVectorLayer( defB, u"layerB"_s, u"memory"_s );

  // set constraints on joined layer
  layerB->setConstraintExpression( 1, u"col0 < 10"_s );
  layerB->setFieldConstraint( 1, QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintStrengthSoft );

  // join configuration
  QgsVectorLayerJoinInfo infoJoinAB;
  infoJoinAB.setTargetFieldName( u"id_a"_s );
  infoJoinAB.setJoinLayer( layerB );
  infoJoinAB.setJoinFieldName( u"id_b"_s );
  infoJoinAB.setDynamicFormEnabled( true );

  layerA->addJoin( infoJoinAB );

  // add features for main layer
  QgsFeature ftA( layerA->fields() );
  ftA.setAttribute( u"id_a"_s, 1 );
  layerA->startEditing();
  layerA->addFeature( ftA );
  layerA->commitChanges();

  // add features for joined layer
  QgsFeature ft0B( layerB->fields() );
  ft0B.setAttribute( u"id_b"_s, 30 );
  ft0B.setAttribute( u"col0"_s, 9 );
  layerB->startEditing();
  layerB->addFeature( ft0B );
  layerB->commitChanges();

  QgsFeature ft1B( layerB->fields() );
  ft1B.setAttribute( u"id_b"_s, 31 );
  ft1B.setAttribute( u"col0"_s, 11 );
  layerB->startEditing();
  layerB->addFeature( ft1B );
  layerB->commitChanges();

  // toggle start editing to show constraint labels
  layerA->startEditing();
  layerB->startEditing();

  // build a form for this feature
  QgsAttributeForm form( layerA );
  form.setMode( QgsAttributeEditorContext::AddFeatureMode );
  form.setFeature( ftA );

  // change layerA join id field
  form.changeAttribute( u"id_a"_s, QVariant( 30 ) );

  // compare
  QgsEditorWidgetWrapper *ww = nullptr;
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[1] );
  QCOMPARE( constraintsLabel( &form, ww )->text(), validLabel );

  // change layerA join id field
  form.changeAttribute( u"id_a"_s, QVariant( 31 ) );

  // compare
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[1] );
  QCOMPARE( constraintsLabel( &form, ww )->text(), warningLabel );
}

void TestQgsAttributeForm::testEditableJoin()
{
  // make temporary layers
  const QString defA = u"Point?field=id_a:integer"_s;
  QgsVectorLayer *layerA = new QgsVectorLayer( defA, u"layerA"_s, u"memory"_s );

  const QString defB = u"Point?field=id_b:integer&field=col0:integer"_s;
  QgsVectorLayer *layerB = new QgsVectorLayer( defB, u"layerB"_s, u"memory"_s );

  const QString defC = u"Point?field=id_c:integer&field=col0:integer"_s;
  QgsVectorLayer *layerC = new QgsVectorLayer( defC, u"layerC"_s, u"memory"_s );

  // join configuration
  QgsVectorLayerJoinInfo infoJoinAB;
  infoJoinAB.setTargetFieldName( u"id_a"_s );
  infoJoinAB.setJoinLayer( layerB );
  infoJoinAB.setJoinFieldName( u"id_b"_s );
  infoJoinAB.setDynamicFormEnabled( true );
  infoJoinAB.setEditable( true );

  layerA->addJoin( infoJoinAB );

  QgsVectorLayerJoinInfo infoJoinAC;
  infoJoinAC.setTargetFieldName( u"id_a"_s );
  infoJoinAC.setJoinLayer( layerC );
  infoJoinAC.setJoinFieldName( u"id_c"_s );
  infoJoinAC.setDynamicFormEnabled( true );
  infoJoinAC.setEditable( false );

  layerA->addJoin( infoJoinAC );

  // add features for main layer
  QgsFeature ftA( layerA->fields() );
  ftA.setAttribute( u"id_a"_s, 31 );
  layerA->startEditing();
  layerA->addFeature( ftA );
  layerA->commitChanges();

  // add features for joined layers
  QgsFeature ft0B( layerB->fields() );
  ft0B.setAttribute( u"id_b"_s, 31 );
  ft0B.setAttribute( u"col0"_s, 11 );
  layerB->startEditing();
  layerB->addFeature( ft0B );
  layerB->commitChanges();

  QgsFeature ft0C( layerC->fields() );
  ft0C.setAttribute( u"id_c"_s, 31 );
  ft0C.setAttribute( u"col0"_s, 13 );
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
  form.setMode( QgsAttributeEditorContext::SingleEditMode );
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
  QCOMPARE( qobject_cast<QgsSpinBox *>( ww->widget() )->isReadOnly(), false );

  // test if widget is enabled for layerB
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[1] );
  QCOMPARE( qobject_cast<QgsSpinBox *>( ww->widget() )->isReadOnly(), false );

  // test if widget is disabled for layerC
  ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[2] );
  QCOMPARE( qobject_cast<QgsSpinBox *>( ww->widget() )->isReadOnly(), true );

  // change attributes
  form.changeAttribute( u"layerB_col0"_s, QVariant( 333 ) );
  form.changeAttribute( u"layerC_col0"_s, QVariant( 444 ) );
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

  // all editor widget must have a multi edit button
  layerA->startEditing();
  layerB->startEditing();
  layerC->startEditing();
  layerA->select( ftA.id() );
  form.setMode( QgsAttributeEditorContext::MultiEditMode );

  // multi edit button must be displayed for A
  QgsAttributeFormEditorWidget *formWidget = qobject_cast<QgsAttributeFormEditorWidget *>( form.mFormWidgets[1] );
  QVERIFY( formWidget->mMultiEditButton->parent() );

  // multi edit button must be displayed for B (join is editable)
  formWidget = qobject_cast<QgsAttributeFormEditorWidget *>( form.mFormWidgets[1] );
  QVERIFY( formWidget->mMultiEditButton->parent() );

  // multi edit button must not be displayed for C (join is not editable)
  formWidget = qobject_cast<QgsAttributeFormEditorWidget *>( form.mFormWidgets[2] );
  QVERIFY( !formWidget->mMultiEditButton->parent() );

  // clean
  delete layerA;
  delete layerB;
  delete layerC;
}

void TestQgsAttributeForm::testUpsertOnEdit()
{
  // make temporary layers
  const QString defA = u"Point?field=id_a:integer"_s;
  QgsVectorLayer *layerA = new QgsVectorLayer( defA, u"layerA"_s, u"memory"_s );

  const QString defB = u"Point?field=id_b:integer&field=col0:integer"_s;
  QgsVectorLayer *layerB = new QgsVectorLayer( defB, u"layerB"_s, u"memory"_s );

  const QString defC = u"Point?field=id_c:integer&field=col0:integer"_s;
  QgsVectorLayer *layerC = new QgsVectorLayer( defC, u"layerC"_s, u"memory"_s );

  // join configuration
  QgsVectorLayerJoinInfo infoJoinAB;
  infoJoinAB.setTargetFieldName( u"id_a"_s );
  infoJoinAB.setJoinLayer( layerB );
  infoJoinAB.setJoinFieldName( u"id_b"_s );
  infoJoinAB.setDynamicFormEnabled( true );
  infoJoinAB.setEditable( true );
  infoJoinAB.setUpsertOnEdit( true );

  layerA->addJoin( infoJoinAB );

  QgsVectorLayerJoinInfo infoJoinAC;
  infoJoinAC.setTargetFieldName( u"id_a"_s );
  infoJoinAC.setJoinLayer( layerC );
  infoJoinAC.setJoinFieldName( u"id_c"_s );
  infoJoinAC.setDynamicFormEnabled( true );
  infoJoinAC.setEditable( true );
  infoJoinAC.setUpsertOnEdit( false );

  layerA->addJoin( infoJoinAC );

  // add features for main layer
  QgsFeature ft0A( layerA->fields() );
  ft0A.setAttribute( u"id_a"_s, 31 );
  layerA->startEditing();
  layerA->addFeature( ft0A );
  layerA->commitChanges();

  // add features for joined layers
  QgsFeature ft0B( layerB->fields() );
  ft0B.setAttribute( u"id_b"_s, 33 );
  ft0B.setAttribute( u"col0"_s, 11 );
  layerB->startEditing();
  layerB->addFeature( ft0B );
  layerB->commitChanges();

  QgsFeature ft0C( layerC->fields() );
  ft0C.setAttribute( u"id_c"_s, 31 );
  ft0C.setAttribute( u"col0"_s, 13 );
  layerC->startEditing();
  layerC->addFeature( ft0C );
  layerC->commitChanges();

  // get committed feature from layerA
  QgsFeature feature;
  QString filter = QgsExpression::createFieldEqualityExpression( u"id_a"_s, 31 );

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
  form.setMode( QgsAttributeEditorContext::AddFeatureMode );
  form.setFeature( ft0A );

  // count features
  QCOMPARE( ( int ) layerA->featureCount(), 1 );
  QCOMPARE( ( int ) layerB->featureCount(), 1 );
  QCOMPARE( ( int ) layerC->featureCount(), 1 );

  // add a new feature with null joined fields. Joined feature should not be
  // added
  form.changeAttribute( u"id_a"_s, QVariant( 32 ) );
  form.changeAttribute( u"layerB_col0"_s, QVariant() );
  form.changeAttribute( u"layerC_col0"_s, QVariant() );
  form.save();

  // commit
  layerA->commitChanges();
  layerB->commitChanges();
  layerC->commitChanges();

  // count features
  QCOMPARE( ( int ) layerA->featureCount(), 2 );
  QCOMPARE( ( int ) layerB->featureCount(), 1 );
  QCOMPARE( ( int ) layerC->featureCount(), 1 );

  // start editing layers
  layerA->startEditing();
  layerB->startEditing();
  layerC->startEditing();

  // add a new feature with not null joined fields. Joined feature should be
  // added
  QgsAttributeForm form1( layerA );
  form1.setMode( QgsAttributeEditorContext::AddFeatureMode );
  form1.setFeature( ft0A );

  form1.changeAttribute( u"id_a"_s, QVariant( 34 ) );
  form1.changeAttribute( u"layerB_col0"_s, QVariant( 3434 ) );
  form1.changeAttribute( u"layerC_col0"_s, QVariant( 343434 ) );
  form1.save();

  // commit
  layerA->commitChanges();
  layerB->commitChanges();
  layerC->commitChanges();

  // count features
  QCOMPARE( ( int ) layerA->featureCount(), 3 );
  QCOMPARE( ( int ) layerB->featureCount(), 2 );
  QCOMPARE( ( int ) layerC->featureCount(), 1 );

  // check joined feature value
  filter = QgsExpression::createFieldEqualityExpression( u"id_a"_s, 34 );

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
  form2.setMode( QgsAttributeEditorContext::AddFeatureMode );
  form2.setFeature( ft0A );
  form2.changeAttribute( u"id_a"_s, QVariant( 33 ) );
  form2.changeAttribute( u"layerB_col0"_s, QVariant( 3333 ) );
  form2.changeAttribute( u"layerC_col0"_s, QVariant( 323232 ) );
  form2.save();

  // commit
  layerA->commitChanges();
  layerB->commitChanges();
  layerC->commitChanges();

  // count features
  QCOMPARE( ( int ) layerA->featureCount(), 4 );
  QCOMPARE( ( int ) layerB->featureCount(), 2 );
  QCOMPARE( ( int ) layerC->featureCount(), 1 );

  // check joined feature value
  filter = QgsExpression::createFieldEqualityExpression( u"id_a"_s, 33 );

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
  form3.setMode( QgsAttributeEditorContext::SingleEditMode );
  form3.setFeature( ft0A );
  form3.changeAttribute( u"id_a"_s, QVariant( 31 ) );
  form3.changeAttribute( u"layerB_col0"_s, QVariant() );
  form3.changeAttribute( u"layerC_col0"_s, QVariant() );
  form3.save();

  // commit
  layerA->commitChanges();
  layerB->commitChanges();
  layerC->commitChanges();

  // count features
  QCOMPARE( ( int ) layerA->featureCount(), 4 );
  QCOMPARE( ( int ) layerB->featureCount(), 2 );
  QCOMPARE( ( int ) layerC->featureCount(), 1 );

  // start editing layers
  layerA->startEditing();
  layerB->startEditing();
  layerC->startEditing();

  // update feature which does not exist in joined layer with NOT null joined
  // fields. A new feature should be added in joined layer
  QgsAttributeForm form4( layerA );
  form4.setMode( QgsAttributeEditorContext::SingleEditMode );
  form4.setFeature( ft0A );
  form4.changeAttribute( u"id_a"_s, QVariant( 31 ) );
  form4.changeAttribute( u"layerB_col0"_s, QVariant( 1111 ) );
  form4.changeAttribute( u"layerC_col0"_s, QVariant( 3131 ) );
  form4.save();

  // commit
  layerA->commitChanges();
  layerB->commitChanges();
  layerC->commitChanges();

  // count features
  QCOMPARE( ( int ) layerA->featureCount(), 4 );
  QCOMPARE( ( int ) layerB->featureCount(), 3 );
  QCOMPARE( ( int ) layerC->featureCount(), 1 );

  // check joined feature value
  filter = QgsExpression::createFieldEqualityExpression( u"id_a"_s, 31 );

  request.setFilterExpression( filter );
  request.setLimit( 1 );
  layerA->getFeatures( request ).nextFeature( feature );

  QCOMPARE( feature.attribute( "layerB_col0" ), QVariant( 1111 ) );

  // clean
  delete layerA;
  delete layerB;
  delete layerC;
}

void TestQgsAttributeForm::testFixAttributeForm()
{
  const QString def = u"Point?field=id:integer&field=col1:integer"_s;
  QgsVectorLayer *layer = new QgsVectorLayer( def, u"layer"_s, u"memory"_s );

  QVERIFY( layer );

  QgsFeature f( layer->fields() );
  f.setAttribute( 0, 1 );
  f.setAttribute( 1, 681 );

  QgsAttributeForm form( layer );

  form.setMode( QgsAttributeEditorContext::FixAttributeMode );
  form.setFeature( f );

  QgsEditorWidgetWrapper *ww = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[1] );
  QCOMPARE( ww->field().name(), QString( "col1" ) );
  QCOMPARE( ww->value(), QVariant( 681 ) );

  // now change the value
  ww->setValue( QVariant( 630 ) );

  // the value should be updated
  QCOMPARE( ww->value(), QVariant( 630 ) );
  // the feature is not saved yet, so contains the old value
  QCOMPARE( form.feature().attribute( u"col1"_s ), QVariant( 681 ) );
  // now save the feature and enjoy its new value, but don't update the layer
  QVERIFY( form.save() );
  QCOMPARE( form.feature().attribute( u"col1"_s ), QVariant( 630 ) );
  QCOMPARE( ( int ) layer->featureCount(), 0 );

  delete layer;
}

void TestQgsAttributeForm::testAttributeFormInterface()
{
  // Issue https://github.com/qgis/QGIS/issues/29667
  // we simulate a python code execution that would be triggered
  // at form opening and that would modify the value of a widget.
  // We want to check that emitted signal widgetValueChanged is
  // correctly emitted with correct parameters

  // make a temporary vector layer
  const QString def = u"Point?field=col0:integer"_s;
  QgsVectorLayer *layer = new QgsVectorLayer( def, u"test"_s, u"memory"_s );
  layer->setEditorWidgetSetup( 0, QgsEditorWidgetSetup( u"TextEdit"_s, QVariantMap() ) );

  // add a feature to the vector layer
  QgsFeature ft( layer->dataProvider()->fields(), 1 );
  ft.setAttribute( u"col0"_s, 10 );

  class MyInterface : public QgsAttributeFormInterface
  {
    public:
      MyInterface( QgsAttributeForm *form )
        : QgsAttributeFormInterface( form ) {}

      void featureChanged() override
      {
        QgsAttributeForm *f = form();
        QLineEdit *le = f->findChild<QLineEdit *>( "col0" );
        le->setText( "100" );
      }
  };

  // build a form for this feature
  QgsAttributeForm form( layer );
  form.addInterface( new MyInterface( &form ) );

  bool set = false;
  connect( &form, &QgsAttributeForm::widgetValueChanged, this, [&set]( const QString &attribute, const QVariant &newValue, bool attributeChanged ) {
    // Check that our value set by the QgsAttributeFormInterface has correct parameters.
    // attributeChanged has to be true because it won't be taken into account by others
    // (QgsValueRelationWidgetWrapper for instance)
    if ( attribute == "col0" && newValue.toInt() == 100 && attributeChanged )
      set = true;
  } );

  form.setFeature( ft );
  QVERIFY( set );
}


void TestQgsAttributeForm::testDefaultValueUpdate()
{
  // make a temporary layer to check through
  const QString def = u"Point?field=col0:integer&field=col1:integer&field=col2:integer&field=col3:integer"_s;
  QgsVectorLayer *layer = new QgsVectorLayer( def, u"test"_s, u"memory"_s );

  //set defaultValueDefinitions
  //col0 - no default value
  //col1 - "col0"+1
  //col2 - "col0"+"col1"
  //col3 - "col2"

  // set constraints for each field
  layer->setDefaultValueDefinition( 1, QgsDefaultValue( u"\"col0\"+1"_s, true ) );
  layer->setDefaultValueDefinition( 2, QgsDefaultValue( u"\"col0\"+\"col1\""_s, true ) );
  layer->setDefaultValueDefinition( 3, QgsDefaultValue( u"\"col2\""_s, true ) );

  layer->startEditing();

  // build a form for this feature
  QgsFeature ft( layer->dataProvider()->fields(), 1 );
  ft.setAttribute( u"col0"_s, 0 );
  QgsAttributeForm form( layer );
  form.setMode( QgsAttributeEditorContext::AddFeatureMode );
  form.setFeature( ft );

  // get wrappers for each widget
  QgsEditorWidgetWrapper *ww0, *ww1, *ww2, *ww3;
  ww0 = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[0] );
  ww1 = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[1] );
  ww2 = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[2] );
  ww3 = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[3] );

  //set value in col0:
  ww0->setValue( 5 );

  //we expect
  //col0 - 5
  //col1 - 6
  //col2 - 11
  //col3 - 11

  QCOMPARE( ww0->value().toInt(), 5 );
  QCOMPARE( ww1->value().toInt(), 6 );
  QCOMPARE( ww2->value().toInt(), 11 );
  QCOMPARE( ww3->value().toInt(), 11 );

  //set value in col1:
  ww1->setValue( 10 );

  //we expect
  //col0 - 5
  //col1 - 10
  //col2 - 15
  //col3 - 15

  QCOMPARE( ww0->value().toInt(), 5 );
  QCOMPARE( ww1->value().toInt(), 10 );
  QCOMPARE( ww2->value().toInt(), 15 );
  QCOMPARE( ww3->value().toInt(), 15 );
}

void TestQgsAttributeForm::testDefaultValueUpdateRecursion()
{
  // make a temporary layer to check through
  const QString def = u"Point?field=col0:integer&field=col1:integer&field=col2:integer&field=col3:integer"_s;
  QgsVectorLayer *layer = new QgsVectorLayer( def, u"test"_s, u"memory"_s );

  //let's make a recursion
  //col0 - COALESCE( 0, "col3"+1)
  //col1 - COALESCE( 0, "col0"+1)
  //col2 - COALESCE( 0, "col1"+1)
  //col3 - COALESCE( 0, "col2"+1)

  // set constraints for each field
  layer->setDefaultValueDefinition( 0, QgsDefaultValue( u"\"col3\"+1"_s, true ) );
  layer->setDefaultValueDefinition( 1, QgsDefaultValue( u"\"col0\"+1"_s, true ) );
  layer->setDefaultValueDefinition( 2, QgsDefaultValue( u"\"col1\"+1"_s, true ) );
  layer->setDefaultValueDefinition( 3, QgsDefaultValue( u"\"col2\"+1"_s, true ) );

  layer->startEditing();

  // build a form for this feature
  QgsFeature ft( layer->dataProvider()->fields(), 1 );
  ft.setAttribute( u"col0"_s, 0 );
  QgsAttributeForm form( layer );
  form.setMode( QgsAttributeEditorContext::AddFeatureMode );
  form.setFeature( ft );

  // get wrappers for each widget
  QgsEditorWidgetWrapper *ww0, *ww1, *ww2, *ww3;
  ww0 = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[0] );
  ww1 = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[1] );
  ww2 = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[2] );
  ww3 = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[3] );

  //set value in col0:
  ww0->setValue( 20 );

  //we expect
  //col0 - 20
  //col1 - 21
  //col2 - 22
  //col3 - 23

  QCOMPARE( ww0->value().toInt(), 20 );
  QCOMPARE( ww1->value().toInt(), 21 );
  QCOMPARE( ww2->value().toInt(), 22 );
  QCOMPARE( ww3->value().toInt(), 23 );

  //set value in col2:
  ww2->setValue( 30 );

  //we expect
  //col0 - 32
  //col1 - 33
  //col2 - 30
  //col3 - 31

  QCOMPARE( ww0->value().toInt(), 32 );
  QCOMPARE( ww1->value().toInt(), 33 );
  QCOMPARE( ww2->value().toInt(), 30 );
  QCOMPARE( ww3->value().toInt(), 31 );

  //set value in col0 again:
  ww0->setValue( 40 );

  //we expect
  //col0 - 40
  //col1 - 41
  //col2 - 42
  //col3 - 43

  QCOMPARE( ww0->value().toInt(), 40 );
  QCOMPARE( ww1->value().toInt(), 41 );
  QCOMPARE( ww2->value().toInt(), 42 );
  QCOMPARE( ww3->value().toInt(), 43 );
}

void TestQgsAttributeForm::testParentFeatureUpdate()
{
  // make a temporary layer to check through
  const QString def = u"Point?field=col0:integer"_s;
  QgsVectorLayer *layer = new QgsVectorLayer( def, u"test"_s, u"memory"_s );
  layer->setDefaultValueDefinition( 0, QgsDefaultValue( u"current_parent_value('colZero\')"_s, true ) );
  layer->startEditing();

  // initialize parent feature
  QgsFields parentFields;
  parentFields.append( QgsField( u"colZero"_s, QMetaType::Type::Int ) );

  QgsFeature parentFeature( parentFields, 1 );
  parentFeature.setAttribute( u"colZero"_s, 10 );

  // initialize child feature
  QgsFeature feature( layer->dataProvider()->fields(), 1 );

  // build a form
  QgsAttributeEditorContext context;
  context.setParentFormFeature( parentFeature );
  QgsAttributeForm form( layer, feature, context );

  // get wrappers for each widget
  QgsEditorWidgetWrapper *ww0;
  ww0 = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[0] );

  form.parentFormValueChanged( u"colZero"_s, 20 );

  QCOMPARE( ww0->value().toInt(), 20 );
}

void TestQgsAttributeForm::testSameFieldSync()
{
  // Check that widget synchronisation works when a form contains the same field several times
  // and there is no issues when editing

  // make a temporary vector layer
  const QString def = u"Point?field=col0:integer"_s;
  QgsVectorLayer *layer = new QgsVectorLayer( def, u"test"_s, u"memory"_s );
  layer->setEditorWidgetSetup( 0, QgsEditorWidgetSetup( u"TextEdit"_s, QVariantMap() ) );

  // add a feature to the vector layer
  QgsFeature ft( layer->dataProvider()->fields(), 1 );
  ft.setAttribute( u"col0"_s, 10 );

  // add same field twice so they get synced
  QgsEditFormConfig editFormConfig = layer->editFormConfig();
  editFormConfig.clearTabs();
  editFormConfig.addTab( new QgsAttributeEditorField( "col0", 0, editFormConfig.invisibleRootContainer() ) );
  editFormConfig.addTab( new QgsAttributeEditorField( "col0", 0, editFormConfig.invisibleRootContainer() ) );
  editFormConfig.setLayout( Qgis::AttributeFormLayout::DragAndDrop );
  layer->setEditFormConfig( editFormConfig );

  layer->startEditing();

  // build a form for this feature
  QgsAttributeForm form( layer );
  form.setFeature( ft );

  QList<QLineEdit *> les = form.findChildren<QLineEdit *>( "col0" );
  QCOMPARE( les.count(), 2 );

  les[0]->setCursorPosition( 1 );
  QTest::keyClick( les[0], Qt::Key_2 );
  QTest::keyClick( les[0], Qt::Key_3 );

  QCOMPARE( les[0]->text(), QString( "1230" ) );
  QCOMPARE( les[0]->cursorPosition(), 3 );
  QCOMPARE( les[1]->text(), QString( "1230" ) );
  QCOMPARE( les[1]->cursorPosition(), 4 );
}

void TestQgsAttributeForm::testZeroDoubles()
{
  // See issue GH #34118
  const QString def = u"Point?field=col0:double"_s;
  QgsVectorLayer layer { def, u"test"_s, u"memory"_s };
  layer.setEditorWidgetSetup( 0, QgsEditorWidgetSetup( u"TextEdit"_s, QVariantMap() ) );
  QgsFeature ft( layer.dataProvider()->fields(), 1 );
  ft.setAttribute( u"col0"_s, 0.0 );
  QgsAttributeForm form( &layer );
  form.setFeature( ft );
  const QList<QLineEdit *> les = form.findChildren<QLineEdit *>( "col0" );
  QCOMPARE( les.count(), 1 );
  QCOMPARE( les.at( 0 )->text(), u"0"_s );
}

void TestQgsAttributeForm::testMinimumWidth()
{
  // ensure that the minimum width of editor widgets is as small as possible for the actual attribute form mode.
  const QString def = u"Point?field=col0:double"_s;
  QgsVectorLayer layer { def, u"test"_s, u"memory"_s };
  layer.setEditorWidgetSetup( 0, QgsEditorWidgetSetup( u"TextEdit"_s, QVariantMap() ) );
  QgsFeature ft( layer.dataProvider()->fields(), 1 );
  ft.setAttribute( u"col0"_s, 0.0 );
  QgsAttributeEditorContext context;
  context.setAttributeFormMode( QgsAttributeEditorContext::SingleEditMode );
  auto form = std::make_unique<QgsAttributeForm>( &layer, QgsFeature(), context );
  form->setFeature( ft );
  form->show();
  // we don't want the larger width requirement of the search wrappers to be enforced when the attribute form
  // is not in search modes
  QLineEdit le;
  const QFontMetrics leMetrics( le.fontMetrics() );
  QGSVERIFYLESSTHAN( form->minimumWidth(), leMetrics.horizontalAdvance( 'x' ) * 20 );

  form->setMode( QgsAttributeEditorContext::SearchMode );
  QGSVERIFYLESSTHAN( form->minimumWidth(), leMetrics.horizontalAdvance( 'x' ) * 150 );

  context.setAttributeFormMode( QgsAttributeEditorContext::AddFeatureMode );
  form = std::make_unique<QgsAttributeForm>( &layer, QgsFeature(), context );
  form->setFeature( ft );
  form->show();
  form->setMode( QgsAttributeEditorContext::AddFeatureMode );
  QGSVERIFYLESSTHAN( form->minimumWidth(), leMetrics.horizontalAdvance( 'x' ) * 20 );

  context.setAttributeFormMode( QgsAttributeEditorContext::AggregateSearchMode );
  form = std::make_unique<QgsAttributeForm>( &layer, QgsFeature(), context );
  form->setFeature( ft );
  form->show();
  form->setMode( QgsAttributeEditorContext::AggregateSearchMode );
  QGSVERIFYLESSTHAN( form->minimumWidth(), leMetrics.horizontalAdvance( 'x' ) * 150 );

  context.setAttributeFormMode( QgsAttributeEditorContext::MultiEditMode );
  form = std::make_unique<QgsAttributeForm>( &layer, QgsFeature(), context );
  form->setFeature( ft );
  form->setMode( QgsAttributeEditorContext::MultiEditMode );
  form->show();
  QGSVERIFYLESSTHAN( form->minimumWidth(), leMetrics.horizontalAdvance( 'x' ) * 100 );
}

void TestQgsAttributeForm::testFieldConstraintDuplicateField()
{
  // make a temporary vector layer
  const QString def = u"Point?field=col0:integer"_s;
  QgsVectorLayer *layer = new QgsVectorLayer( def, u"test"_s, u"memory"_s );
  layer->setEditorWidgetSetup( 0, QgsEditorWidgetSetup( u"Range"_s, QVariantMap() ) );

  // add same field twice so they get synced
  QgsEditFormConfig editFormConfig = layer->editFormConfig();
  editFormConfig.clearTabs();
  editFormConfig.invisibleRootContainer()->addChildElement( new QgsAttributeEditorField( "col0", 0, editFormConfig.invisibleRootContainer() ) );
  editFormConfig.invisibleRootContainer()->addChildElement( new QgsAttributeEditorField( "col0", 0, editFormConfig.invisibleRootContainer() ) );
  editFormConfig.setLayout( Qgis::AttributeFormLayout::DragAndDrop );
  layer->setEditFormConfig( editFormConfig );

  // add a feature to the vector layer
  QgsFeature ft( layer->dataProvider()->fields(), 1 );
  ft.setAttribute( u"col0"_s, 1 );

  // set a not null constraint
  layer->setConstraintExpression( 0, u"col0 > 10"_s );

  // build a form for this feature
  QgsAttributeForm form( layer );
  form.setFeature( ft );

  const QList<QgsAttributeFormEditorWidget *> formEditorWidgets = form.mFormEditorWidgets.values( 0 );
  QCOMPARE( formEditorWidgets[0]->editorWidget()->constraintResult(), QgsEditorWidgetWrapper::ConstraintResultFailHard );
  QCOMPARE( formEditorWidgets[1]->editorWidget()->constraintResult(), QgsEditorWidgetWrapper::ConstraintResultFailHard );

  formEditorWidgets[0]->editorWidget()->setValues( 20, QVariantList() );
  QCOMPARE( formEditorWidgets[0]->editorWidget()->constraintResult(), QgsEditorWidgetWrapper::ConstraintResultPass );
  QCOMPARE( formEditorWidgets[1]->editorWidget()->constraintResult(), QgsEditorWidgetWrapper::ConstraintResultPass );
}

void TestQgsAttributeForm::testCaseInsensitiveFieldConstraint()
{
  auto layer = std::make_unique<QgsVectorLayer>( u"Point?field=f1:integer&field=f2:integer&field=f3:integer"_s, u"test"_s, u"memory"_s );

  layer->setEditorWidgetSetup( 0, QgsEditorWidgetSetup( u"TextEdit"_s, QVariantMap() ) );
  layer->setEditorWidgetSetup( 1, QgsEditorWidgetSetup( u"TextEdit"_s, QVariantMap() ) );
  layer->setEditorWidgetSetup( 2, QgsEditorWidgetSetup( u"TextEdit"_s, QVariantMap() ) );

  // the expressions only differ by case
  layer->setConstraintExpression( 0, QStringLiteral( R"exp("f3" > 0)exp" ) );
  layer->setConstraintExpression( 1, QStringLiteral( R"exp("F3" > 0)exp" ) );

  QgsAttributeForm form( layer.get() );

  QVERIFY( form.mWidgets.size() == 3 );

  auto depsF3 = form.constraintDependencies( qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[2] ) );
  QCOMPARE( depsF3.size(), 2 );
}

QGSTEST_MAIN( TestQgsAttributeForm )
#include "testqgsattributeform.moc"
