/***************************************************************************
    qgsexpressionpreviewwidget.cpp
     --------------------------------------
    Date                 : march 2020 - quarantine day 12
    Copyright            : (C) 2020 by Denis Rouzaud
    Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexpressionpreviewwidget.h"
#include "qgsmessageviewer.h"
#include "qgsvectorlayer.h"
#include "qgsfeaturepickerwidget.h"





QgsExpressionPreviewWidget::QgsExpressionPreviewWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );
  mPreviewLabel->clear();
  mFeaturePickerWidget->setShowBrowserButtons( true );

  connect( mFeaturePickerWidget, &QgsFeaturePickerWidget::featureChanged, this, &QgsExpressionPreviewWidget::setCurrentFeature );
  connect( mPreviewLabel, &QLabel::linkActivated, this, &QgsExpressionPreviewWidget::linkActivated );
}

void QgsExpressionPreviewWidget::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;
  mFeaturePickerWidget->setLayer( layer );
}

void QgsExpressionPreviewWidget::setExpressionText( const QString &expression )
{
  mExpressionText = expression;
  refreshPreview();
}

void QgsExpressionPreviewWidget::setCurrentFeature( const QgsFeature &feature )
{
  // todo: update the combo box if it has been set externaly?

  // force the feature to be valid, so it can evaluate an invalid feature but having its fields set
  if ( !feature.isValid() )
  {
    QgsFeature validFeature( feature );
    validFeature.setValid( true );
    mExpressionContext.setFeature( validFeature );
    mFeaturePickerWidget->setEnabled( false );
    mFeaturePickerWidget->setToolTip( tr( "No feature was found on this layer to evaluate the expression." ) );
  }
  else
  {
    mExpressionContext.setFeature( feature );
    mFeaturePickerWidget->setEnabled( true );
  }
  refreshPreview();
}

void QgsExpressionPreviewWidget::setGeomCalculator( const QgsDistanceArea &da )
{
  mDa = da;
  mUseGeomCalculator = true;
}

void QgsExpressionPreviewWidget::setExpressionContext( const QgsExpressionContext &context )
{
  mExpressionContext = context;
}

void QgsExpressionPreviewWidget::refreshPreview()
{
  // If the string is empty the expression will still "fail" although
  // we don't show the user an error as it will be confusing.
  if ( mExpressionText.isEmpty() )
  {
    mPreviewLabel->clear();
    mPreviewLabel->setStyleSheet( QString() );
    setExpressionToolTip( QString() );
    emit expressionParsed( false );
    mExpression = QgsExpression();
  }
  else
  {
    mExpression = QgsExpression( mExpressionText );

    if ( mUseGeomCalculator )
    {
      // only set an explicit geometry calculator if a call to setGeomCalculator was made. If not,
      // let the expression context handle this correctly
      mExpression.setGeomCalculator( &mDa );
    }

    QVariant value = mExpression.evaluate( &mExpressionContext );
    if ( !mExpression.hasEvalError() )
    {
      mPreviewLabel->setText( QgsExpression::formatPreviewString( value ) );
    }

    if ( mExpression.hasParserError() || mExpression.hasEvalError() )
    {
      QString errorString = mExpression.parserErrorString().replace( "\n", "<br>" );
      QString tooltip;
      if ( mExpression.hasParserError() )
        tooltip = QStringLiteral( "<b>%1:</b>"
                                  "%2" ).arg( tr( "Parser Errors" ), errorString );
      // Only show the eval error if there is no parser error.
      if ( !mExpression.hasParserError() && mExpression.hasEvalError() )
        tooltip += QStringLiteral( "<b>%1:</b> %2" ).arg( tr( "Eval Error" ), mExpression.evalErrorString() );

      mPreviewLabel->setText( tr( "Expression is invalid <a href=""more"">(more info)</a>" ) );
      mPreviewLabel->setStyleSheet( QStringLiteral( "color: rgba(255, 6, 10,  255);" ) );
      setExpressionToolTip( tooltip );
      emit expressionParsed( false );
      setParserError( mExpression.hasParserError() );
      setEvalError( mExpression.hasEvalError() );
    }
    else
    {
      mPreviewLabel->setStyleSheet( QString() );
      setExpressionToolTip( QString() );
      emit expressionParsed( true );
      setParserError( false );
      setEvalError( false );
    }
  }
}

void QgsExpressionPreviewWidget::linkActivated( const QString & )
{
  QgsMessageViewer mv( this, QgsGuiUtils::ModalDialogFlags, false );
  mv.setWindowTitle( tr( "More Info on Expression Error" ) );
  mv.setMessageAsHtml( mToolTip );
  mv.exec();
}

void QgsExpressionPreviewWidget::setExpressionToolTip( const QString &toolTip )
{
  if ( toolTip == mToolTip )
    return;

  mToolTip = toolTip;
  mPreviewLabel->setToolTip( mToolTip );
  emit toolTipChanged( mToolTip );
}

void QgsExpressionPreviewWidget::setParserError( bool parserError )
{
  if ( parserError != mParserError )
  {
    mParserError = parserError;
    emit parserErrorChanged();
  }
}
bool QgsExpressionPreviewWidget::parserError() const
{
  return mParserError;
}

void QgsExpressionPreviewWidget::setEvalError( bool evalError )
{
  if ( evalError == mEvalError )
    return;

  mEvalError = evalError;
  emit evalErrorChanged();
}

bool QgsExpressionPreviewWidget::evalError() const
{
  return mEvalError;
}
