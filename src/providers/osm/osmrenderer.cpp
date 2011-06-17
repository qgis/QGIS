/***************************************************************************
    osmrenderer.cpp - handler for parsing OSM data
    ------------------
    begin                : April 2009
    copyright            : (C) 2009 by Lukas Berka
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "osmrenderer.h"

#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsgeometry.h"
#include "qgsvectorlayer.h"

#include <QtXml/QXmlSimpleReader>
#include <QtXml/QXmlInputSource>
#include <QtGui/QApplication>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtXml/QXmlAttributes>
#include <QtCore/QFile>

using namespace std;



OsmRenderer::OsmRenderer( QGis::GeometryType geometryType, QString styleFileName )
    : QgsRenderer(), osmstyle( styleFileName ), mGeomType( geometryType )
{
}


QMap<QString, QString> OsmRenderer::parse_tags( QString tags )
{
  QMap<QString, QString> t;
  if ( tags.size() == 0 )
  {
    QgsDebugMsg( "no tags for feature!" );
    return t;
  }

  // tags: "key1"="val1","key2"="val2","key3"="val3"
  // -all original ; in keyX and valX are replaced by ;;
  // -all original , in keyX and valX are replaced by ;
  // -all original - in keyX and valX are replaced by --
  // -all original = in keyX and valX are replaced by -

  QStringList tag_pairs = tags.split( "," );
  for ( int i = 0; i < tag_pairs.size(); ++i )
  {
    QStringList duo = tag_pairs.at( i ).split( "=" );
    if ( duo.count() != 2 )
    {
      QgsDebugMsg( "invalid tag value: " + tag_pairs.at( i ) );
      continue;
    }
    QString key = duo[0];
    QString val = duo[1];

    key = key.replace( ';', "," );
    val = val.replace( ';', "," );
    key = key.replace( ";;", ";" );
    val = val.replace( ";;", ";" );

    key = key.replace( '-', "=" );
    val = val.replace( '-', "=" );
    key = key.replace( "--", "-" );
    val = val.replace( "--", "-" );

    // dequoting
    key = key.mid( 1, key.size() - 2 );
    val = val.mid( 1, val.size() - 2 );
    // put tag into map
    t.insert( key, val );
  }
  return t;
}


bool OsmRenderer::willRenderFeature( QgsFeature *f )
{
  Q_UNUSED( f );
  return true;
}


void OsmRenderer::renderFeature( QgsRenderContext &renderContext, QgsFeature& f, QImage* pic, bool selected, double opacity )
{
  Q_UNUSED( selected );
  QPainter* p = renderContext.painter();
  QgsAttributeMap attr_map = f.attributeMap();
  QMap<QString, QString> tags = parse_tags( attr_map[2].toString() );

  if ( mGeomType == QGis::Line )
  {
    QPen pen = osmstyle.get_pen( tags );
    p->setPen( osmstyle.get_pen( tags ) );
    p->setOpacity( opacity );
  }
  else if ( mGeomType == QGis::Polygon )
  {
    QBrush br;
    p->setPen( osmstyle.get_pen_brush( tags, br ) );
    p->setBrush( br );
    p->setBackgroundMode( Qt::TransparentMode );
    p->setOpacity( opacity );
  }
  else if ( mGeomType == QGis::Point )
  {
    *pic = osmstyle.get_image( tags );
    p->setOpacity( opacity );
  }
}


int OsmRenderer::readXML( const QDomNode &rnode, QgsVectorLayer &vl )
{
  Q_UNUSED( rnode );
  Q_UNUSED( vl );
  return 0;
}


bool OsmRenderer::writeXML( QDomNode &layer_node, QDomDocument &document, const QgsVectorLayer &vl ) const
{
  Q_UNUSED( layer_node );
  Q_UNUSED( document );
  Q_UNUSED( vl );
  return true;
}


bool OsmRenderer::needsAttributes() const
{
  return true;
}


QgsAttributeList OsmRenderer::classificationAttributes() const
{
  QgsAttributeList attr_list;
  attr_list.append( 2 );
  return attr_list;
}


QString OsmRenderer::name() const
{
  return QString( "OSM" );
}


const QList< QgsSymbol * > OsmRenderer::symbols() const
{
  const QList<QgsSymbol*> sym;
  return sym;
}


QgsRenderer *OsmRenderer::clone() const
{
  return 0;
}


bool OsmRenderer::containsPixmap() const
{
  return false;
}


bool OsmRenderer::usesTransparency() const
{
  return false;
}

