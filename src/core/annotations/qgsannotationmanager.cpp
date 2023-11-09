/***************************************************************************
    qgsannotationmanager.cpp
    ------------------------
    Date                 : January 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationmanager.h"
#include "qgsproject.h"
#include "qgsannotation.h"
#include "qgsannotationregistry.h"
#include "qgsapplication.h"
#include "qgsstyleentityvisitor.h"

QgsAnnotationManager::QgsAnnotationManager( QgsProject *project )
  : QObject( project )
  , mProject( project )
{

}

QgsAnnotationManager::~QgsAnnotationManager()
{
  clear();
}

bool QgsAnnotationManager::addAnnotation( QgsAnnotation *annotation )
{
  if ( !annotation )
    return false;

  if ( mAnnotations.contains( annotation ) )
    return true;

  mAnnotations << annotation;
  emit annotationAdded( annotation );
  mProject->setDirty( true );
  return true;
}

bool QgsAnnotationManager::removeAnnotation( QgsAnnotation *annotation )
{
  if ( !annotation )
    return false;

  if ( !mAnnotations.contains( annotation ) )
    return false;

  emit annotationAboutToBeRemoved( annotation );
  mAnnotations.removeAll( annotation );
  delete annotation;
  emit annotationRemoved();
  mProject->setDirty( true );
  return true;
}

void QgsAnnotationManager::clear()
{
  for ( auto *a : std::as_const( mAnnotations ) )
  {
    removeAnnotation( a );
  }
}

QList<QgsAnnotation *> QgsAnnotationManager::annotations() const
{
  return mAnnotations;
}

QList<QgsAnnotation *> QgsAnnotationManager::cloneAnnotations() const
{
  QList<QgsAnnotation *> results;
  for ( const auto *a : std::as_const( mAnnotations ) )
  {
    results << a->clone();
  }
  return results;
}

bool QgsAnnotationManager::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  clear();
  //restore each annotation
  bool result = true;

  QDomElement annotationsElem = element.firstChildElement( QStringLiteral( "Annotations" ) );

  QDomElement annotationElement = annotationsElem.firstChildElement( QStringLiteral( "Annotation" ) );
  while ( ! annotationElement .isNull() )
  {
    createAnnotationFromXml( annotationElement, context );
    annotationElement = annotationElement.nextSiblingElement( QStringLiteral( "Annotation" ) );
  }

  // restore old (pre 3.0) project annotations
  if ( annotationElement.isNull() )
  {
    QDomNodeList oldItemList = element.elementsByTagName( QStringLiteral( "TextAnnotationItem" ) );
    for ( int i = 0; i < oldItemList.size(); ++i )
    {
      createAnnotationFromXml( oldItemList.at( i ).toElement(), context );
    }
    oldItemList = element.elementsByTagName( QStringLiteral( "FormAnnotationItem" ) );
    for ( int i = 0; i < oldItemList.size(); ++i )
    {
      createAnnotationFromXml( oldItemList.at( i ).toElement(), context );
    }
    oldItemList = element.elementsByTagName( QStringLiteral( "HtmlAnnotationItem" ) );
    for ( int i = 0; i < oldItemList.size(); ++i )
    {
      createAnnotationFromXml( oldItemList.at( i ).toElement(), context );
    }
    oldItemList = element.elementsByTagName( QStringLiteral( "SVGAnnotationItem" ) );
    for ( int i = 0; i < oldItemList.size(); ++i )
    {
      createAnnotationFromXml( oldItemList.at( i ).toElement(), context );
    }
  }

  return result;
}

QDomElement QgsAnnotationManager::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement annotationsElem = doc.createElement( QStringLiteral( "Annotations" ) );
  QListIterator<QgsAnnotation *> i( mAnnotations );
  // save lowermost annotation (at end of list) first
  i.toBack();
  while ( i.hasPrevious() )
  {
    QgsAnnotation *annotation = i.previous();

    if ( !annotation )
    {
      continue;
    }

    annotation->writeXml( annotationsElem, doc, context );
  }
  return annotationsElem;
}

bool QgsAnnotationManager::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( mAnnotations.empty() )
    return true;

  // NOTE: if visitEnter returns false it means "don't visit any annotations", not "abort all further visitations"
  if ( !visitor->visitEnter( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::Annotations, QStringLiteral( "annotations" ), tr( "Annotations" ) ) ) )
    return true;

  for ( QgsAnnotation *a : mAnnotations )
  {
    if ( !a->accept( visitor ) )
      return false;
  }

  if ( !visitor->visitExit( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::Annotations, QStringLiteral( "annotations" ), tr( "Annotations" ) ) ) )
    return false;

  return true;
}

void QgsAnnotationManager::createAnnotationFromXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  QString type = element.tagName();
  QgsAnnotation *annotation = QgsApplication::annotationRegistry()->create( type );
  if ( !annotation )
    return;

  annotation->readXml( element, context );

  if ( !annotation->mapPositionCrs().isValid() )
  {
    annotation->setMapPositionCrs( mProject->crs() );
  }

  addAnnotation( annotation );
}
