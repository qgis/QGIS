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
#include "qgsstylesavedialog.h"
#include <QDialogButtonBox>
#include <QMessageBox>

QgsLegendPatchShapeWidget::QgsLegendPatchShapeWidget( QWidget *parent, const QgsLegendPatchShape &shape )
  : QgsPanelWidget( parent )
{
  setupUi( this );
  setPanelTitle( tr( "Legend Patch Shape" ) );

  mStyleItemsListWidget->setStyle( QgsStyle::defaultStyle() );
  mStyleItemsListWidget->setEntityType( QgsStyle::LegendPatchShapeEntity );
  mStyleItemsListWidget->setSymbolType( shape.symbolType() );

  setShape( shape );

  connect( mPreserveRatioCheckBox, &QCheckBox::toggled, this, &QgsLegendPatchShapeWidget::changed );
  connect( mShapeEdit, &QPlainTextEdit::textChanged, this, &QgsLegendPatchShapeWidget::changed );

  connect( mStyleItemsListWidget, &QgsStyleItemsListWidget::selectionChanged, this, &QgsLegendPatchShapeWidget::setShapeFromStyle );
  connect( mStyleItemsListWidget, &QgsStyleItemsListWidget::saveEntity, this, &QgsLegendPatchShapeWidget::saveShape );
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

void QgsLegendPatchShapeWidget::setShapeFromStyle( const QString &name, QgsStyle::StyleEntity )
{
  if ( !QgsStyle::defaultStyle()->legendPatchShapeNames().contains( name ) )
    return;

  const QgsLegendPatchShape newShape = QgsStyle::defaultStyle()->legendPatchShape( name );
  setShape( newShape );
}

void QgsLegendPatchShapeWidget::saveShape()
{
  QgsStyle *style = QgsStyle::defaultStyle();
  if ( !style )
    return;

  QgsStyleSaveDialog saveDlg( this, QgsStyle::LegendPatchShapeEntity );
  saveDlg.setDefaultTags( mStyleItemsListWidget->currentTagFilter() );
  if ( !saveDlg.exec() )
    return;

  if ( saveDlg.name().isEmpty() )
    return;

  // check if there is no shape with same name
  if ( style->legendPatchShapeNames().contains( saveDlg.name() ) )
  {
    const int res = QMessageBox::warning( this, tr( "Save Legend Patch Shape" ),
                                          tr( "A legend patch shape with the name '%1' already exists. Overwrite?" )
                                          .arg( saveDlg.name() ),
                                          QMessageBox::Yes | QMessageBox::No );
    if ( res != QMessageBox::Yes )
    {
      return;
    }
    style->removeEntityByName( QgsStyle::LegendPatchShapeEntity, saveDlg.name() );
  }

  const QStringList symbolTags = saveDlg.tags().split( ',' );

  const QgsLegendPatchShape newShape = shape();
  style->addLegendPatchShape( saveDlg.name(), newShape );
  style->saveLegendPatchShape( saveDlg.name(), newShape, saveDlg.isFavorite(), symbolTags );
}

//
// QgsLegendPatchShapeDialog
//

QgsLegendPatchShapeDialog::QgsLegendPatchShapeDialog( const QgsLegendPatchShape &shape, QWidget *parent )
  : QDialog( parent )
{
  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsLegendPatchShapeWidget( nullptr, shape );
  vLayout->addWidget( mWidget );
  connect( mWidget, &QgsPanelWidget::panelAccepted, this, &QDialog::reject );

  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Ok, Qt::Horizontal );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  vLayout->addWidget( mButtonBox );
  setLayout( vLayout );
  setWindowTitle( tr( "Legend Patch Shape" ) );
}

QDialogButtonBox *QgsLegendPatchShapeDialog::buttonBox() const
{
  return mButtonBox;
}
