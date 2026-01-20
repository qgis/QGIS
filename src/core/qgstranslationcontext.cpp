/***************************************************************************
  qgstranslationcontext.cpp

 ---------------------
 begin                : 23.5.2018
 copyright            : (C) 2018 by David Signer
 email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstranslationcontext.h"

#include "qgssettings.h"

#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QTextStream>

QgsProject *QgsTranslationContext::project() const
{
  return mProject;
}

void QgsTranslationContext::setProject( QgsProject *project )
{
  mProject = project;
}

QString QgsTranslationContext::fileName() const
{
  return mFileName;
}

void QgsTranslationContext::setFileName( const QString &fileName )
{
  mFileName = fileName;
}

void QgsTranslationContext::registerTranslation( const QString &context, const QString &source )
{
  TranslatableObject translatableObject;
  translatableObject.context = context;
  translatableObject.source = source;
  if ( !mTranslatableObjects.contains( translatableObject ) )
  {
    mTranslatableObjects.append( translatableObject );
  }
}

void QgsTranslationContext::writeTsFile( const QString &locale ) const
{
  //write xml
  QDomDocument doc( u"TS"_s );

  QDomElement tsElement = doc.createElement( u"TS"_s );
  tsElement.setAttribute( u"sourcelanguage"_s, locale );
  doc.appendChild( tsElement );

  for ( const TranslatableObject &translatableObject : mTranslatableObjects )
  {
    QDomElement contextElement = doc.createElement( u"context"_s );
    tsElement.appendChild( contextElement );

    QDomElement nameElement = doc.createElement( u"name"_s );
    const QDomText nameText = doc.createTextNode( translatableObject.context );
    nameElement.appendChild( nameText );
    contextElement.appendChild( nameElement );

    QDomElement messageElement = doc.createElement( u"message"_s );
    contextElement.appendChild( messageElement );

    QDomElement sourceElement = doc.createElement( u"source"_s );
    const QDomText sourceText = doc.createTextNode( translatableObject.source );
    sourceElement.appendChild( sourceText );
    messageElement.appendChild( sourceElement );

    QDomElement translationElement = doc.createElement( u"translation"_s );
    translationElement.setAttribute( u"type"_s, u"unfinished"_s );
    messageElement.appendChild( translationElement );
  }

  //write file
  QFile tsFile( fileName() );
  if ( !tsFile.open( QIODevice::WriteOnly ) )
  {
    QgsDebugError( u"Can't open file %1"_s.arg( fileName() ) );
    return;
  }
  QTextStream stream( &tsFile );
  stream << doc.toString();
  tsFile.close();
}
