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

QgsAnnotationManager::QgsAnnotationManager( QgsProject* project )
    : QObject( project )
    , mProject( project )
{

}

QgsAnnotationManager::~QgsAnnotationManager()
{
  clear();
}

bool QgsAnnotationManager::addAnnotation( QgsAnnotation* annotation )
{
  if ( !annotation )
    return false;

  if ( mAnnotations.contains( annotation ) )
    return true;

  mAnnotations << annotation;
  emit annotationAdded( annotation );
  return true;
}

bool QgsAnnotationManager::removeAnnotation( QgsAnnotation* annotation )
{
  if ( !annotation )
    return false;

  if ( !mAnnotations.contains( annotation ) )
    return false;

  emit annotationAboutToBeRemoved( annotation );
  mAnnotations.removeAll( annotation );
  delete annotation;
  emit annotationRemoved();
  return true;
}

void QgsAnnotationManager::clear()
{
  Q_FOREACH ( QgsAnnotation* a, mAnnotations )
  {
    removeAnnotation( a );
  }
}

QList<QgsAnnotation*> QgsAnnotationManager::annotations() const
{
  return mAnnotations;
}

bool QgsAnnotationManager::readXml( const QDomElement& element, const QDomDocument& doc )
{
  clear();
  //restore each annotation
  bool result = true;
  QDomNodeList annotationNodes = element.elementsByTagName( QStringLiteral( "Annotations" ) );
  for ( int i = 0; i < annotationNodes.size(); ++i )
  {
    QgsAnnotation* a = createAnnotationFromXml( annotationNodes.at( i ).toElement(), doc );
    if ( !a )
    {
      result = false;
      continue;
    }
    addAnnotation( a );
  }
  return result;
}

QgsAnnotation* QgsAnnotationManager::createAnnotationFromXml( const QDomElement& element, const QDomDocument& ) const
{
  QDomNodeList annotationNodeList = element.elementsByTagName( QStringLiteral( "Annotation" ) );

  if ( annotationNodeList.size() > 0 )
  {
    return nullptr;
  }
  else
  {
    return nullptr;
  }
}
