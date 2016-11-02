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


#include <QtTest/QtTest>
#include <QPushButton>

#include <editorwidgets/core/qgseditorwidgetregistry.h>
#include "qgsattributeform.h"
#include <qgsapplication.h>
#include "qgseditorwidgetwrapper.h"
#include <qgsvectorlayer.h>
#include "qgsvectordataprovider.h"
#include <qgsfeature.h>

class TestQgsAttributeForm : public QObject
{
    Q_OBJECT
  public:
    TestQgsAttributeForm() {}

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testFieldConstraint();
    void testFieldMultiConstraints();
    void testOKButtonStatus();
};

void TestQgsAttributeForm::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsEditorWidgetRegistry::initEditors();
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
  QgsVectorLayer* layer = new QgsVectorLayer( def, QStringLiteral( "test" ), QStringLiteral( "memory" ) );
  layer->editFormConfig().setWidgetType( QStringLiteral( "col0" ), QStringLiteral( "TextEdit" ) );

  // add a feature to the vector layer
  QgsFeature ft( layer->dataProvider()->fields(), 1 );
  ft.setAttribute( QStringLiteral( "col0" ), 0 );

  // build a form for this feature
  QgsAttributeForm form( layer );
  form.setFeature( ft );

  // testing stuff
  QString validLabel = QStringLiteral( "col0<font color=\"green\">✔</font>" );
  QString invalidLabel = QStringLiteral( "col0<font color=\"red\">✘</font>" );
  QString warningLabel = QStringLiteral( "col0<font color=\"orange\">✘</font>" );

  // set constraint
  layer->setConstraintExpression( 0, QString() );

  // get wrapper
  QgsEditorWidgetWrapper *ww;
  ww = qobject_cast<QgsEditorWidgetWrapper*>( form.mWidgets[0] );

  // no constraint so we expect a label with just the field name
  QLabel *label = form.mBuddyMap.value( ww->widget() );
  QCOMPARE( label->text(), QString( "col0" ) );

  // set a not null constraint
  layer->setConstraintExpression( 0, QStringLiteral( "col0 is not null" ) );
  // build a form for this feature
  QgsAttributeForm form2( layer );
  form2.setFeature( ft );
  QSignalSpy spy( &form2, SIGNAL( attributeChanged( QString, QVariant ) ) );
  ww = qobject_cast<QgsEditorWidgetWrapper*>( form2.mWidgets[0] );
  label = form2.mBuddyMap.value( ww->widget() );

  // set value to 1
  ww->setValue( 1 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( label->text(), validLabel );

  // set value to null
  spy.clear();
  ww->setValue( QVariant() );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( label->text(), invalidLabel );

  // set value to 1
  spy.clear();
  ww->setValue( 1 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( label->text(), validLabel );

  // set a soft constraint
  layer->setConstraintExpression( 0, QStringLiteral( "col0 is not null" ) );
  layer->setFieldConstraint( 0, QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintStrengthSoft );
  // build a form for this feature
  QgsAttributeForm form3( layer );
  form3.setFeature( ft );
  ww = qobject_cast<QgsEditorWidgetWrapper*>( form3.mWidgets[0] );
  label = form3.mBuddyMap.value( ww->widget() );

  // set value to 1
  ww->setValue( 1 );
  QCOMPARE( label->text(), validLabel );

  // set value to null
  ww->setValue( QVariant() );
  QCOMPARE( label->text(), warningLabel );

  // set value to 1
  ww->setValue( 1 );
  QCOMPARE( label->text(), validLabel );
}

void TestQgsAttributeForm::testFieldMultiConstraints()
{
  // make a temporary layer to check through
  QString def = QStringLiteral( "Point?field=col0:integer&field=col1:integer&field=col2:integer&field=col3:integer" );
  QgsVectorLayer* layer = new QgsVectorLayer( def, QStringLiteral( "test" ), QStringLiteral( "memory" ) );

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
  QString val = QStringLiteral( "<font color=\"green\">✔</font>" );
  QString inv = QStringLiteral( "<font color=\"red\">✘</font>" );

  // get wrappers for each widget
  QgsEditorWidgetWrapper *ww0, *ww1, *ww2, *ww3;
  ww0 = qobject_cast<QgsEditorWidgetWrapper*>( form.mWidgets[0] );
  ww1 = qobject_cast<QgsEditorWidgetWrapper*>( form.mWidgets[1] );
  ww2 = qobject_cast<QgsEditorWidgetWrapper*>( form.mWidgets[2] );
  ww3 = qobject_cast<QgsEditorWidgetWrapper*>( form.mWidgets[3] );

  // get label for wrappers
  QLabel *label0 = form.mBuddyMap.value( ww0->widget() );
  QLabel *label1 = form.mBuddyMap.value( ww1->widget() );
  QLabel *label2 = form.mBuddyMap.value( ww2->widget() );
  QLabel *label3 = form.mBuddyMap.value( ww3->widget() );

  // no constraint so we expect a label with just the field name
  QCOMPARE( label0->text(), QString( "col0" ) );
  QCOMPARE( label1->text(), QString( "col1" ) );
  QCOMPARE( label2->text(), QString( "col2" ) );
  QCOMPARE( label3->text(), QString( "col3" ) );

  // update constraint
  layer->setConstraintExpression( 0, QStringLiteral( "col0 < (col1 * col2)" ) );
  layer->setConstraintExpression( 1, QString() );
  layer->setConstraintExpression( 2, QString() );
  layer->setConstraintExpression( 3, QStringLiteral( "col0 = 2" ) );

  QgsAttributeForm form2( layer );
  form2.setFeature( ft );
  ww0 = qobject_cast<QgsEditorWidgetWrapper*>( form2.mWidgets[0] );
  ww1 = qobject_cast<QgsEditorWidgetWrapper*>( form2.mWidgets[1] );
  ww2 = qobject_cast<QgsEditorWidgetWrapper*>( form2.mWidgets[2] );
  ww3 = qobject_cast<QgsEditorWidgetWrapper*>( form2.mWidgets[3] );
  label0 = form2.mBuddyMap.value( ww0->widget() );
  label1 = form2.mBuddyMap.value( ww1->widget() );
  label2 = form2.mBuddyMap.value( ww2->widget() );
  label3 = form2.mBuddyMap.value( ww3->widget() );
  QSignalSpy spy2( &form2, SIGNAL( attributeChanged( QString, QVariant ) ) );

  // change value
  ww0->setValue( 2 ); // update col0
  QCOMPARE( spy2.count(), 2 );

  QCOMPARE( label0->text(), QString( "col0" + inv ) ); // 2 < ( 1 + 2 )
  QCOMPARE( label1->text(), QString( "col1" ) );
  QCOMPARE( label2->text(), QString( "col2" ) );
  QCOMPARE( label3->text(), QString( "col3" + val ) ); // 2 = 2

  // change value
  spy2.clear();
  ww0->setValue( 1 ); // update col0
  QCOMPARE( spy2.count(), 2 );

  QCOMPARE( label0->text(), QString( "col0" + val ) ); // 1 < ( 1 + 2 )
  QCOMPARE( label1->text(), QString( "col1" ) );
  QCOMPARE( label2->text(), QString( "col2" ) );
  QCOMPARE( label3->text(), QString( "col3" + inv ) ); // 2 = 1
}

void TestQgsAttributeForm::testOKButtonStatus()
{
  // make a temporary vector layer
  QString def = QStringLiteral( "Point?field=col0:integer" );
  QgsVectorLayer* layer = new QgsVectorLayer( def, QStringLiteral( "test" ), QStringLiteral( "memory" ) );

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
  QgsEditorWidgetWrapper *ww;
  ww = qobject_cast<QgsEditorWidgetWrapper*>( form.mWidgets[0] );

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
  ww = qobject_cast<QgsEditorWidgetWrapper*>( form2.mWidgets[0] );
  okButton = form2.mButtonBox->button( QDialogButtonBox::Ok );
  ww->setValue( 1 );
  QCOMPARE( okButton->isEnabled(), false );

  // valid constraint and editable layer : OK button enabled
  layer->setConstraintExpression( 0, QStringLiteral( "col0 = 2" ) );
  QgsAttributeForm form3( layer );
  form3.setFeature( ft );
  ww = qobject_cast<QgsEditorWidgetWrapper*>( form3.mWidgets[0] );
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
  ww = qobject_cast<QgsEditorWidgetWrapper*>( form4.mWidgets[0] );
  okButton = form4.mButtonBox->button( QDialogButtonBox::Ok );
  ww->setValue( 1 );
  QVERIFY( !okButton->isEnabled() );
  layer->startEditing();
  // just a soft constraint, so OK should be enabled
  QVERIFY( okButton->isEnabled() );
  layer->rollBack();
  QVERIFY( !okButton->isEnabled() );
}

QTEST_MAIN( TestQgsAttributeForm )
#include "testqgsattributeform.moc"
