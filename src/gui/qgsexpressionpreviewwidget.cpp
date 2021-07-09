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

  mExpressionContext.setFeature( feature );
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
    QString preview = QgsExpression::formatPreviewString( value );
    if ( !mExpression.hasEvalError() )
    {
      mPreviewLabel->setText( preview );
    }

    if ( mExpression.hasParserError() || mExpression.hasEvalError() )
    {
      // if parser error was a result of missing feature, then skip the misleading parser error message
      // and instead show a friendly message, and allow the user to accept the expression anyway
      // refs https://github.com/qgis/QGIS/issues/42884
      if ( !mExpressionContext.feature().isValid() )
      {
        if ( !mExpression.referencedColumns().isEmpty() || mExpression.needsGeometry() )
        {
          mPreviewLabel->setText( tr( "No feature was found on this layer to evaluate the expression." ) );
          mPreviewLabel->setStyleSheet( QStringLiteral( "color: rgba(220, 125, 0, 255);" ) );
          emit expressionParsed( true );
          setParserError( false );
          setEvalError( false );
          return;
        }
      }

      QString errorString = mExpression.parserErrorString().replace( QLatin1String( "\n" ), QLatin1String( "<br>" ) );
      QString tooltip;
      if ( mExpression.hasParserError() )
        tooltip = QStringLiteral( "<b>%1:</b>"
                                  "%2" ).arg( tr( "Parser Errors" ), errorString );
      // Only show the eval error if there is no parser error.
      if ( !mExpression.hasParserError() && mExpression.hasEvalError() )
        tooltip += QStringLiteral( "<b>%1:</b> %2" ).arg( tr( "Eval Error" ), mExpression.evalErrorString() );

      mPreviewLabel->setText( tr( "Expression is invalid <a href=""more"">(more info)</a>" ) );
      mPreviewLabel->setStyleSheet( QStringLiteral( "color: rgba(255, 6, 10, 255);" ) );
      setExpressionToolTip( tooltip );
      emit expressionParsed( false );
      setParserError( mExpression.hasParserError() );
      setEvalError( mExpression.hasEvalError() );
    }
    else
    {
      mPreviewLabel->setStyleSheet( QString() );
      QString longerPreview = QgsExpression::formatPreviewString( value, true, 255 );
      if ( longerPreview != preview )
        setExpressionToolTip( longerPreview );
      else
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
