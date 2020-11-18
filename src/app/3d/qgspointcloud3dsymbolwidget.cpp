/***************************************************************************
  qgspointcloud3dsymbolwidget.cpp
  ------------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Nedjima Belgacem
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloud3dsymbolwidget.h"

#include "qgspointcloudlayer.h"
#include "qgspointcloud3dsymbol.h"
#include "qgspointcloudlayer3drenderer.h"

QgsPointCloud3DSymbolWidget::QgsPointCloud3DSymbolWidget( QgsPointCloud3DSymbol *symbol, QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mPointSizeSpinBox->setClearValue( 2.0 );

  if ( symbol )
    setSymbol( symbol );

  connect( mPointSizeSpinBox, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsPointCloud3DSymbolWidget::changed );
}


void QgsPointCloud3DSymbolWidget::setSymbol( QgsPointCloud3DSymbol *symbol )
{
  mPointSizeSpinBox->setValue( symbol->pointSize() );
}

QgsPointCloud3DSymbol *QgsPointCloud3DSymbolWidget::symbol() const
{
  QgsPointCloud3DSymbol *symb = new QgsPointCloud3DSymbol;
  symb->setPointSize( mPointSizeSpinBox->value() );
  return symb;
}

