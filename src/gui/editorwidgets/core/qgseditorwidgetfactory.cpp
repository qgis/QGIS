/***************************************************************************
    qgseditorwidgetfactory.cpp
     --------------------------------------
    Date                 : 21.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgseditorwidgetfactory.h"
#include "qgsdefaultsearchwidgetwrapper.h"
#include "qgssearchwidgetwrapper.h"
#include "qgsfields.h"
#include "qgsvectordataprovider.h"

#include <QSettings>

class QgsDefaultSearchWidgetWrapper;

QgsEditorWidgetFactory::QgsEditorWidgetFactory( const QString &name )
  : mName( name )
{
}

/**
 * By default a simple QgsFilterLineEdit is returned as search widget.
 * Override in own factory to get something different than the default.
 */
QgsSearchWidgetWrapper *QgsEditorWidgetFactory::createSearchWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsDefaultSearchWidgetWrapper( vl, fieldIdx, parent );
}

QString QgsEditorWidgetFactory::name() const
{
  return mName;
}

unsigned int QgsEditorWidgetFactory::fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const
{
  Q_UNUSED( vl )
  Q_UNUSED( fieldIdx )
  return 5;
}
