/***************************************************************************
    qgsreadwritecontext.cpp
    ---------------------
    begin                : May 2017
    copyright            : (C) 2017 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsreadwritecontext.h"

QgsReadWriteContext::QgsReadWriteContext()
  : mProjectTranslator( &mDefaultTranslator )
{

}

QgsReadWriteContext::~QgsReadWriteContext()
{
  // be sure that categories have been emptied
  Q_ASSERT( mCategories.isEmpty() );
}

const QgsPathResolver &QgsReadWriteContext::pathResolver() const
{
  return mPathResolver;
}

void QgsReadWriteContext::setPathResolver( const QgsPathResolver &resolver )
{
  mPathResolver = resolver;
}

void QgsReadWriteContext::pushMessage( const QString &message, Qgis::MessageLevel level )
{
  mMessages.append( ReadWriteMessage( message, level, mCategories ) );
}

QgsReadWriteContextCategoryPopper QgsReadWriteContext::enterCategory( const QString &category, const QString &details )
{
  QString message = category;
  if ( !details.isEmpty() )
    message.append( QString( " :: %1" ).arg( details ) );
  mCategories.push_back( message );
  return QgsReadWriteContextCategoryPopper( *this );
}

void QgsReadWriteContext::leaveCategory()
{
  if ( !mCategories.isEmpty() )
    mCategories.pop_back();
}

void QgsReadWriteContext::setProjectTranslator( QgsProjectTranslator *projectTranslator )
{
  mProjectTranslator = projectTranslator;
}

QList<QgsReadWriteContext::ReadWriteMessage > QgsReadWriteContext::takeMessages()
{
  QList<QgsReadWriteContext::ReadWriteMessage > messages = mMessages;
  mMessages.clear();
  return messages;
}

QString QgsReadWriteContext::DefaultTranslator::translate( const QString &context, const QString &sourceText, const char *disambiguation, int n ) const
{
  Q_UNUSED( context );
  Q_UNUSED( disambiguation );
  Q_UNUSED( n );
  return sourceText;
}
