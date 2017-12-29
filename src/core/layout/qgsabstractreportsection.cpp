/***************************************************************************
                             qgsabstractreportsection.cpp
                             --------------------
    begin                : December 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsabstractreportsection.h"
#include "qgslayout.h"

QgsAbstractReportSection::~QgsAbstractReportSection()
{
  qDeleteAll( mChildren );
}

QString QgsAbstractReportSection::filePath( const QString &baseFilePath, const QString &extension )
{
  QString base = QDir( baseFilePath ).filePath( "report_" ) + QString::number( mSectionNumber );
  if ( !extension.startsWith( '.' ) )
    base += '.';
  base += extension;
  return base;
}

QgsLayout *QgsAbstractReportSection::layout()
{
  return mCurrentLayout;
}

bool QgsAbstractReportSection::beginRender()
{
  // reset this section
  mCurrentLayout = nullptr;
  mNextChild = 0;
  mNextSection = Header;
  mSectionNumber = 0;

  // and all children too
  bool result = true;
  for ( QgsAbstractReportSection *child : qgis::as_const( mChildren ) )
  {
    result = result && child->beginRender();
  }
  return result;
}

bool QgsAbstractReportSection::next()
{
  mSectionNumber++;

  switch ( mNextSection )
  {
    case Header:
    {
      // regardless of whether we have a header or not, the next section will be the body
      mNextSection = Body;

      // if we have a header, then the current section will be the header
      if ( mHeaderEnabled && mHeader )
      {
        mCurrentLayout = mHeader.get();
        return true;
      }

      // but if not, then the current section is the body
      FALLTHROUGH;
    }

    case Body:
    {
      // if we have a body, use it
      if ( QgsLayout *body = nextBody() )
      {
        mCurrentLayout = body;
        return true;
      }

      mNextSection = Children;
      FALLTHROUGH;
    }

    case Children:
    {
      // we iterate through all the section's children...
      while ( mNextChild < mChildren.count() )
      {
        // ... staying on the current child only while it still has content for us
        if ( mChildren.at( mNextChild )->next() )
        {
          mCurrentLayout = mChildren.at( mNextChild )->layout();
          return true;
        }
        else
        {
          // no more content for this child, so move to next child
          mNextChild++;
        }
      }

      // all children have spent their content, so move to the footer
      mNextSection = Footer;
      FALLTHROUGH;
    }

    case Footer:
    {
      // regardless of whether we have a footer or not, this is the last section
      mNextSection = End;

      // if we have a footer, then the current section will be the footer
      if ( mFooterEnabled && mFooter )
      {
        mCurrentLayout = mFooter.get();
        return true;
      }

      // if not, then we're all done
      FALLTHROUGH;
    }

    case End:
      break;
  }

  mCurrentLayout = nullptr;
  return false;
}

bool QgsAbstractReportSection::endRender()
{
  // reset this section
  mCurrentLayout = nullptr;
  mNextChild = 0;
  mNextSection = Header;

  // and all children too
  bool result = true;
  for ( QgsAbstractReportSection *child : qgis::as_const( mChildren ) )
  {
    result = result && child->endRender();
  }
  return result;
}

QgsAbstractReportSection *QgsAbstractReportSection::child( int index )
{
  return mChildren.value( index );
}

void QgsAbstractReportSection::appendChild( QgsAbstractReportSection *section )
{
  mChildren.append( section );
}

void QgsAbstractReportSection::insertChild( int index, QgsAbstractReportSection *section )
{
  index = std::max( 0, index );
  index = std::min( index, mChildren.count() );
  mChildren.insert( index, section );
}

void QgsAbstractReportSection::removeChild( QgsAbstractReportSection *section )
{
  mChildren.removeAll( section );
  delete section;
}

void QgsAbstractReportSection::removeChildAt( int index )
{
  if ( index < 0 || index >= mChildren.count() )
    return;

  QgsAbstractReportSection *section = mChildren.at( index );
  removeChild( section );
}

void QgsAbstractReportSection::copyCommonProperties( QgsAbstractReportSection *destination ) const
{
  destination->mHeaderEnabled = mHeaderEnabled;
  if ( mHeader )
    destination->mHeader.reset( mHeader->clone() );
  else
    destination->mHeader.reset();

  destination->mFooterEnabled = mFooterEnabled;
  if ( mFooter )
    destination->mFooter.reset( mFooter->clone() );
  else
    destination->mFooter.reset();

  qDeleteAll( destination->mChildren );
  destination->mChildren.clear();

  for ( QgsAbstractReportSection *child : qgis::as_const( mChildren ) )
  {
    destination->mChildren.append( child->clone() );
  }
}


// QgsReport

QgsReport *QgsReport::clone() const
{
  std::unique_ptr< QgsReport > copy = qgis::make_unique< QgsReport >();
  copyCommonProperties( copy.get() );
  return copy.release();
}

//
// QgsReportSectionLayout
//

QgsReportSectionLayout *QgsReportSectionLayout::clone() const
{
  std::unique_ptr< QgsReportSectionLayout > copy = qgis::make_unique< QgsReportSectionLayout >();
  copyCommonProperties( copy.get() );

  if ( mBody )
    copy->mBody.reset( mBody->clone() );
  else
    copy->mBody.reset();

  return copy.release();
}

bool QgsReportSectionLayout::beginRender()
{
  mExportedBody = false;
  return QgsAbstractReportSection::beginRender();
}

QgsLayout *QgsReportSectionLayout::nextBody()
{
  if ( !mExportedBody && mBody )
  {
    mExportedBody = true;
    return mBody.get();
  }
  else
  {
    return nullptr;
  }
}
