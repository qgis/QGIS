/***************************************************************************
    qgsgeometrywidgetwrapper.cpp
     -----------------------
    Date                 : February 2023
    Copyright            : (C) 2023 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrywidgetwrapper.h"
#include "moc_qgsgeometrywidgetwrapper.cpp"
#include "qgsvectorlayer.h"
#include "qgsmessagebar.h"
#include "qgsgeometrywidget.h"
#include <QHBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QToolButton>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QUrl>

QgsGeometryWidgetWrapper::QgsGeometryWidgetWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor, QWidget *parent, QgsMessageBar *messageBar )
  : QgsEditorWidgetWrapper( layer, fieldIdx, editor, parent )
{
  Q_UNUSED( messageBar )
}


QVariant QgsGeometryWidgetWrapper::value() const
{
  if ( !mWidget )
    return QVariant();

  const QgsReferencedGeometry geomValue = mWidget->geometryValue();
  if ( geomValue.isNull() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::User );

  return QVariant::fromValue( geomValue );
}

void QgsGeometryWidgetWrapper::setEnabled( bool enabled )
{
  if ( mWidget )
    mWidget->setReadOnly( !enabled );
}

QWidget *QgsGeometryWidgetWrapper::createWidget( QWidget *parent )
{
  QgsGeometryWidget *widget = new QgsGeometryWidget( parent );
  widget->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum );
  return widget;
}

void QgsGeometryWidgetWrapper::initWidget( QWidget *editor )
{
  mWidget = qobject_cast<QgsGeometryWidget *>( editor );

  if ( mWidget )
  {
    connect( mWidget, &QgsGeometryWidget::geometryValueChanged, this, &QgsEditorWidgetWrapper::emitValueChanged );
  }
}

bool QgsGeometryWidgetWrapper::valid() const
{
  return mWidget;
}

void QgsGeometryWidgetWrapper::updateValues( const QVariant &value, const QVariantList & )
{
  QgsReferencedGeometry geom;
  if ( !QgsVariantUtils::isNull( value ) )
  {
    if ( value.userType() == qMetaTypeId<QgsReferencedGeometry>() )
    {
      geom = value.value<QgsReferencedGeometry>();
    }
    else if ( value.userType() == qMetaTypeId<QgsGeometry>() )
    {
      geom = QgsReferencedGeometry( value.value<QgsGeometry>(), QgsCoordinateReferenceSystem() );
    }
  }

  if ( mWidget )
  {
    mWidget->setGeometryValue( geom );
  }
}
