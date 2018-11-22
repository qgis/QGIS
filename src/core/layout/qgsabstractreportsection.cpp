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
#include "qgsreport.h"
#include "qgsreportsectionfieldgroup.h"
#include "qgsreportsectionlayout.h"
#include "qgsvectorlayer.h"

///@cond NOT_STABLE

QgsAbstractReportSection::QgsAbstractReportSection( QgsAbstractReportSection *parent )
  : mParent( parent )
{}

QgsAbstractReportSection::~QgsAbstractReportSection()
{
  qDeleteAll( mChildren );
}

QgsProject *QgsAbstractReportSection::project()
{
  if ( QgsReport *report = dynamic_cast< QgsReport * >( this ) )
    return report->layoutProject();

  QgsAbstractReportSection *current = this;
  while ( QgsAbstractReportSection *parent = current->parentSection() )
  {
    if ( QgsReport *report = dynamic_cast< QgsReport * >( parent ) )
      return report->layoutProject();

    current = parent;
  }
  return nullptr;
}

void QgsAbstractReportSection::setContext( const QgsReportSectionContext &context )
{
  auto setReportContext = [&context]( QgsLayout * layout )
  {
    if ( context.currentLayer )
    {
      layout->reportContext().blockSignals( true );
      layout->reportContext().setLayer( context.currentLayer );
      layout->reportContext().blockSignals( false );
    }
    layout->reportContext().setFeature( context.feature );
  };

  mContext = context;
  if ( mHeader )
    setReportContext( mHeader.get() );
  if ( mFooter )
    setReportContext( mFooter.get() );

  for ( QgsAbstractReportSection *section : qgis::as_const( mChildren ) )
  {
    section->setContext( mContext );
  }
}

bool QgsAbstractReportSection::writeXml( QDomElement &parentElement, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement element = doc.createElement( QStringLiteral( "Section" ) );
  element.setAttribute( QStringLiteral( "type" ), type() );

  element.setAttribute( QStringLiteral( "headerEnabled" ), mHeaderEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  if ( mHeader )
  {
    QDomElement headerElement = doc.createElement( QStringLiteral( "header" ) );
    headerElement.appendChild( mHeader->writeXml( doc, context ) );
    element.appendChild( headerElement );
  }
  element.setAttribute( QStringLiteral( "footerEnabled" ), mFooterEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  if ( mFooter )
  {
    QDomElement footerElement = doc.createElement( QStringLiteral( "footer" ) );
    footerElement.appendChild( mFooter->writeXml( doc, context ) );
    element.appendChild( footerElement );
  }

  for ( QgsAbstractReportSection *section : mChildren )
  {
    section->writeXml( element, doc, context );
  }

  writePropertiesToElement( element, doc, context );

  parentElement.appendChild( element );
  return true;
}

bool QgsAbstractReportSection::readXml( const QDomElement &element, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  if ( element.nodeName() != QStringLiteral( "Section" ) )
  {
    return false;
  }

  mHeaderEnabled = element.attribute( QStringLiteral( "headerEnabled" ), QStringLiteral( "0" ) ).toInt();
  mFooterEnabled = element.attribute( QStringLiteral( "footerEnabled" ), QStringLiteral( "0" ) ).toInt();
  const QDomElement headerElement = element.firstChildElement( QStringLiteral( "header" ) );
  if ( !headerElement.isNull() )
  {
    const QDomElement headerLayoutElem = headerElement.firstChild().toElement();
    std::unique_ptr< QgsLayout > header = qgis::make_unique< QgsLayout >( project() );
    header->readXml( headerLayoutElem, doc, context );
    mHeader = std::move( header );
  }
  const QDomElement footerElement = element.firstChildElement( QStringLiteral( "footer" ) );
  if ( !footerElement.isNull() )
  {
    const QDomElement footerLayoutElem = footerElement.firstChild().toElement();
    std::unique_ptr< QgsLayout > footer = qgis::make_unique< QgsLayout >( project() );
    footer->readXml( footerLayoutElem, doc, context );
    mFooter = std::move( footer );
  }

  const QDomNodeList sectionItemList = element.childNodes();
  for ( int i = 0; i < sectionItemList.size(); ++i )
  {
    const QDomElement currentSectionElem = sectionItemList.at( i ).toElement();
    if ( currentSectionElem.nodeName() != QStringLiteral( "Section" ) )
      continue;

    const QString sectionType = currentSectionElem.attribute( QStringLiteral( "type" ) );

    //TODO - eventually move this to a registry when there's enough subclasses to warrant it
    std::unique_ptr< QgsAbstractReportSection > section;
    if ( sectionType == QLatin1String( "SectionFieldGroup" ) )
    {
      section = qgis::make_unique< QgsReportSectionFieldGroup >();
    }
    else if ( sectionType == QLatin1String( "SectionLayout" ) )
    {
      section = qgis::make_unique< QgsReportSectionLayout >();
    }

    if ( section )
    {
      appendChild( section.get() );
      section->readXml( currentSectionElem, doc, context );
      ( void )section.release(); //ownership was transferred already
    }
  }

  bool result = readPropertiesFromElement( element, doc, context );
  return result;
}

void QgsAbstractReportSection::reloadSettings()
{
  if ( mHeader )
    mHeader->reloadSettings();
  if ( mFooter )
    mFooter->reloadSettings();
}

QString QgsAbstractReportSection::filePath( const QString &baseFilePath, const QString &extension )
{
  QString base = QStringLiteral( "%1_%2" ).arg( baseFilePath ).arg( mSectionNumber, 4, 10, QChar( '0' ) );
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
  reset();
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
        if ( prepareHeader() )
        {
          mCurrentLayout = mHeader.get();
          return true;
        }
      }

      // but if not, then the current section is a body
      mNextSection = Body;
      FALLTHROUGH
    }

    case Body:
    {
      mNextSection = Children;

      bool ok = false;
      // if we have a next body available, use it
      QgsLayout *body = nextBody( ok );
      if ( body )
      {
        mNextChild = 0;
        mCurrentLayout = body;
        return true;
      }

      FALLTHROUGH
    }

    case Children:
    {
      bool bodiesAvailable = false;
      do
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

        // used up all the children
        // if we have a next body available, use it
        QgsLayout *body = nextBody( bodiesAvailable );
        if ( bodiesAvailable )
        {
          mNextChild = 0;

          for ( QgsAbstractReportSection *section : qgis::as_const( mChildren ) )
          {
            section->reset();
          }
        }
        if ( body )
        {
          mCurrentLayout = body;
          return true;
        }
      }
      while ( bodiesAvailable );

      // all children and bodies have spent their content, so move to the footer
      mNextSection = Footer;
      FALLTHROUGH
    }

    case Footer:
    {
      // regardless of whether we have a footer or not, this is the last section
      mNextSection = End;

      // if we have a footer, then the current section will be the footer
      if ( mFooterEnabled && mFooter )
      {
        if ( prepareFooter() )
        {
          mCurrentLayout = mFooter.get();
          return true;
        }
      }

      // if not, then we're all done
      FALLTHROUGH
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
  reset();

  // and all children too
  bool result = true;
  for ( QgsAbstractReportSection *child : qgis::as_const( mChildren ) )
  {
    result = result && child->endRender();
  }
  return result;
}

void QgsAbstractReportSection::reset()
{
  mCurrentLayout = nullptr;
  mNextChild = 0;
  mNextSection = Header;
  for ( QgsAbstractReportSection *section : qgis::as_const( mChildren ) )
  {
    section->reset();
  }
}

bool QgsAbstractReportSection::prepareHeader()
{
  return true;
}

bool QgsAbstractReportSection::prepareFooter()
{
  return true;
}

void QgsAbstractReportSection::setHeader( QgsLayout *header )
{
  mHeader.reset( header );
}

void QgsAbstractReportSection::setFooter( QgsLayout *footer )
{
  mFooter.reset( footer );
}

int QgsAbstractReportSection::row() const
{
  if ( mParent )
    return mParent->childSections().indexOf( const_cast<QgsAbstractReportSection *>( this ) );

  return 0;
}

QgsAbstractReportSection *QgsAbstractReportSection::childSection( int index )
{
  return mChildren.value( index );
}

void QgsAbstractReportSection::appendChild( QgsAbstractReportSection *section )
{
  section->setParentSection( this );
  mChildren.append( section );
}

void QgsAbstractReportSection::insertChild( int index, QgsAbstractReportSection *section )
{
  section->setParentSection( this );
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
    destination->appendChild( child->clone() );
  }
}

bool QgsAbstractReportSection::writePropertiesToElement( QDomElement &, QDomDocument &, const QgsReadWriteContext & ) const
{
  return true;
}

bool QgsAbstractReportSection::readPropertiesFromElement( const QDomElement &, const QDomDocument &, const QgsReadWriteContext & )
{
  return true;
}

///@endcond

