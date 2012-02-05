/***************************************************************************
     test_template.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:23 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest>
#include <QDomDocument>
#include <QFile>
//header for class being tested
#include <qgsrulebasedrendererv2.h>

#if QT_VERSION < 0x40701
// See http://hub.qgis.org/issues/4284
Q_DECLARE_METATYPE( QVariant )
#endif


class TestQgsRuleBasedRenderer: public QObject
{
    Q_OBJECT
  private slots:

  void test_load_xml()
  {
    QDomDocument doc;
    xml2domElement( "rulebasedrenderer_simple.xml", doc );
    QDomElement elem = doc.documentElement();

    QgsRuleBasedRendererV2* r = static_cast<QgsRuleBasedRendererV2*>( QgsRuleBasedRendererV2::create( elem ) );
    QVERIFY( r );
    check_tree_valid( r->rootRule() );
    delete r;
  }

  void test_load_invalid_xml()
  {
    QDomDocument doc;
    xml2domElement( "rulebasedrenderer_invalid.xml", doc );
    QDomElement elem = doc.documentElement();

    QgsRuleBasedRendererV2* r = static_cast<QgsRuleBasedRendererV2*>( QgsRuleBasedRendererV2::create( elem ) );
    QVERIFY( r == NULL );
  }

private:
  void xml2domElement( QString testFile, QDomDocument& doc )
  {
    QString fileName = QString( TEST_DATA_DIR ) + QDir::separator() + testFile;
    QFile f(fileName);
    bool fileOpen = f.open(QIODevice::ReadOnly);
    QVERIFY( fileOpen );

    QString msg;
    int line, col;
    bool parse = doc.setContent( &f, &msg, &line, &col );
    QVERIFY( parse );
  }

  void check_tree_valid( QgsRuleBasedRendererV2::Rule* root )
  {
    // root must always exist (although it does not have children)
    QVERIFY( root );
    // and does not have a parent
    QVERIFY( root->parent() == NULL );

    foreach ( QgsRuleBasedRendererV2::Rule* node, root->children() )
      check_non_root_rule( node );
  }

  void check_non_root_rule( QgsRuleBasedRendererV2::Rule* node )
  {
    qDebug() << node->dump();
    // children must not be NULL
    QVERIFY( node );
    // and must have a parent
    QVERIFY( node->parent() );
    // check that all children are okay
    foreach ( QgsRuleBasedRendererV2::Rule* child, node->children() )
      check_non_root_rule( child );
  }

};

QTEST_MAIN( TestQgsRuleBasedRenderer )

#include "moc_testqgsrulebasedrenderer.cxx"

