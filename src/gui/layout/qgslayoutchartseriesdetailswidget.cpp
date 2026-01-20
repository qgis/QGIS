/***************************************************************************
                         qgslayoutchartseriesdetailswidget.cpp
                         --------------------------
     begin                : August 2025
     copyright            : (C) 2025 by Mathieu
     email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutchartseriesdetailswidget.h"

#include "qgsexpressionbuilderdialog.h"

#include "moc_qgslayoutchartseriesdetailswidget.cpp"

QgsLayoutChartSeriesDetailsWidget::QgsLayoutChartSeriesDetailsWidget( QgsVectorLayer *layer, int index, const QgsLayoutItemChart::SeriesDetails &seriesDetails, QWidget *parent )
  : QgsPanelWidget( parent )
  , mVectorLayer( layer )
  , mIndex( index )
{
  setupUi( this );

  if ( mVectorLayer )
  {
    mXExpressionWidget->setLayer( mVectorLayer.data() );
    mYExpressionWidget->setLayer( mVectorLayer.data() );
  }

  mXExpressionWidget->setExpression( seriesDetails.xExpression() );
  mYExpressionWidget->setExpression( seriesDetails.yExpression() );
  mFilterLineEdit->setText( seriesDetails.filterExpression() );

  connect( mXExpressionWidget, static_cast<void ( QgsFieldExpressionWidget::* )( const QString &, bool )>( &QgsFieldExpressionWidget::fieldChanged ), this, [this]( const QString &, bool ) { emit widgetChanged(); } );
  connect( mYExpressionWidget, static_cast<void ( QgsFieldExpressionWidget::* )( const QString &, bool )>( &QgsFieldExpressionWidget::fieldChanged ), this, [this]( const QString &, bool ) { emit widgetChanged(); } );
  connect( mFilterLineEdit, &QLineEdit::textChanged, this, [this] { emit widgetChanged(); } );
  connect( mFilterButton, &QToolButton::clicked, this, &QgsLayoutChartSeriesDetailsWidget::mFilterButton_clicked );
}

int QgsLayoutChartSeriesDetailsWidget::index() const
{
  return mIndex;
}

QString QgsLayoutChartSeriesDetailsWidget::xExpression() const
{
  return mXExpressionWidget->asExpression();
}

QString QgsLayoutChartSeriesDetailsWidget::yExpression() const
{
  return mYExpressionWidget->asExpression();
}

QString QgsLayoutChartSeriesDetailsWidget::filterExpression() const
{
  return mFilterLineEdit->text();
}

void QgsLayoutChartSeriesDetailsWidget::mFilterButton_clicked()
{
  if ( !mVectorLayer )
    return;

  const QgsExpressionContext context = mVectorLayer->createExpressionContext();
  QgsExpressionBuilderDialog expressionBuilderDialog( mVectorLayer.data(), mFilterLineEdit->text(), this, u"generic"_s, context );
  expressionBuilderDialog.setWindowTitle( tr( "Expression Based Filter" ) );

  if ( expressionBuilderDialog.exec() == QDialog::Accepted )
  {
    const QString expression = expressionBuilderDialog.expressionText();
    if ( !expression.isEmpty() )
    {
      mFilterLineEdit->setText( expression );
    }
  }
}
