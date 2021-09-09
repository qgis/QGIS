/***************************************************************************
  qgsmeshtransformcoordinatesdockwidget.cpp - QgsMeshTransformCoordinatesDockWidget

 ---------------------
 begin                : 26.8.2021
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

#include "qgsmeshtransformcoordinatesdockwidget.h"

#include "qgsgui.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmesheditor.h"
#include "qgsmeshlayer.h"
#include "qgsmeshadvancedediting.h"
#include "qgsproject.h"
#include "qgsguiutils.h"
#include "qgshelp.h"
#include "qgscoordinateutils.h"
#include "qgsapplication.h"

QgsMeshTransformCoordinatesDockWidget::QgsMeshTransformCoordinatesDockWidget( QWidget *parent ):
  QgsDockWidget( parent )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  setWindowTitle( tr( "Transform Mesh Vertices by Expression" ) );
  mExpressionLineEdits << mExpressionEditX << mExpressionEditY << mExpressionEditZ;
  mCheckBoxes << mCheckBoxX << mCheckBoxY << mCheckBoxZ;

  Q_ASSERT( mExpressionLineEdits.count() == mCheckBoxes.count() );

  for ( int i = 0; i < mExpressionLineEdits.count(); ++i )
  {
    mExpressionLineEdits.at( i )->registerExpressionContextGenerator( this );
    mExpressionLineEdits.at( i )->setEnabled( mCheckBoxes.at( i )->isChecked() );
    connect( mCheckBoxes.at( i ), &QCheckBox::toggled, mExpressionLineEdits.at( i ), &QWidget::setEnabled );

    connect( mExpressionLineEdits.at( i ), &QgsExpressionLineEdit::expressionChanged, this, &QgsMeshTransformCoordinatesDockWidget::updateButton );
    connect( mCheckBoxes.at( i ), &QCheckBox::toggled, this, &QgsMeshTransformCoordinatesDockWidget::updateButton );
  }

  connect( mButtonPreview, &QToolButton::clicked, this, &QgsMeshTransformCoordinatesDockWidget::calculate );
  connect( mButtonApply, &QPushButton::clicked, this, &QgsMeshTransformCoordinatesDockWidget::apply );
  connect( mButtonImport, &QToolButton::toggled, this, &QgsMeshTransformCoordinatesDockWidget::onImportVertexClicked );
}

QgsExpressionContext QgsMeshTransformCoordinatesDockWidget::createExpressionContext() const
{
  return QgsExpressionContext( {QgsExpressionContextUtils::meshExpressionScope( QgsMesh::Vertex )} );
}

QgsMeshVertex QgsMeshTransformCoordinatesDockWidget::transformedVertex( int i )
{
  if ( ! mInputLayer || !mIsCalculated )
    return QgsMeshVertex();

  return mTransformVertices.transformedVertex( mInputLayer, i );
}

bool QgsMeshTransformCoordinatesDockWidget::isResultValid() const
{
  return mIsResultValid;
}

bool QgsMeshTransformCoordinatesDockWidget::isCalculated() const
{
  return mIsCalculated;
}

void QgsMeshTransformCoordinatesDockWidget::setInput( QgsMeshLayer *layer, const QList<int> &vertexIndexes )
{
  mInputLayer = layer;
  mInputVertices = vertexIndexes;
  mIsCalculated = false;
  mIsResultValid = false;
  if ( !mInputLayer )
    mLabelInformation->setText( tr( "No active mesh layer" ) );
  else
  {
    if ( !mInputLayer->isEditable() )
      mLabelInformation->setText( tr( "Mesh layer \"%1\" not in edit mode" ).arg( mInputLayer->name() ) );
    else
    {
      if ( mInputVertices.count() == 0 )
        mLabelInformation->setText( tr( "No vertex selected for mesh \"%1\"" ).arg( mInputLayer->name() ) );
      else if ( mInputVertices.count() == 1 )
        mLabelInformation->setText( tr( "1 vertex of mesh layer \"%1\" to transform" ).arg( mInputLayer->name() ) );
      else
        mLabelInformation->setText( tr( "%1 vertices of mesh layer \"%2\" to transform" ).
                                    arg( QString::number( mInputVertices.count() ), mInputLayer->name() ) );
    }
  }
  importVertexCoordinates();
  updateButton();
  emit calculationUpdated();
}

void QgsMeshTransformCoordinatesDockWidget::calculate()
{
  if ( !mInputLayer || mInputVertices.isEmpty() )
    return;

  QgsTemporaryCursorOverride busyCursor( Qt::WaitCursor );
  mTransformVertices.clear();
  mTransformVertices.setInputVertices( mInputVertices );
  mTransformVertices.setExpressions( mCheckBoxX->isChecked() ? mExpressionEditX->expression() : QString(),
                                     mCheckBoxY->isChecked() ? mExpressionEditY->expression() : QString(),
                                     mCheckBoxZ->isChecked() ? mExpressionEditZ->expression() : QString() );
  QgsExpressionContext context;
  context.appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );

  mIsResultValid = mTransformVertices.calculate( mInputLayer );

  mIsCalculated = true;
  mButtonApply->setEnabled( mIsResultValid );

  emit calculationUpdated();
}

void QgsMeshTransformCoordinatesDockWidget::updateButton()
{
  mButtonApply->setEnabled( false );
  bool isCalculable = mInputLayer && !mInputVertices.isEmpty();
  if ( isCalculable )
  {
    isCalculable = false;
    for ( const QCheckBox *cb : std::as_const( mCheckBoxes ) )
      isCalculable |= cb->isChecked();

    if ( isCalculable )
    {
      for ( int i = 0; i < mCheckBoxes.count(); ++i )
      {
        bool checked = mCheckBoxes.at( i )->isChecked();
        isCalculable &= !checked || mExpressionLineEdits.at( i )->isValidExpression();
      }
    }
  }

  mButtonPreview->setEnabled( isCalculable );
}

void QgsMeshTransformCoordinatesDockWidget::apply()
{
  emit aboutToBeApplied();
  QgsTemporaryCursorOverride busyCursor( Qt::WaitCursor );
  if ( mIsResultValid && mInputLayer && mInputLayer->meshEditor() )
    mInputLayer->meshEditor()->advancedEdit( & mTransformVertices );
  emit applied();
}

void QgsMeshTransformCoordinatesDockWidget::onImportVertexClicked( bool checked )
{
  if ( checked )
    importVertexCoordinates();
  else
  {
    mExpressionEditX->setExpression( QString() );
    mExpressionEditY->setExpression( QString() );
    mExpressionEditZ->setExpression( QString() );
  }
}


QString QgsMeshTransformCoordinatesDockWidget::displayCoordinateText( const QgsCoordinateReferenceSystem &crs, double value )
{
  return QString::number( value, 'f', QgsCoordinateUtils::calculateCoordinatePrecisionForCrs( crs, QgsProject::instance() ) );
}

void QgsMeshTransformCoordinatesDockWidget::importVertexCoordinates()
{
  if ( mButtonImport->isChecked() && mInputLayer )
  {
    if ( mInputVertices.count() == 1 )
    {
      mExpressionEditX->setExpression( displayCoordinateText( mInputLayer->crs(), mInputLayer->nativeMesh()->vertex( mInputVertices.first() ).x() ) );
      mExpressionEditY->setExpression( displayCoordinateText( mInputLayer->crs(), mInputLayer->nativeMesh()->vertex( mInputVertices.first() ).y() ) );
      mExpressionEditZ->setExpression( displayCoordinateText( mInputLayer->crs(), mInputLayer->nativeMesh()->vertex( mInputVertices.first() ).z() ) );
    }
    else
    {
      mExpressionEditX->setExpression( QString() );
      mExpressionEditY->setExpression( QString() );
      mExpressionEditZ->setExpression( QString() );
    }
  }
}

