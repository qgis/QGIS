/***************************************************************************
                             qgsreportsectionlayout.cpp
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

#include "qgsreportsectionlayout.h"
#include "qgslayout.h"

///@cond NOT_STABLE

QgsReportSectionLayout::QgsReportSectionLayout( QgsAbstractReportSection *parent )
  : QgsAbstractReportSection( parent )
{}

QgsReportSectionLayout *QgsReportSectionLayout::clone() const
{
  std::unique_ptr< QgsReportSectionLayout > copy = qgis::make_unique< QgsReportSectionLayout >( nullptr );
  copyCommonProperties( copy.get() );

  if ( mBody )
  {
    copy->mBody.reset( mBody->clone() );
  }
  else
    copy->mBody.reset();

  return copy.release();
}

bool QgsReportSectionLayout::beginRender()
{
  mExportedBody = false;
  return QgsAbstractReportSection::beginRender();
}

QgsLayout *QgsReportSectionLayout::nextBody( bool &ok )
{
  if ( !mExportedBody && mBody )
  {
    mExportedBody = true;
    ok = true;
    return mBody.get();
  }
  else
  {
    ok = false;
    return nullptr;
  }
}

///@endcond

