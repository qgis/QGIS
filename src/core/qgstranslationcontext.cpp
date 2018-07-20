/***************************************************************************
  qgstranslationcontext.cpp - %{Cpp:License:ClassName}

 ---------------------
 begin                : 23.5.2018
 copyright            : (C) 2018 by david
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstranslationcontext.h"

#include <QDir>
#include <QTextStream>
#include <QDomElement>
#include <QDomDocument>

QgsTranslationContext::QgsTranslationContext()
{}

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
  mTranslatableObjects.append( translatableObject );
}

void QgsTranslationContext::writeTsFile()
{
  //write xml
  QDomDocument doc( QStringLiteral( "TS" ) );

  QDomElement tsElement = doc.createElement( QStringLiteral( "TS" ) );
  doc.appendChild( tsElement );

  for ( TranslatableObject translatableObject : mTranslatableObjects )
  {
    QDomElement contextElement = doc.createElement( QStringLiteral( "context" ) );
    tsElement.appendChild( contextElement );

    QDomElement nameElement = doc.createElement( QStringLiteral( "name" ) );
    QDomText nameText = doc.createTextNode( translatableObject.context );
    nameElement.appendChild( nameText );
    contextElement.appendChild( nameElement );

    QDomElement messageElement = doc.createElement( QStringLiteral( "message" ) );
    contextElement.appendChild( messageElement );

    QDomElement sourceElement = doc.createElement( QStringLiteral( "source" ) );
    QDomText sourceText = doc.createTextNode( translatableObject.source );
    sourceElement.appendChild( sourceText );
    messageElement.appendChild( sourceElement );

    QDomElement translationElement = doc.createElement( QStringLiteral( "translation" ) );
    QDomText translationText = doc.createTextNode( translatableObject.source );
    translationElement.appendChild( translationText );
    messageElement.appendChild( translationElement );
  }

  //write file
  QFile tsFile( fileName() );
  tsFile.open( QIODevice::WriteOnly );
  QTextStream stream( &tsFile );
  stream << doc.toString();
  tsFile.close();
}
