/***************************************************************************
                         qgssinglebandpseudocolorrendererwidget.cpp
                         ------------------------------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssinglebandpseudocolorrendererwidget.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrastershader.h"
#include "qgsrasterminmaxwidget.h"
#include "qgsdoublevalidator.h"
#include "qgstreewidgetitem.h"
#include "qgssettings.h"
#include "qgsmapcanvas.h"
#include "qgsguiutils.h"

// for color ramps - todo add rasterStyle and refactor raster vs. vector ramps
#include "qgsstyle.h"
#include "qgscolorramp.h"
#include "qgscolorrampbutton.h"
#include "qgscolordialog.h"

#include <QCursor>
#include <QPushButton>
#include <QInputDialog>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTextStream>
#include <QTreeView>

QgsSingleBandPseudoColorRendererWidget::QgsSingleBandPseudoColorRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent )
  : QgsRasterRendererWidget( layer, extent )
  , mMinMaxOrigin( 0 )
{
  const QgsSettings settings;

  setupUi( this );

  mColorRampShaderWidget->initializeForUseWithRasterLayer();

  connect( mMinLineEdit, &QLineEdit::textChanged, this, &QgsSingleBandPseudoColorRendererWidget::mMinLineEdit_textChanged );
  connect( mMaxLineEdit, &QLineEdit::textChanged, this, &QgsSingleBandPseudoColorRendererWidget::mMaxLineEdit_textChanged );
  connect( mMinLineEdit, &QLineEdit::textEdited, this, &QgsSingleBandPseudoColorRendererWidget::mMinLineEdit_textEdited );
  connect( mMaxLineEdit, &QLineEdit::textEdited, this, &QgsSingleBandPseudoColorRendererWidget::mMaxLineEdit_textEdited );

  if ( !mRasterLayer )
  {
    return;
  }

  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return;
  }

  // Must be before adding items to mBandComboBox (signal)
  mMinLineEdit->setValidator( new QgsDoubleValidator( mMinLineEdit ) );
  mMaxLineEdit->setValidator( new QgsDoubleValidator( mMaxLineEdit ) );

  mMinMaxWidget = new QgsRasterMinMaxWidget( layer, this );
  mMinMaxWidget->setExtent( extent );
  mMinMaxWidget->setMapCanvas( mCanvas );

  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  mMinMaxContainerWidget->setLayout( layout );
  layout->addWidget( mMinMaxWidget );

  mColorRampShaderWidget->setRasterDataProvider( provider );
  mBandComboBox->setLayer( mRasterLayer );

  setFromRenderer( layer->renderer() );

  connect( mMinMaxWidget, &QgsRasterMinMaxWidget::load, this, &QgsSingleBandPseudoColorRendererWidget::loadMinMax );
  connect( mMinMaxWidget, &QgsRasterMinMaxWidget::widgetChanged, this, &QgsSingleBandPseudoColorRendererWidget::widgetChanged );

  // If there is currently no min/max, load default with user current default options
  if ( mMinLineEdit->text().isEmpty() || mMaxLineEdit->text().isEmpty() )
  {
    QgsRasterMinMaxOrigin minMaxOrigin = mMinMaxWidget->minMaxOrigin();
    if ( minMaxOrigin.limits() == QgsRasterMinMaxOrigin::None )
    {
      minMaxOrigin.setLimits( QgsRasterMinMaxOrigin::MinMax );
      mMinMaxWidget->setFromMinMaxOrigin( minMaxOrigin );
    }
    mMinMaxWidget->doComputations();
  }

  whileBlocking( mColorRampShaderWidget )->setMinimumMaximum( lineEditValue( mMinLineEdit ), lineEditValue( mMaxLineEdit ) );

  connect( mBandComboBox, &QgsRasterBandComboBox::bandChanged, this, &QgsSingleBandPseudoColorRendererWidget::bandChanged );
  connect( mColorRampShaderWidget, &QgsColorRampShaderWidget::minimumMaximumChangedFromTree, this, &QgsSingleBandPseudoColorRendererWidget::loadMinMaxFromTree );
  connect( mColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged, this, &QgsSingleBandPseudoColorRendererWidget::widgetChanged );
}

QgsRasterRenderer *QgsSingleBandPseudoColorRendererWidget::renderer()
{
  QgsRasterShader *rasterShader = new QgsRasterShader();

  mColorRampShaderWidget->setMinimumMaximum( lineEditValue( mMinLineEdit ), lineEditValue( mMaxLineEdit ) );
  mColorRampShaderWidget->setExtent( mMinMaxWidget->extent() );

  QgsColorRampShader *fcn = new QgsColorRampShader( mColorRampShaderWidget->shader() );
  rasterShader->setRasterShaderFunction( fcn );

  const int bandNumber = mBandComboBox->currentBand();
  QgsSingleBandPseudoColorRenderer *renderer = new QgsSingleBandPseudoColorRenderer( mRasterLayer->dataProvider(), bandNumber, rasterShader );
  renderer->setClassificationMin( lineEditValue( mMinLineEdit ) );
  renderer->setClassificationMax( lineEditValue( mMaxLineEdit ) );
  renderer->setMinMaxOrigin( mMinMaxWidget->minMaxOrigin() );
  return renderer;
}

void QgsSingleBandPseudoColorRendererWidget::doComputations()
{
  mMinMaxWidget->doComputations();
}

QgsRasterMinMaxWidget *QgsSingleBandPseudoColorRendererWidget::minMaxWidget() { return mMinMaxWidget; }

void QgsSingleBandPseudoColorRendererWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  QgsRasterRendererWidget::setMapCanvas( canvas );
  mMinMaxWidget->setMapCanvas( canvas );
  mColorRampShaderWidget->setExtent( mMinMaxWidget->extent() );
}

void QgsSingleBandPseudoColorRendererWidget::setFromRenderer( const QgsRasterRenderer *r )
{
  const QgsSingleBandPseudoColorRenderer *pr = dynamic_cast<const QgsSingleBandPseudoColorRenderer *>( r );
  if ( pr )
  {
    mBandComboBox->setBand( pr->band() );
    mMinMaxWidget->setBands( QList< int >() << pr->band() );
    mColorRampShaderWidget->setRasterBand( pr->band() );

    // need to set min/max properties here because if we use the raster shader below,
    // we may set a new color ramp which needs to have min/max values defined.
    setLineEditValue( mMinLineEdit, pr->classificationMin() );
    setLineEditValue( mMaxLineEdit, pr->classificationMax() );
    mMinMaxWidget->setFromMinMaxOrigin( pr->minMaxOrigin() );

    const QgsRasterShader *rasterShader = pr->shader();
    if ( rasterShader )
    {
      const QgsColorRampShader *colorRampShader = dynamic_cast<const QgsColorRampShader *>( rasterShader->rasterShaderFunction() );
      if ( colorRampShader )
      {
        mColorRampShaderWidget->setFromShader( *colorRampShader );
      }
    }
  }
  else
  {
    mMinMaxWidget->setBands( QList< int >() << mBandComboBox->currentBand() );
    mColorRampShaderWidget->setRasterBand( mBandComboBox->currentBand() );
  }
}

void QgsSingleBandPseudoColorRendererWidget::bandChanged()
{
  QList<int> bands;
  bands.append( mBandComboBox->currentBand() );
  mMinMaxWidget->setBands( bands );
  mColorRampShaderWidget->setRasterBand( mBandComboBox->currentBand() );
  mColorRampShaderWidget->classify();
}

void QgsSingleBandPseudoColorRendererWidget::loadMinMax( int bandNo, double min, double max )
{
  QgsDebugMsg( QStringLiteral( "theBandNo = %1 min = %2 max = %3" ).arg( bandNo ).arg( min ).arg( max ) );

  const QString oldMinTextvalue = mMinLineEdit->text();
  const QString oldMaxTextvalue = mMaxLineEdit->text();

  if ( std::isnan( min ) )
  {
    whileBlocking( mMinLineEdit )->clear();
  }
  else
  {
    whileBlocking( mMinLineEdit )->setText( displayValueWithMaxPrecision( min ) );
  }

  if ( std::isnan( max ) )
  {
    whileBlocking( mMaxLineEdit )->clear();
  }
  else
  {
    whileBlocking( mMaxLineEdit )->setText( displayValueWithMaxPrecision( max ) );
  }

  // We compare old min and new min as text because QString::number keeps a fixed number of significant
  // digits (default 6) and so loaded min/max will always differ from current one, which triggers a
  // classification, and wipe out every user modification (see https://github.com/qgis/QGIS/issues/36172)
  if ( mMinLineEdit->text() != oldMinTextvalue || mMaxLineEdit->text() != oldMaxTextvalue )
  {
    whileBlocking( mColorRampShaderWidget )->setRasterBand( bandNo );
    whileBlocking( mColorRampShaderWidget )->setMinimumMaximumAndClassify( min, max );
  }
}


void QgsSingleBandPseudoColorRendererWidget::loadMinMaxFromTree( double min, double max )
{
  whileBlocking( mMinLineEdit )->setText( displayValueWithMaxPrecision( min ) );
  whileBlocking( mMaxLineEdit )->setText( displayValueWithMaxPrecision( max ) );
  minMaxModified();
}


void QgsSingleBandPseudoColorRendererWidget::setLineEditValue( QLineEdit *lineEdit, double value )
{
  QString s;
  if ( !std::isnan( value ) )
  {
    s = displayValueWithMaxPrecision( value );
  }
  lineEdit->setText( s );
}

double QgsSingleBandPseudoColorRendererWidget::lineEditValue( const QLineEdit *lineEdit ) const
{
  if ( lineEdit->text().isEmpty() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return QgsDoubleValidator::toDouble( lineEdit->text() );
}

void QgsSingleBandPseudoColorRendererWidget::mMinLineEdit_textEdited( const QString & )
{
  minMaxModified();
  whileBlocking( mColorRampShaderWidget )->setMinimumMaximumAndClassify( lineEditValue( mMinLineEdit ), lineEditValue( mMaxLineEdit ) );
  emit widgetChanged();
}

void QgsSingleBandPseudoColorRendererWidget::mMaxLineEdit_textEdited( const QString & )
{
  minMaxModified();
  whileBlocking( mColorRampShaderWidget )->setMinimumMaximumAndClassify( lineEditValue( mMinLineEdit ), lineEditValue( mMaxLineEdit ) );
  emit widgetChanged();
}

void QgsSingleBandPseudoColorRendererWidget::mMinLineEdit_textChanged( const QString & )
{
  whileBlocking( mColorRampShaderWidget )->setMinimumMaximum( lineEditValue( mMinLineEdit ), lineEditValue( mMaxLineEdit ) );
  emit widgetChanged();
}

void QgsSingleBandPseudoColorRendererWidget::mMaxLineEdit_textChanged( const QString & )
{
  whileBlocking( mColorRampShaderWidget )->setMinimumMaximum( lineEditValue( mMinLineEdit ), lineEditValue( mMaxLineEdit ) );
  emit widgetChanged();
}


void QgsSingleBandPseudoColorRendererWidget::minMaxModified()
{
  mMinMaxWidget->userHasSetManualMinMaxValues();
}

QString QgsSingleBandPseudoColorRendererWidget::displayValueWithMaxPrecision( const double value )
{
  if ( mRasterLayer->dataProvider() )
  {
    return QgsGuiUtils::displayValueWithMaximumDecimals( mRasterLayer->dataProvider()->dataType( mBandComboBox->currentBand() ), value );
  }
  else
  {
    // Use QLocale default
    return QLocale().toString( value, 'g' );
  }
}

void QgsSingleBandPseudoColorRendererWidget::setMin( const QString &value, int )
{
  mMinLineEdit->setText( value );
  minMaxModified();
  mColorRampShaderWidget->classify();
}

void QgsSingleBandPseudoColorRendererWidget::setMax( const QString &value, int )
{
  mMaxLineEdit->setText( value );
  minMaxModified();
  mColorRampShaderWidget->classify();
}
