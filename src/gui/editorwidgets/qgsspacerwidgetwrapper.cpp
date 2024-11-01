/***************************************************************************
  qgsspacerwidgetwrapper.cpp - QgsSpacerWidgetWrapper

 ---------------------
 begin                : 16.1.2023
 copyright            : (C) 2023 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsspacerwidgetwrapper.h"
#include "moc_qgsspacerwidgetwrapper.cpp"
#include "qframe.h"


QgsSpacerWidgetWrapper::QgsSpacerWidgetWrapper( QgsVectorLayer *layer, QWidget *editor, QWidget *parent )
  : QgsWidgetWrapper( layer, editor, parent )
{
}

bool QgsSpacerWidgetWrapper::valid() const
{
  return true;
}

QWidget *QgsSpacerWidgetWrapper::createWidget( QWidget *parent )
{
  QFrame *hFrame = new QFrame( parent );
  hFrame->setFrameShape( mDrawLine ? QFrame::HLine : QFrame::NoFrame );
  return hFrame;
}

void QgsSpacerWidgetWrapper::setFeature( const QgsFeature &feature )
{
  Q_UNUSED( feature );
}

bool QgsSpacerWidgetWrapper::drawLine() const
{
  return mDrawLine;
}

void QgsSpacerWidgetWrapper::setDrawLine( bool drawLine )
{
  mDrawLine = drawLine;
}
