/***************************************************************************
                             qgsacademicreference.cpp
                             -------------------
    begin                : August 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsacademicreference.h"

#include <QString>

using namespace Qt::StringLiterals;

QgsAcademicReference QgsAcademicReference::createBook( const QStringList &authors, int year, const QString &title, const QString &publisher )
{
  QgsAcademicReference reference;
  reference.setType( Qgis::AcademicReferenceType::Book );
  reference.setAuthors( authors );
  reference.setYear( year );
  reference.setTitle( title );
  reference.setPublisher( publisher );
  return reference;
}

QgsAcademicReference QgsAcademicReference::createJournalArticle(
  const QStringList &authors, int year, const QString &title, const QString &journal, const QString &volume, const QString &issue, const QString &pages
)
{
  QgsAcademicReference reference;
  reference.setType( Qgis::AcademicReferenceType::JournalArticle );
  reference.setAuthors( authors );
  reference.setYear( year );
  reference.setTitle( title );
  reference.setJournal( journal );
  reference.setVolume( volume );
  reference.setIssue( issue );
  reference.setPages( pages );
  return reference;
}

QgsAcademicReference QgsAcademicReference::createPresentation( const QStringList &authors, int year, const QString &title, const QString &meeting, const QString &publisher, const QString &pages )
{
  QgsAcademicReference reference;
  reference.setType( Qgis::AcademicReferenceType::Presentation );
  reference.setAuthors( authors );
  reference.setYear( year );
  reference.setTitle( title );
  reference.setJournal( meeting );
  reference.setPublisher( publisher );
  reference.setPages( pages );
  return reference;
}

QgsAcademicReference QgsAcademicReference::createWebPage( const QStringList &authors, int year, const QString &title, const QString &url )
{
  QgsAcademicReference reference;
  reference.setType( Qgis::AcademicReferenceType::WebPage );
  reference.setAuthors( authors );
  reference.setYear( year );
  reference.setTitle( title );
  reference.setUrl( url );
  return reference;
}

QgsAcademicReference QgsAcademicReference::createPreprint( const QStringList &authors, int year, const QString &title, const QString &repository, const QString &url )
{
  QgsAcademicReference reference;
  reference.setType( Qgis::AcademicReferenceType::Preprint );
  reference.setAuthors( authors );
  reference.setYear( year );
  reference.setTitle( title );
  reference.setJournal( repository );
  reference.setUrl( url );
  return reference;
}

Qgis::AcademicReferenceType QgsAcademicReference::type() const
{
  return mType;
}

void QgsAcademicReference::setType( Qgis::AcademicReferenceType type )
{
  mType = type;
}

QStringList QgsAcademicReference::authors() const
{
  return mAuthors;
}

void QgsAcademicReference::setAuthors( const QStringList &authors )
{
  mAuthors = authors;
}

int QgsAcademicReference::year() const
{
  return mYear;
}

void QgsAcademicReference::setYear( int year )
{
  mYear = year;
}

QString QgsAcademicReference::title() const
{
  return mTitle;
}

void QgsAcademicReference::setTitle( const QString &title )
{
  mTitle = title;
}

QString QgsAcademicReference::journal() const
{
  return mJournal;
}

void QgsAcademicReference::setJournal( const QString &journal )
{
  mJournal = journal;
}

QString QgsAcademicReference::volume() const
{
  return mVolume;
}

void QgsAcademicReference::setVolume( const QString &volume )
{
  mVolume = volume;
}

QString QgsAcademicReference::issue() const
{
  return mIssue;
}

void QgsAcademicReference::setIssue( const QString &issue )
{
  mIssue = issue;
}

QString QgsAcademicReference::pages() const
{
  return mPages;
}

void QgsAcademicReference::setPages( const QString &pages )
{
  mPages = pages;
}

QString QgsAcademicReference::publisher() const
{
  return mPublisher;
}

void QgsAcademicReference::setPublisher( const QString &publisher )
{
  mPublisher = publisher;
}

QString QgsAcademicReference::url() const
{
  return mUrl;
}

void QgsAcademicReference::setUrl( const QString &url )
{
  mUrl = url;
}

QString QgsAcademicReference::asPlainText() const
{
  return formatted( false );
}

QString QgsAcademicReference::asHtml() const
{
  return formatted( true );
}

QString QgsAcademicReference::formatted( bool asHtml ) const
{
  QString formattedReference;

  if ( !mAuthors.isEmpty() )
  {
    if ( mAuthors.size() == 1 )
    {
      formattedReference += mAuthors.first();
    }
    else if ( mAuthors.size() == 2 )
    {
      formattedReference += mAuthors.at( 0 ) + u" & "_s + mAuthors.at( 1 );
    }
    else
    {
      for ( int index = 0; index < mAuthors.size(); ++index )
      {
        if ( index > 0 )
        {
          if ( index == mAuthors.size() - 1 )
          {
            formattedReference += ", & "_L1;
          }
          else
          {
            formattedReference += ", "_L1;
          }
        }
        formattedReference += mAuthors.at( index );
      }
    }

    if ( !formattedReference.endsWith( '.' ) )
    {
      formattedReference += '.'_L1;
    }
  }

  if ( mYear > 0 )
  {
    formattedReference += u" ("_s + QString::number( mYear ) + u")."_s;
  }

  if ( !mTitle.isEmpty() )
  {
    if ( !formattedReference.isEmpty() )
    {
      formattedReference += ' '_L1;
    }
    formattedReference += mTitle;
    if ( !formattedReference.endsWith( '.' ) )
    {
      formattedReference += '.'_L1;
    }
  }

  if ( !mJournal.isEmpty() )
  {
    if ( !formattedReference.isEmpty() )
    {
      formattedReference += ' '_L1;
    }

    const QString journalText = asHtml ? u"<i>%1</i>"_s.arg( mJournal ) : mJournal;
    formattedReference += journalText;

    if ( !mVolume.isEmpty() )
    {
      const QString volumeText = asHtml ? u"<i>%1</i>"_s.arg( mVolume ) : mVolume;
      formattedReference += u" "_s + volumeText;
    }

    if ( !mIssue.isEmpty() )
    {
      formattedReference += u"("_s + mIssue + u")"_s;
    }

    if ( !mPages.isEmpty() )
    {
      formattedReference += u", "_s + mPages;
    }

    formattedReference += '.'_L1;
  }
  else
  {
    if ( !mPublisher.isEmpty() )
    {
      if ( !formattedReference.isEmpty() )
      {
        formattedReference += ' '_L1;
      }
      formattedReference += mPublisher + u"."_s;
    }

    if ( !mPages.isEmpty() )
    {
      if ( !formattedReference.isEmpty() )
      {
        formattedReference += ' '_L1;
      }
      formattedReference += mPages + u"."_s;
    }
  }

  if ( !mUrl.isEmpty() )
  {
    if ( !formattedReference.isEmpty() )
    {
      formattedReference += ' '_L1;
    }
    if ( asHtml )
    {
      formattedReference += u"<a href=\"%1\">%1</a>"_s.arg( mUrl );
    }
    else
    {
      formattedReference += mUrl;
    }
  }

  return formattedReference;
}
