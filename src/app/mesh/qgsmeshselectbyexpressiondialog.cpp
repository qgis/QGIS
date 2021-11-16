/***************************************************************************
  qgsmeshselectbyexpressiondialog.cpp - QgsMeshSelectByExpressionDialog

 ---------------------
 begin                : 23.8.2021
 copyright            : (C) 2021 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmeshselectbyexpressiondialog.h"

#include <QAction>

#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"
#include "qgshelp.h"
#include "qgsgui.h"

QgsMeshSelectByExpressionDialog::QgsMeshSelectByExpressionDialog( QWidget *parent ):
  QDialog( parent )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  setWindowTitle( tr( "Select Mesh Elements by Expression" ) );

  mActionSelect = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpressionSelect.svg" ) ),  tr( "Select" ), this );
  mActionAddToSelection = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mIconSelectAdd.svg" ) ), tr( "Add to current selection" ), this );
  mActionRemoveFromSelection = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mIconSelectRemove.svg" ) ), tr( "Remove from current selection" ), this );

  mButtonSelect->addAction( mActionSelect );
  mButtonSelect->setDefaultAction( mActionSelect );
  mButtonSelect->addAction( mActionAddToSelection );
  mButtonSelect->addAction( mActionRemoveFromSelection );

  mComboBoxElementType->addItem( tr( "Select by Vertices" ), QgsMesh::Vertex );
  mComboBoxElementType->addItem( tr( "Select by Faces" ), QgsMesh::Face );
  QgsSettings settings;
  QgsMesh::ElementType elementType = QgsMesh::Vertex;
  if ( settings.contains( QStringLiteral( "/meshSelection/elementType" ) ) )
    elementType = static_cast<QgsMesh::ElementType>( settings.value( QStringLiteral( "/meshSelection/elementType" ) ).toInt() );

  int comboIndex = mComboBoxElementType->findData( elementType );

  if ( comboIndex >= 0 )
    mComboBoxElementType->setCurrentIndex( comboIndex );

  onElementTypeChanged();

  connect( mActionSelect, &QAction::triggered, this, [this]
  {
    emit select( mExpressionBuilder->expressionText(), Qgis::SelectBehavior::SetSelection, currentElementType() );
  } );
  connect( mActionAddToSelection, &QAction::triggered, this, [this]
  {
    emit select( mExpressionBuilder->expressionText(), Qgis::SelectBehavior::AddToSelection, currentElementType() );
  } );
  connect( mActionRemoveFromSelection, &QAction::triggered, this, [this]
  {
    emit select( mExpressionBuilder->expressionText(), Qgis::SelectBehavior::RemoveFromSelection, currentElementType() );
  } );

  connect( mActionSelect, &QAction::triggered, this, &QgsMeshSelectByExpressionDialog::saveRecent );
  connect( mActionAddToSelection, &QAction::triggered, this,  &QgsMeshSelectByExpressionDialog::saveRecent );
  connect( mActionRemoveFromSelection, &QAction::triggered, this,  &QgsMeshSelectByExpressionDialog::saveRecent );

  connect( mButtonClose, &QPushButton::clicked, this, &QgsMeshSelectByExpressionDialog::close );
  connect( mButtonZoomToSelected, &QToolButton::clicked, this, &QgsMeshSelectByExpressionDialog::zoomToSelected );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsMeshSelectByExpressionDialog::showHelp );

  connect( mComboBoxElementType, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsMeshSelectByExpressionDialog::onElementTypeChanged );

  mExpressionBuilder->setExpressionPreviewVisible( false );
}

QString QgsMeshSelectByExpressionDialog::expression() const
{
  return mExpressionBuilder->expressionText();
}

void QgsMeshSelectByExpressionDialog::showHelp() const
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html" ) );
}

void QgsMeshSelectByExpressionDialog::saveRecent() const
{
  mExpressionBuilder->expressionTree()->saveToRecent( mExpressionBuilder->expressionText(), QStringLiteral( "mesh_vertex_selection" ) );
}

void QgsMeshSelectByExpressionDialog::onElementTypeChanged() const
{
  QgsMesh::ElementType elementType = currentElementType() ;
  QgsSettings settings;
  settings.setValue( QStringLiteral( "/meshSelection/elementType" ), elementType );

  QgsExpressionContext expressionContext( {QgsExpressionContextUtils::meshExpressionScope( elementType )} );
  mExpressionBuilder->init( expressionContext, QStringLiteral( "mesh_vertex_selection" ), QgsExpressionBuilderWidget::LoadAll );
}

QgsMesh::ElementType QgsMeshSelectByExpressionDialog::currentElementType() const
{
  return static_cast<QgsMesh::ElementType>( mComboBoxElementType->currentData().toInt() );
}
