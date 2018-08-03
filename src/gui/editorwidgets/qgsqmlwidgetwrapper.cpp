/***************************************************************************
  qgsqmlwidgetwrapper.cpp

 ---------------------
 begin                : 25.6.2018
 copyright            : (C) 2018 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsqmlwidgetwrapper.h"
#include "qgsmessagelog.h"
#include <QtQuickWidgets/QQuickWidget>

QgsQmlWidgetWrapper::QgsQmlWidgetWrapper( QgsVectorLayer *layer, QWidget *editor, QWidget *parent )
  : QgsWidgetWrapper( layer, editor, parent )
{

}

bool QgsQmlWidgetWrapper::valid() const
{
  return true;
}

QWidget *QgsQmlWidgetWrapper::createWidget( QWidget *parent )
{
  return new QQuickWidget( parent );
}

void QgsQmlWidgetWrapper::initWidget( QWidget *editor )
{
  QQuickWidget *quickWidget = qobject_cast<QQuickWidget *>( editor );

  if ( !quickWidget )
    return;

  if ( !mQmlFile.open() )
  {
    QgsMessageLog::logMessage( tr( "Failed to open temporary QML file" ) );
    return;
  }

  QString qmlCode = QStringLiteral(
                      "import QtQuick 2.0"
                      "import QtCharts 2.0"
                      ""
                      "ChartView {"
                      "    width: 600"
                      "    height: 400"
                      ""
                      "    PieSeries {"
                      "        id: pieSeries"
                      "        PieSlice { label: \"outlet 1\"; value: attributes.outlet_1 }"
                      "        PieSlice { label: \"outlet 2\"; value: attributes.outlet_2 }"
                      "        PieSlice { label: \"outlet 3\"; value: attributes.outlet_3 }"
                      "        PieSlice { label: \"outlet 4\"; value: attributes.outlet_4 }"
                      "    }"
                      "}" );

  mQmlFile.write( qmlCode.toUtf8() );

  quickWidget->setSource( QUrl::fromLocalFile( mQmlFile.fileName() ) );
}

void QgsQmlWidgetWrapper::setFeature( const QgsFeature &feature )
{

}
