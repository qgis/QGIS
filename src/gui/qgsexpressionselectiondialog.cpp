/***************************************************************************
    qgisexpressionselectiondialog.cpp
     --------------------------------------
    Date                 : 24.1.2013
    Copyright            : (C) 2013 by Matthias kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexpressionselectiondialog.h"
#include "qgsapplication.h"
#include "qgsexpression.h"

#include <QSettings>

QgsExpressionSelectionDialog::QgsExpressionSelectionDialog( QgsVectorLayer* layer, const QString& startText, QWidget* parent )
    : QDialog( parent )
    , mLayer( layer )
{
  setupUi( this );

  setWindowTitle( QString( "Select by expression - %1" ).arg( layer->name() ) );

  mActionSelect->setIcon( QgsApplication::getThemeIcon( "/mIconExpressionSelect.svg" ) );
  mActionAddToSelection->setIcon( QgsApplication::getThemeIcon( "/mIconSelectAdd.svg" ) );
  mActionRemoveFromSelection->setIcon( QgsApplication::getThemeIcon( "/mIconSelectRemove.svg" ) );
  mActionSelectIntersect->setIcon( QgsApplication::getThemeIcon( "/mIconSelectIntersect.svg" ) );

  mButtonSelect->addAction( mActionSelect );
  mButtonSelect->addAction( mActionAddToSelection );
  mButtonSelect->addAction( mActionRemoveFromSelection );
  mButtonSelect->addAction( mActionSelectIntersect );
  mButtonSelect->setDefaultAction( mActionSelect );

  mExpressionBuilder->setLayer( layer );
  mExpressionBuilder->setExpressionText( startText );
  mExpressionBuilder->loadFieldNames();
  mExpressionBuilder->loadRecent( "Selection" );

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( mLayer );
  mExpressionBuilder->setExpressionContext( context );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/ExpressionSelectionDialog/geometry" ).toByteArray() );
}

QgsExpressionBuilderWidget* QgsExpressionSelectionDialog::expressionBuilder()
{
  return mExpressionBuilder;
}

void QgsExpressionSelectionDialog::setExpressionText( const QString& text )
{
  mExpressionBuilder->setExpressionText( text );
}

QString QgsExpressionSelectionDialog::expressionText()
{
  return mExpressionBuilder->expressionText();
}

void QgsExpressionSelectionDialog::setGeomCalculator( const QgsDistanceArea & da )
{
  // Store in child widget only.
  mExpressionBuilder->setGeomCalculator( da );
}

void QgsExpressionSelectionDialog::on_mActionSelect_triggered()
{
  mLayer->selectByExpression( mExpressionBuilder->expressionText(),
                              QgsVectorLayer::SetSelection );
  saveRecent();
}

void QgsExpressionSelectionDialog::on_mActionAddToSelection_triggered()
{
  mLayer->selectByExpression( mExpressionBuilder->expressionText(),
                              QgsVectorLayer::AddToSelection );
  saveRecent();
}

void QgsExpressionSelectionDialog::on_mActionSelectIntersect_triggered()
{
  mLayer->selectByExpression( mExpressionBuilder->expressionText(),
                              QgsVectorLayer::IntersectSelection );
  saveRecent();
}

void QgsExpressionSelectionDialog::on_mActionRemoveFromSelection_triggered()
{
  mLayer->selectByExpression( mExpressionBuilder->expressionText(),
                              QgsVectorLayer::RemoveFromSelection );
  saveRecent();
}

void QgsExpressionSelectionDialog::closeEvent( QCloseEvent *closeEvent )
{
  QDialog::closeEvent( closeEvent );

  QSettings settings;
  settings.setValue( "/Windows/ExpressionSelectionDialog/geometry", saveGeometry() );
}

void QgsExpressionSelectionDialog::on_mPbnClose_clicked()
{
  close();
}

void QgsExpressionSelectionDialog::done( int r )
{
  QDialog::done( r );
  close();
}

void QgsExpressionSelectionDialog::saveRecent()
{
  mExpressionBuilder->saveToRecent( "Selection" );
}
