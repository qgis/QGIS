/***************************************************************************
                             qgslegendpatchshapewidget.cpp
                             -----------------------------
    Date                 : April 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslegendpatchshapewidget.h"

QgsLegendPatchShapeWidget::QgsLegendPatchShapeWidget( QWidget *parent, const QgsLegendPatchShape &shape )
  : QgsPanelWidget( parent )
{
  setupUi( this );
  setPanelTitle( tr( "Legend Patch Shape" ) );

  mStyleItemsListWidget->hide(); // not functional yet
  setShape( shape );

  connect( mPreserveRatioCheckBox, &QCheckBox::toggled, this, &QgsLegendPatchShapeWidget::changed );
  connect( mShapeEdit, &QPlainTextEdit::textChanged, this, &QgsLegendPatchShapeWidget::changed );
}

QgsLegendPatchShape QgsLegendPatchShapeWidget::shape() const
{
  QgsLegendPatchShape res( mType, QgsGeometry::fromWkt( mShapeEdit->toPlainText() ), mPreserveRatioCheckBox->isChecked() );
  return res;
}

void QgsLegendPatchShapeWidget::setShape( const QgsLegendPatchShape &shape )
{
  if ( shape.geometry().asWkt() == mShapeEdit->toPlainText() && shape.preserveAspectRatio() == mPreserveRatioCheckBox->isChecked() && shape.symbolType() == mType )
    return;

  mType = shape.symbolType();
  whileBlocking( mShapeEdit )->setPlainText( shape.geometry().asWkt() );
  whileBlocking( mPreserveRatioCheckBox )->setChecked( shape.preserveAspectRatio() );
  emit changed();
}

///@endcond
