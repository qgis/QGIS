/***************************************************************************
    testqgsqmlwidgetwrapper.cpp
     --------------------------------------
    Date                 : October 2026
    Copyright            : (C) 2026 Mathieu Pellerin
    Email                : mathieu at opengis dot ch
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
#include "qgsattributeform.h"
#include "qgsfeature.h"
#include "qgsgui.h"
#include "qgsqmlwidgetwrapper.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QQuickItem>
#include <QQuickWidget>
#include <QString>

using namespace Qt::StringLiterals;

class TestQgsQmlWidgetWrapper : public QObject
{
    Q_OBJECT
  public:
    TestQgsQmlWidgetWrapper() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testAttributeFormBridge();
};

void TestQgsQmlWidgetWrapper::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsGui::editorWidgetRegistry()->initEditors();
}

void TestQgsQmlWidgetWrapper::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsQmlWidgetWrapper::init()
{}

void TestQgsQmlWidgetWrapper::cleanup()
{}

void TestQgsQmlWidgetWrapper::testAttributeFormBridge()
{
  auto layer = std::make_unique<QgsVectorLayer>( u"Point?crs=EPSG:4326&field=str_field:string"_s, u"test_layer"_s, u"memory"_s );

  QgsFeature feature( layer->fields() );
  feature.setAttribute( u"str_field"_s, u"str_value"_s );
  feature.setGeometry( QgsGeometry::fromWkt( u"POINT(1 1)"_s ) );

  layer->dataProvider()->addFeature( feature );

  QgsAttributeForm form( layer.get() );
  form.setFeature( feature );

  QgsQmlWidgetWrapper *qmlWrapper = new QgsQmlWidgetWrapper( layer.get(), nullptr, &form );
  QQuickWidget *qmlWidget = qobject_cast<QQuickWidget *>( qmlWrapper->widget() );
  QVERIFY( qmlWidget );

  qmlWrapper->setContext( form.mContext );
  qmlWrapper->setQmlCode( QStringLiteral(
    "import QtQuick\n"
    "Item {\n"
    "  width: parent ? parent.width : 400\n"
    "  height: 400\n"
    "\n"
    "  property bool t_editable: attributeFormBridge.editable\n"
    "  property var t_layerName: attributeFormBridge.layer ? attributeFormBridge.layer.name : \"\"\n"
    "  property var t_bindingAttribute: attributeFormBridge.currentFormFeature.attribute(\"str_field\")\n"
    "  property var t_staticAttribute: attributeFormBridge.attribute(\"str_field\")\n"
    "\n"
    "  function t_changeAttribute() { attributeFormBridge.changeAttribute(\"str_field\", \"str_changed_value_again\"); }\n"
    "  function t_changeGeometry() { attributeFormBridge.changeGeometry(attributeFormBridge.currentFormFeature.geometry.fromWkt(\"POINT(2 2 )\")); }\n"
    "}\n"
  ) );
  qmlWrapper->reinitWidget();

  const QList<QQmlError> errors = qmlWidget->errors();
  QCOMPARE( errors.count(), 0 );

  QQuickItem *rootObject = qmlWidget->rootObject();
  QVERIFY( rootObject );

  QCOMPARE( rootObject->property( "t_editable" ).toBool(), false );
  QCOMPARE( rootObject->property( "t_layerName" ).toString(), u"test_layer"_s );
  QCOMPARE( rootObject->property( "t_bindingAttribute" ).toString(), u"str_value"_s );
  QCOMPARE( rootObject->property( "t_staticAttribute" ).toString(), u"str_value"_s );

  layer->startEditing();
  QCOMPARE( rootObject->property( "t_editable" ).toBool(), true );

  form.changeAttribute( u"str_field"_s, u"str_changed_value"_s );
  QCOMPARE( rootObject->property( "t_bindingAttribute" ).toString(), u"str_changed_value"_s );
  QCOMPARE( rootObject->property( "t_staticAttribute" ).toString(), u"str_value"_s );

  QMetaObject::invokeMethod( rootObject, "t_changeAttribute" );
  QCOMPARE( form.currentFormFeature().attribute( u"str_field"_s ), u"str_changed_value_again"_s );
  QCOMPARE( rootObject->property( "t_bindingAttribute" ).toString(), u"str_changed_value_again"_s );
  QCOMPARE( rootObject->property( "t_staticAttribute" ).toString(), u"str_value"_s );

  QMetaObject::invokeMethod( rootObject, "t_changeGeometry" );
  QCOMPARE( form.currentFormFeature().geometry().asWkt(), u"Point (2 2)"_s );

  layer->commitChanges();
  QCOMPARE( rootObject->property( "t_editable" ).toBool(), false );
}

QGSTEST_MAIN( TestQgsQmlWidgetWrapper )
#include "testqgsqmlwidgetwrapper.moc"
