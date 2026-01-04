/***************************************************************************
                         TestQgsCompositionConverter.cpp
                         -----------------
    begin                : December 2017
    copyright            : (C) 2017 by Alessandro Pasotti
    email                : elpaso at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscompositionconverter.h"
#include "qgsfontutils.h"
#include "qgslayoutatlas.h"
#include "qgslayoutitemattributetable.h"
#include "qgslayoutitemgroup.h"
#include "qgslayoutitemhtml.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemlegend.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitempicture.h"
#include "qgslayoutitempolygon.h"
#include "qgslayoutitempolyline.h"
#include "qgslayoutitemscalebar.h"
#include "qgslayoutitemshape.h"
#include "qgslayoutmanager.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutrendercontext.h"
#include "qgsprintlayout.h"
#include "qgsproject.h"
#include "qgsreadwritecontext.h"
#include "qgstest.h"

#include <QDebug>

// Debug output for dom nodes
QDebug operator<<( QDebug dbg, const QDomNode &node )
{
  QString s;
  QTextStream str( &s, QIODevice::WriteOnly );
  node.save( str, 2 );
  dbg << qPrintable( s );
  return dbg;
}

class TestQgsCompositionConverter : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsCompositionConverter()
      : QgsTest( u"Composition Converter Tests"_s, u"compositionconverter"_s ) {}

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void init();         // will be called before each testfunction is executed.
    void cleanup();      // will be called after every testfunction.


    /**
     * Test import legend from a composer template
     */
    void importComposerTemplateLegend();

    /**
     * Test import attribute table from a composer template
     */
    void importComposerTemplateAttributeTable();

    /**
     * Test import label from a composer template
     */
    void importComposerTemplateLabel();

    /**
     * Test import shape from a composer template
     */
    void importComposerTemplateShape();

    /**
     * Test import pictures from a composer template
     */
    void importComposerTemplatePicture();

    /**
     * Test import polygon from a composer template
     */
    void importComposerTemplatePolygon();

    /**
     * Test import polyline from a composer template
     */
    void importComposerTemplatePolyline();

    /**
     * Test import arrow from a composer template
     */
    void importComposerTemplateArrow();

    /**
     * Test import map from a composer template
     */
    void importComposerTemplateMap();

    /**
     * Test import scalebar from a composer template
     */
    void importComposerTemplateScaleBar();

    /**
     * Test import group from a composer template
     */
    void importComposerTemplateGroup();

    /**
     * Test import multiple elements from a composer template
     */
    void importComposerTemplate();

    /**
     * Test import atlas from a composer template
     */
    void importComposerAtlas();

    /**
     * Test automatic conversion from a composer template
     */
    void convertComposition();

    /**
     * Test if a composition template can be detected from a dom document
     */
    void isCompositionTemplate();

    /**
     * Test if a composition template can be converted to a layout template
     */
    void convertCompositionTemplate();


  private:
    QSize renderedPageSize( QgsLayout *layout, int pageNumber = 0 );

    QDomElement loadComposer( const QString &name );
};

void TestQgsCompositionConverter::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::settingsSearchPathsForSVG->setValue( QStringList() << QStringLiteral( TEST_DATA_DIR ) );
  QgsFontUtils::loadStandardTestFonts( { u"Bold"_s, u"Oblique"_s, u"Roman"_s } );
}

void TestQgsCompositionConverter::init()
{
}

void TestQgsCompositionConverter::cleanup()
{
}


void TestQgsCompositionConverter::importComposerTemplateLabel()
{
  QDomElement composerElem( loadComposer( u"2x_template_label.qpt"_s ) );
  QgsProject project;
  project.read( QStringLiteral( TEST_DATA_DIR ) + "/layouts/sample_project.qgs" );
  QDomElement docElem = composerElem.elementsByTagName( u"Composition"_s ).at( 0 ).toElement();
  std::unique_ptr<QgsPrintLayout> layout( QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project ) );

  QVERIFY( layout.get() );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemLabel *> items;
  layout->layoutItems<QgsLayoutItemLabel>( items );
  QVERIFY( items.size() > 0 );

  // Check the label
  const QgsLayoutItemLabel *label = items.at( 0 );
  QVERIFY( label );
  QCOMPARE( label->text(), u"QGIS"_s );
  QCOMPARE( label->pos().x(), 55.5333 );
  QCOMPARE( label->pos().y(), 35.3929 );
  QCOMPARE( label->sizeWithUnits().width(), 15.3686 );
  QCOMPARE( label->sizeWithUnits().height(), 7.93747 );
  QCOMPARE( label->referencePoint(), QgsLayoutItem::ReferencePoint::UpperRight );
  QCOMPARE( label->frameStrokeColor(), QColor( 251, 0, 0, 255 ) );
  QCOMPARE( label->frameStrokeWidth().length(), 0.2 );
  QCOMPARE( ( int ) label->rotation(), 4 );

  QGSVERIFYLAYOUTCHECK( "importComposerTemplateLabel_0", layout.get(), 0, 0, renderedPageSize( layout.get(), 0 ), 0 );

  qDeleteAll( items );
}

void TestQgsCompositionConverter::importComposerTemplateShape()
{
  QDomElement composerElem( loadComposer( u"2x_template_shape.qpt"_s ) );
  QgsProject project;
  project.read( QStringLiteral( TEST_DATA_DIR ) + "/layouts/sample_project.qgs" );
  QDomElement docElem = composerElem.elementsByTagName( u"Composition"_s ).at( 0 ).toElement();

  std::unique_ptr<QgsPrintLayout> layout( QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project ) );

  QVERIFY( layout.get() );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemShape *> items;
  layout->layoutItems<QgsLayoutItemShape>( items );
  QVERIFY( items.size() > 0 );

  // Check the shape
  const QgsLayoutItemShape *shape = items.at( 0 );
  QCOMPARE( shape->pos().x(), 261.132 );
  QCOMPARE( shape->pos().y(), 83.1791 );
  QCOMPARE( shape->sizeWithUnits().width(), 12.0988 );
  QCOMPARE( shape->sizeWithUnits().height(), 33.2716 );
  QCOMPARE( shape->sizeWithUnits().units(), Qgis::LayoutUnit::Millimeters );
  QCOMPARE( shape->referencePoint(), QgsLayoutItem::ReferencePoint::MiddleRight );
  QCOMPARE( shape->frameStrokeColor(), QColor( 0, 0, 0, 255 ) );
  QCOMPARE( shape->frameStrokeWidth().length(), 0.3 );
  QCOMPARE( shape->backgroundColor(), QColor( 255, 255, 255, 255 ) );
  QCOMPARE( ( int ) shape->rotation(), 0 );
  QCOMPARE( shape->frameEnabled(), false );
  QCOMPARE( shape->hasBackground(), false );

  QGSVERIFYLAYOUTCHECK( "importComposerTemplateShape_0", layout.get(), 0, 0, renderedPageSize( layout.get(), 0 ), 0 );
  qDeleteAll( items );
}

void TestQgsCompositionConverter::importComposerTemplatePicture()
{
  QDomElement composerElem( loadComposer( u"2x_template_pictures.qpt"_s ) );
  QVERIFY( !composerElem.isNull() );
  QgsProject project;
  project.read( QStringLiteral( TEST_DATA_DIR ) + "/layouts/sample_project.qgs" );
  QDomElement docElem = composerElem.elementsByTagName( u"Composition"_s ).at( 0 ).toElement();

  std::unique_ptr<QgsPrintLayout> layout( QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project ) );
  QVERIFY( layout.get() );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemPicture *> items;
  layout->layoutItems<QgsLayoutItemPicture>( items );
  QCOMPARE( items.size(), 1 );
  QVERIFY( QFile( items.at( 0 )->picturePath() ).exists() );

  QgsLayoutItemPicture *item = items.at( 0 );
  QCOMPARE( item->mPictureHeight, 18.1796 );
  QCOMPARE( item->mPictureWidth, 18.1796 );
  QCOMPARE( item->sizeWithUnits().width(), 25.7099 );
  QCOMPARE( item->sizeWithUnits().height(), 30.7511 );
  QCOMPARE( item->pos().x(), 207.192 );
  QCOMPARE( item->pos().y(), 12.6029 );
  QVERIFY( item->isVisible() );

  QGSVERIFYLAYOUTCHECK( "importComposerTemplatePicture_0", layout.get(), 0, 0, renderedPageSize( layout.get(), 0 ), 0 );

  qDeleteAll( items );
}

void TestQgsCompositionConverter::importComposerTemplatePolygon()
{
  QDomElement composerElem( loadComposer( u"2x_template_polygon.qpt"_s ) );
  QVERIFY( !composerElem.isNull() );
  QgsProject project;
  project.read( QStringLiteral( TEST_DATA_DIR ) + "/layouts/sample_project.qgs" );
  QDomElement docElem = composerElem.elementsByTagName( u"Composition"_s ).at( 0 ).toElement();

  std::unique_ptr<QgsPrintLayout> layout( QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project ) );
  QVERIFY( layout.get() );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemPolygon *> items;
  layout->layoutItems<QgsLayoutItemPolygon>( items );
  QCOMPARE( items.size(), 1 );

  QgsLayoutItemPolygon *item = items.at( 0 );
  QVERIFY( item->isVisible() );
  QCOMPARE( item->nodes().count(), 7 );

  QGSVERIFYLAYOUTCHECK( "importComposerTemplatePolygon_0", layout.get(), 0, 0, renderedPageSize( layout.get(), 0 ), 0 );

  qDeleteAll( items );
}

void TestQgsCompositionConverter::importComposerTemplatePolyline()
{
  QDomElement composerElem( loadComposer( u"2x_template_polyline.qpt"_s ) );
  QVERIFY( !composerElem.isNull() );
  QgsProject project;
  project.read( QStringLiteral( TEST_DATA_DIR ) + "/layouts/sample_project.qgs" );
  QDomElement docElem = composerElem.elementsByTagName( u"Composition"_s ).at( 0 ).toElement();

  std::unique_ptr<QgsPrintLayout> layout( QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project ) );
  QVERIFY( layout.get() );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemPolyline *> items;
  layout->layoutItems<QgsLayoutItemPolyline>( items );
  QCOMPARE( items.size(), 1 );

  QgsLayoutItemPolyline *item = items.at( 0 );
  QVERIFY( item->isVisible() );
  QCOMPARE( item->nodes().count(), 4 );
  QCOMPARE( item->startMarker(), QgsLayoutItemPolyline::MarkerMode::NoMarker );
  QCOMPARE( item->endMarker(), QgsLayoutItemPolyline::MarkerMode::NoMarker );

  QGSVERIFYLAYOUTCHECK( "importComposerTemplatePolyline_0", layout.get(), 0, 0, renderedPageSize( layout.get(), 0 ), 0 );

  qDeleteAll( items );
}

void TestQgsCompositionConverter::importComposerTemplateArrow()
{
  QDomElement composerElem( loadComposer( u"2x_template_arrow.qpt"_s ) );
  QVERIFY( !composerElem.isNull() );
  QgsProject project;
  project.read( QStringLiteral( TEST_DATA_DIR ) + "/layouts/sample_project.qgs" );
  QDomElement docElem = composerElem.elementsByTagName( u"Composition"_s ).at( 0 ).toElement();

  std::unique_ptr<QgsPrintLayout> layout( QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project ) );
  QVERIFY( layout.get() );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemPolyline *> items;
  layout->layoutItems<QgsLayoutItemPolyline>( items );
  QCOMPARE( items.size(), 1 );

  QgsLayoutItemPolyline *item = items.at( 0 );
  QVERIFY( item->isVisible() );
  QCOMPARE( item->nodes().count(), 2 );
  QCOMPARE( item->startMarker(), QgsLayoutItemPolyline::MarkerMode::NoMarker );
  QCOMPARE( item->endMarker(), QgsLayoutItemPolyline::MarkerMode::ArrowHead );

  QGSVERIFYLAYOUTCHECK( "importComposerTemplateArrow_0", layout.get(), 0, 0, renderedPageSize( layout.get(), 0 ), 0 );

  qDeleteAll( items );
}


void TestQgsCompositionConverter::importComposerTemplateMap()
{
  QDomElement composerElem( loadComposer( u"2x_template_map_overview.qpt"_s ) );
  QVERIFY( !composerElem.isNull() );
  QgsProject project;
  project.read( QStringLiteral( TEST_DATA_DIR ) + "/layouts/sample_project.qgs" );
  QDomElement docElem = composerElem.elementsByTagName( u"Composition"_s ).at( 0 ).toElement();

  std::unique_ptr<QgsPrintLayout> layout( QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project ) );
  QVERIFY( layout.get() );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemMap *> items;
  layout->layoutItems<QgsLayoutItemMap>( items );
  QCOMPARE( items.size(), 2 );

  QgsLayoutItemMap *item = items.at( 0 );
  QVERIFY( item->isVisible() );

  item->setLayers( project.mapLayers().values() );

  for ( auto const &l : project.mapLayers() )
  {
    QVERIFY( l->isValid() );
  }

  QgsLayoutItemMap *item1 = items.at( 1 );
  QVERIFY( item1->isVisible() );
  QCOMPARE( item1->opacity(), 0.78 );
  item1->setLayers( project.mapLayers().values() );
  item1->setExtent( QgsRectangle( -126.5731570061082038, -4.69162199770811128, -88.56641716083402116, 69.08616711370645191 ) );

  // Check map ids
  QStringList mapUuids;
  QList<QgsLayoutItemMap *> mapItems;
  layout->layoutItems<QgsLayoutItemMap>( mapItems );
  mapUuids.reserve( mapItems.count() );
  for ( auto const &item : mapItems )
  {
    mapUuids << item->uuid();
  }

  {
    int count = 0;
    QList<QgsLayoutItemMap *> items;
    layout->layoutItems<QgsLayoutItemMap>( items );
    for ( auto const &mapItem : std::as_const( items ) )
    {
      const auto overviewItems = mapItem->overviews()->asList();
      for ( auto const &item : overviewItems )
      {
        if ( !item->map()->uuid().isEmpty() )
        {
          QVERIFY( mapUuids.contains( item->map()->uuid() ) );
          count++;
        }
      }
    }
    // We have at least one item linked to a map for this test
    QVERIFY( count > 0 );
  }

  QGSVERIFYLAYOUTCHECK( "importComposerTemplateMap_0", layout.get(), 0, 0, renderedPageSize( layout.get(), 0 ), 0 );

  qDeleteAll( items );
}

void TestQgsCompositionConverter::importComposerTemplateLegend()
{
  QDomElement composerElem( loadComposer( u"2x_template_legend.qpt"_s ) );
  QVERIFY( !composerElem.isNull() );
  QgsProject project;
  project.read( QStringLiteral( TEST_DATA_DIR ) + "/layouts/sample_project.qgs" );
  QDomElement docElem = composerElem.elementsByTagName( u"Composition"_s ).at( 0 ).toElement();

  std::unique_ptr<QgsPrintLayout> layout( QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project ) );
  QVERIFY( layout.get() );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemLegend *> items;
  layout->layoutItems<QgsLayoutItemLegend>( items );
  QCOMPARE( items.size(), 1 );

  QgsLayoutItemLegend *item = items.at( 0 );
  QVERIFY( item->isVisible() );
  QVERIFY( !item->autoUpdateModel() );

  QGSVERIFYLAYOUTCHECK( "importComposerTemplateLegend_0", layout.get(), 0, 0, renderedPageSize( layout.get(), 0 ), 0 );

  qDeleteAll( items );
}

void TestQgsCompositionConverter::importComposerTemplateAttributeTable()
{
  QDomElement composerElem( loadComposer( u"2x_template_attributetable.qpt"_s ) );
  QgsProject project;
  project.read( QStringLiteral( TEST_DATA_DIR ) + "/layouts/sample_project.qgs" );
  QDomElement docElem = composerElem.elementsByTagName( u"Composition"_s ).at( 0 ).toElement();
  std::unique_ptr<QgsPrintLayout> layout( QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project ) );

  QVERIFY( layout.get() );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  // Check the table
  QList<QgsLayoutItemAttributeTable *> items;
  layout->layoutObjects<QgsLayoutItemAttributeTable>( items );
  QVERIFY( items.size() > 0 );
  const QgsLayoutItemAttributeTable *table = items.at( 0 );
  QVERIFY( table );
  QVERIFY( table->sourceLayer() );
  QVERIFY( table->sourceLayer()->isValid() );

  QGSVERIFYLAYOUTCHECK( "importComposerTemplateAttributeTable_0", layout.get(), 0, 0, renderedPageSize( layout.get(), 0 ), 0 );
}

void TestQgsCompositionConverter::importComposerTemplateScaleBar()
{
  QDomElement composerElem( loadComposer( u"2x_template_scalebar.qpt"_s ) );
  QVERIFY( !composerElem.isNull() );
  QgsProject project;
  project.read( QStringLiteral( TEST_DATA_DIR ) + "/layouts/sample_project.qgs" );
  QDomElement docElem = composerElem.elementsByTagName( u"Composition"_s ).at( 0 ).toElement();

  std::unique_ptr<QgsLayout> layout( QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project ) );
  QVERIFY( layout.get() );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemScaleBar *> items;
  layout->layoutItems<QgsLayoutItemScaleBar>( items );
  QCOMPARE( items.size(), 1 );

  QgsLayoutItemScaleBar *item = items.at( 0 );
  QVERIFY( item->isVisible() );

  QVERIFY( !item->linkedMap() );

  QGSVERIFYLAYOUTCHECK( "importComposerTemplateScaleBar_0", layout.get(), 0, 0, renderedPageSize( layout.get(), 0 ), 0 );
  qDeleteAll( items );
}

void TestQgsCompositionConverter::importComposerTemplateGroup()
{
  QDomElement composerElem( loadComposer( u"2x_template_group.qpt"_s ) );
  QVERIFY( !composerElem.isNull() );
  QgsProject project;
  project.read( QStringLiteral( TEST_DATA_DIR ) + "/layouts/sample_project.qgs" );
  QDomElement docElem = composerElem.elementsByTagName( u"Composition"_s ).at( 0 ).toElement();

  std::unique_ptr<QgsLayout> layout( QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project ) );
  QVERIFY( layout.get() );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemGroup *> items;
  layout->layoutItems<QgsLayoutItemGroup>( items );
  QCOMPARE( items.size(), 1 );

  QgsLayoutItemGroup *item = items.at( 0 );
  QVERIFY( item->isVisible() );
}


void TestQgsCompositionConverter::convertComposition()
{
  QgsProject project;
  project.read( QStringLiteral( TEST_DATA_DIR ) + "/layouts/sample_project.qgs" );

  QgsPrintLayout *layout = dynamic_cast<QgsPrintLayout *>( project.layoutManager()->layouts().first() );

  QVERIFY( layout );
  QCOMPARE( layout->pageCollection()->pageCount(), 2 );
  QCOMPARE( layout->name(), u"composer title"_s );
  QCOMPARE( layout->renderContext().dpi(), 305.0 );

  QStringList variableNames = layout->customProperty( u"variableNames"_s ).toStringList();
  QStringList variableValues = layout->customProperty( u"variableValues"_s ).toStringList();
  QCOMPARE( variableNames, QStringList() << u"variable1"_s << u"variable2"_s );
  QCOMPARE( variableValues, QStringList() << u"100"_s << u"200"_s );

  // Check guides
  QCOMPARE( layout->guides().rowCount( QModelIndex() ), 8 );
}

void TestQgsCompositionConverter::isCompositionTemplate()
{
  QString templatePath( QStringLiteral( TEST_DATA_DIR ) + "/layouts/2x_template.qpt" );
  QDomDocument doc( u"mydocument"_s );
  QFile file( templatePath );
  QVERIFY( file.open( QIODevice::ReadOnly ) );
  QVERIFY( doc.setContent( &file ) );
  file.close();

  QVERIFY( QgsCompositionConverter::isCompositionTemplate( doc ) );
}

void TestQgsCompositionConverter::convertCompositionTemplate()
{
  QString templatePath( QStringLiteral( TEST_DATA_DIR ) + "/layouts/2x_template.qpt" );
  QDomDocument doc( u"mydocument"_s );
  QFile file( templatePath );
  QVERIFY( file.open( QIODevice::ReadOnly ) );
  QVERIFY( doc.setContent( &file ) );
  file.close();

  QgsProject project;

  QDomDocument layoutDoc = QgsCompositionConverter::convertCompositionTemplate( doc, &project );
  //qDebug() << layoutDoc;
  QCOMPARE( layoutDoc.elementsByTagName( u"Layout"_s ).count(), 1 );

  auto layout = std::make_unique<QgsLayout>( &project );
  QgsReadWriteContext context;
  context.setPathResolver( project.pathResolver() );
  layout->readXml( layoutDoc.elementsByTagName( u"Layout"_s ).at( 0 ).toElement(), layoutDoc, context );
  QVERIFY( layout.get() );
  QCOMPARE( layout->pageCollection()->pageCount(), 2 );
}

void TestQgsCompositionConverter::importComposerTemplate()
{
  QDomElement composerElem( loadComposer( u"2x_template.qpt"_s ) );
  QgsProject project;
  project.read( QStringLiteral( TEST_DATA_DIR ) + "/layouts/sample_project.qgs" );
  QDomElement docElem = composerElem.elementsByTagName( u"Composition"_s ).at( 0 ).toElement();

  std::unique_ptr<QgsPrintLayout> layout( QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project ) );

  QVERIFY( layout.get() );
  QCOMPARE( layout->pageCollection()->pageCount(), 2 );
  QCOMPARE( layout->name(), u"composer title"_s );

  // Check map ids
  QStringList mapUuids;
  QList<QgsLayoutItemMap *> mapItems;
  layout->layoutItems<QgsLayoutItemMap>( mapItems );
  mapUuids.reserve( mapItems.count() );
  for ( auto const &item : std::as_const( mapItems ) )
  {
    mapUuids << item->uuid();
  }

  // Check that picture elements with a map id point to a valid map uuid
  {
    int count = 0;
    QList<QgsLayoutItemPicture *> items;
    layout->layoutItems<QgsLayoutItemPicture>( items );
    for ( auto const &item : std::as_const( items ) )
    {
      if ( item->linkedMap() )
      {
        QVERIFY( mapUuids.contains( item->linkedMap()->uuid() ) );
        count++;
      }
    }
    // We have at least one item linked to a map for this test
    QVERIFY( count > 0 );
  }


  // Check that elements with a map id point to a valid map uuid
  {
    int count = 0;
    QList<QgsLayoutItemLegend *> items;
    layout->layoutItems<QgsLayoutItemLegend>( items );
    for ( auto const &item : std::as_const( items ) )
    {
      if ( item->linkedMap() )
      {
        QVERIFY( mapUuids.contains( item->linkedMap()->uuid() ) );
        count++;
      }
    }
    // We have at least one item linked to a map for this test
    QVERIFY( count > 0 );
  }

  // Check that elements with a map id point to a valid map uuid
  {
    int count = 0;
    QList<QgsLayoutItemScaleBar *> items;
    layout->layoutItems<QgsLayoutItemScaleBar>( items );
    for ( auto const &item : std::as_const( items ) )
    {
      if ( item->linkedMap() )
      {
        QVERIFY( mapUuids.contains( item->linkedMap()->uuid() ) );
        count++;
      }
    }
    // We have at least one item linked to a map for this test
    QVERIFY( count > 0 );
  }

  QGSVERIFYLAYOUTCHECK( "importComposerTemplate_0", layout.get(), 0, 0, renderedPageSize( layout.get(), 0 ), 0 );
  QGSVERIFYLAYOUTCHECK( "importComposerTemplate_1_nowebkit", layout.get(), 1, 0, renderedPageSize( layout.get(), 1 ), 0 );
}

void TestQgsCompositionConverter::importComposerAtlas()
{
  QDomElement composerElem( loadComposer( u"2x_template_atlas.qpt"_s ) );
  QVERIFY( !composerElem.isNull() );
  QVERIFY( !composerElem.attribute( u"title"_s ).isEmpty() );
  QgsProject project;
  project.read( QStringLiteral( TEST_DATA_DIR ) + "/layouts/sample_project.qgs" );
  QDomElement docElem = composerElem.elementsByTagName( u"Composition"_s ).at( 0 ).toElement();

  std::unique_ptr<QgsPrintLayout> layout( QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project ) );
  QVERIFY( layout.get() );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );
  QCOMPARE( layout->name(), u"composer atlas"_s );

  QVERIFY( layout->atlas()->enabled() );
  QVERIFY( layout->atlas()->updateFeatures() > 0 );

  QGSVERIFYLAYOUTCHECK( "importComposerAtlas_0", layout.get(), 0, 0, renderedPageSize( layout.get(), 0 ), 0 );
}

QSize TestQgsCompositionConverter::renderedPageSize( QgsLayout *layout, const int pageNumber )
{
  return QSize( static_cast< int >( layout->pageCollection()->page( pageNumber )->sizeWithUnits().width() * 3.77 ), static_cast< int >( layout->pageCollection()->page( pageNumber )->sizeWithUnits().height() * 3.77 ) );
}

QDomElement TestQgsCompositionConverter::loadComposer( const QString &name )
{
  QString templatePath( QStringLiteral( TEST_DATA_DIR ) + "/layouts/" + name );
  QDomDocument doc( u"mydocument"_s );
  QFile file( templatePath );
  bool res = file.open( QIODevice::ReadOnly );
  Q_ASSERT( res );
  res = static_cast<bool>( doc.setContent( &file ) );
  Q_ASSERT( res );
  file.close();
  QDomNodeList nodes( doc.elementsByTagName( u"Composer"_s ) );
  if ( nodes.length() > 0 )
    return nodes.at( 0 ).toElement();
  else
  {
    QDomElement elem;
    return elem;
  }
}


QGSTEST_MAIN( TestQgsCompositionConverter )
#include "testqgscompositionconverter.moc"
