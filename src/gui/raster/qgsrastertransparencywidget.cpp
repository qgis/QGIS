/***************************************************************************
    qgsrastertransparencywidget.cpp
    ---------------------
    begin                : May 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QWidget>
#include <QIntValidator>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QRegularExpression>

#include "qgssettings.h"
#include "qgsrastertransparencywidget.h"
#include "moc_qgsrastertransparencywidget.cpp"
#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrastertransparency.h"
#include "qgsmaptoolemitpoint.h"
#include "qgsmapsettings.h"
#include "qgsrectangle.h"
#include "qgsmapcanvas.h"
#include "qgsrasteridentifyresult.h"
#include "qgsdoublevalidator.h"
#include "qgsexpressioncontextutils.h"
#include "qgsrasterrenderer.h"
#include "qgstemporalcontroller.h"

QgsRasterTransparencyWidget::QgsRasterTransparencyWidget( QgsRasterLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
  , TRSTRING_NOT_SET( tr( "Not Set" ) )
  , mRasterLayer( layer )
  , mMapCanvas( canvas )
{
  setupUi( this );
  connect( pbnAddValuesFromDisplay, &QToolButton::clicked, this, &QgsRasterTransparencyWidget::pbnAddValuesFromDisplay_clicked );
  connect( pbnAddValuesManually, &QToolButton::clicked, this, &QgsRasterTransparencyWidget::pbnAddValuesManually_clicked );
  connect( pbnDefaultValues, &QToolButton::clicked, this, &QgsRasterTransparencyWidget::pbnDefaultValues_clicked );
  connect( pbnExportTransparentPixelValues, &QToolButton::clicked, this, &QgsRasterTransparencyWidget::pbnExportTransparentPixelValues_clicked );
  connect( pbnImportTransparentPixelValues, &QToolButton::clicked, this, &QgsRasterTransparencyWidget::pbnImportTransparentPixelValues_clicked );
  connect( pbnRemoveSelectedRow, &QToolButton::clicked, this, &QgsRasterTransparencyWidget::pbnRemoveSelectedRow_clicked );

  mNodataColorButton->setShowNoColor( true );
  mNodataColorButton->setColorDialogTitle( tr( "Select NoData Color" ) );
  syncToLayer();

  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsPanelWidget::widgetChanged );
  connect( cboxTransparencyBand, &QgsRasterBandComboBox::bandChanged, this, &QgsPanelWidget::widgetChanged );
  connect( mSrcNoDataValueCheckBox, &QCheckBox::stateChanged, this, &QgsPanelWidget::widgetChanged );
  connect( leNoDataValue, &QLineEdit::textEdited, this, &QgsPanelWidget::widgetChanged );
  leNoDataValue->setValidator( new QgsDoubleValidator( std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), this ) );
  connect( mNodataColorButton, &QgsColorButton::colorChanged, this, &QgsPanelWidget::widgetChanged );

  mPixelSelectorTool = nullptr;
  if ( mMapCanvas )
  {
    mPixelSelectorTool = new QgsMapToolEmitPoint( mMapCanvas );
    connect( mPixelSelectorTool, &QgsMapToolEmitPoint::canvasClicked, this, &QgsRasterTransparencyWidget::pixelSelected );
  }
  else
  {
    pbnAddValuesFromDisplay->setEnabled( false );
  }

  initializeDataDefinedButton( mOpacityDDBtn, QgsRasterPipe::Property::RendererOpacity );
}

void QgsRasterTransparencyWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mContext = context;
}

QgsExpressionContext QgsRasterTransparencyWidget::createExpressionContext() const
{
  QgsExpressionContext expContext;

  if ( QgsMapCanvas *canvas = mContext.mapCanvas() )
  {
    expContext = canvas->createExpressionContext();
  }
  else
  {
    expContext << QgsExpressionContextUtils::globalScope()
               << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
               << QgsExpressionContextUtils::atlasScope( nullptr )
               << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }

  if ( mRasterLayer )
    expContext << QgsExpressionContextUtils::layerScope( mRasterLayer );

  // additional scopes
  const auto constAdditionalExpressionContextScopes = mContext.additionalExpressionContextScopes();
  for ( const QgsExpressionContextScope &scope : constAdditionalExpressionContextScopes )
  {
    expContext.appendScope( new QgsExpressionContextScope( scope ) );
  }

  return expContext;
}

void QgsRasterTransparencyWidget::syncToLayer()
{
  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
  QgsRasterRenderer *renderer = mRasterLayer->renderer();
  if ( provider )
  {
    if ( provider->dataType( 1 ) == Qgis::DataType::ARGB32
         || provider->dataType( 1 ) == Qgis::DataType::ARGB32_Premultiplied )
    {
      gboxNoDataValue->setEnabled( false );
      gboxCustomTransparency->setEnabled( false );
    }

    cboxTransparencyBand->setShowNotSetOption( true, tr( "None" ) );
    cboxTransparencyBand->setLayer( mRasterLayer );
    if ( provider->sourceHasNoDataValue( 1 ) )
    {
      lblSrcNoDataValue->setText( QgsRasterBlock::printValue( provider->sourceNoDataValue( 1 ) ) );
    }
    else
    {
      lblSrcNoDataValue->setText( tr( "not defined" ) );
    }

    mSrcNoDataValueCheckBox->setChecked( provider->useSourceNoDataValue( 1 ) );

    const bool enableSrcNoData = provider->sourceHasNoDataValue( 1 ) && !std::isnan( provider->sourceNoDataValue( 1 ) );

    mSrcNoDataValueCheckBox->setEnabled( enableSrcNoData );
    lblSrcNoDataValue->setEnabled( enableSrcNoData );
  }

  if ( renderer )
  {
    if ( renderer->nodataColor().isValid() )
      mNodataColorButton->setColor( renderer->nodataColor() );
    else
      mNodataColorButton->setToNull();

    mOpacityWidget->setOpacity( renderer->opacity() );

    cboxTransparencyBand->setBand( renderer->alphaBand() );
  }

  if ( provider )
  {
    const QgsRasterRangeList noDataRangeList = provider->userNoDataValues( 1 );
    QgsDebugMsgLevel( QStringLiteral( "noDataRangeList.size = %1" ).arg( noDataRangeList.size() ), 2 );
    if ( !noDataRangeList.isEmpty() )
    {
      const double v = QgsRasterBlock::printValue( noDataRangeList.value( 0 ).min() ).toDouble();
      leNoDataValue->setText( QLocale().toString( v, 'g', 20 ) );
    }
    else
    {
      leNoDataValue->setText( QString() );
    }
  }
  else
  {
    leNoDataValue->setText( QString() );
  }

  mPropertyCollection = mRasterLayer->pipe()->dataDefinedProperties();
  updateDataDefinedButtons();

  populateTransparencyTable( mRasterLayer->renderer() );
}

void QgsRasterTransparencyWidget::transparencyCellTextEdited( const QString &text )
{
  Q_UNUSED( text )
  QgsDebugMsgLevel( QStringLiteral( "text = %1" ).arg( text ), 2 );

  switch ( mCurrentMode )
  {
    case Mode::SingleBand:
    {
      QLineEdit *lineEdit = qobject_cast<QLineEdit *>( sender() );
      if ( !lineEdit )
        return;
      int row = -1;
      int column = -1;
      for ( int r = 0; r < tableTransparency->rowCount(); r++ )
      {
        for ( int c = 0; c < tableTransparency->columnCount(); c++ )
        {
          if ( tableTransparency->cellWidget( r, c ) == sender() )
          {
            row = r;
            column = c;
            break;
          }
        }
        if ( row != -1 )
          break;
      }
      QgsDebugMsgLevel( QStringLiteral( "row = %1 column =%2" ).arg( row ).arg( column ), 2 );

      if ( column == static_cast<int>( SingleBandTableColumns::From ) )
      {
        QLineEdit *toLineEdit = dynamic_cast<QLineEdit *>( tableTransparency->cellWidget( row, static_cast<int>( SingleBandTableColumns::To ) ) );
        if ( !toLineEdit )
          return;

        const bool toChanged = mTransparencyToEdited.value( row );
        QgsDebugMsgLevel( QStringLiteral( "toChanged = %1" ).arg( toChanged ), 2 );
        if ( !toChanged )
        {
          toLineEdit->setText( lineEdit->text() );
        }
      }
      else if ( column == static_cast<int>( SingleBandTableColumns::To ) )
      {
        setTransparencyToEdited( row );
      }
      break;
    }

    case Mode::RgbBands:
      break;
  }

  emit widgetChanged();
}

void QgsRasterTransparencyWidget::pbnAddValuesFromDisplay_clicked()
{
  if ( mMapCanvas && mPixelSelectorTool )
  {
    mMapCanvas->setMapTool( mPixelSelectorTool );
  }
}

void QgsRasterTransparencyWidget::pbnAddValuesManually_clicked()
{
  QgsRasterRenderer *renderer = mRasterLayer->renderer();
  if ( !renderer )
  {
    return;
  }

  tableTransparency->insertRow( tableTransparency->rowCount() );

  int n = 0;
  switch ( mCurrentMode )
  {
    case Mode::SingleBand:
      n = 2; // set both From and To columns
      break;

    case Mode::RgbBands:
      n = 3;
      break;
  }

  for ( int i = 0; i < n; i++ )
  {
    setTransparencyCell( tableTransparency->rowCount() - 1, i, std::numeric_limits<double>::quiet_NaN() );
  }

  switch ( mCurrentMode )
  {
    case Mode::SingleBand:
      setTransparencyCell( tableTransparency->rowCount() - 1, static_cast<int>( SingleBandTableColumns::Opacity ), 100 );
      break;

    case Mode::RgbBands:
      setTransparencyCell( tableTransparency->rowCount() - 1, static_cast<int>( RgbBandTableColumns::Opacity ), 100 );
      break;
  }

  //tableTransparency->resizeColumnsToContents();
  //tableTransparency->resizeRowsToContents();
}

void QgsRasterTransparencyWidget::pbnDefaultValues_clicked()
{
  QgsRasterRenderer *r = mRasterLayer->renderer();
  if ( !r )
  {
    return;
  }

  const int nBands = r->usesBands().size();

  setupTransparencyTable( nBands );

  //tableTransparency->resizeColumnsToContents(); // works only with values
  //tableTransparency->resizeRowsToContents();
}

void QgsRasterTransparencyWidget::pbnExportTransparentPixelValues_clicked()
{
  const QgsSettings myQSettings;
  const QString myLastDir = myQSettings.value( QStringLiteral( "lastRasterFileFilterDir" ), QDir::homePath() ).toString();
  QString myFileName = QFileDialog::getSaveFileName( this, tr( "Save Pixel Values as File" ), myLastDir, tr( "Textfile" ) + " (*.txt)" );
  if ( !myFileName.isEmpty() )
  {
    if ( !myFileName.endsWith( QLatin1String( ".txt" ), Qt::CaseInsensitive ) )
    {
      myFileName = myFileName + ".txt";
    }

    QFile myOutputFile( myFileName );
    if ( myOutputFile.open( QFile::WriteOnly | QIODevice::Truncate ) )
    {
      QTextStream myOutputStream( &myOutputFile );
      myOutputStream << "# " << tr( "QGIS Generated Transparent Pixel Value Export File" ) << '\n';
      switch ( mCurrentMode )
      {
        case Mode::RgbBands:
        {
          myOutputStream << "#\n#\n# " << tr( "Red" ) << "\t" << tr( "Green" ) << "\t" << tr( "Blue" ) << "\t" << tr( "Percent Transparent" );
          for ( int myTableRunner = 0; myTableRunner < tableTransparency->rowCount(); myTableRunner++ )
          {
            myOutputStream << '\n'
                           << QString::number( transparencyCellValue( myTableRunner, static_cast<int>( RgbBandTableColumns::Red ) ) ) << "\t"
                           << QString::number( transparencyCellValue( myTableRunner, static_cast<int>( RgbBandTableColumns::Green ) ) ) << "\t"
                           << QString::number( transparencyCellValue( myTableRunner, static_cast<int>( RgbBandTableColumns::Blue ) ) ) << "\t"
                           << QString::number( transparencyCellValue( myTableRunner, static_cast<int>( RgbBandTableColumns::Opacity ) ) );
          }
          break;
        }
        case Mode::SingleBand:
        {
          myOutputStream << "#\n#\n# " << tr( "Value" ) << "\t" << tr( "Percent Transparent" );

          for ( int myTableRunner = 0; myTableRunner < tableTransparency->rowCount(); myTableRunner++ )
          {
            myOutputStream << '\n'
                           << QString::number( transparencyCellValue( myTableRunner, static_cast<int>( SingleBandTableColumns::From ) ) ) << "\t"
                           << QString::number( transparencyCellValue( myTableRunner, static_cast<int>( SingleBandTableColumns::To ) ) ) << "\t"
                           << QString::number( transparencyCellValue( myTableRunner, static_cast<int>( SingleBandTableColumns::Opacity ) ) );
          }
          break;
        }
      }
    }
    else
    {
      QMessageBox::warning( this, tr( "Save Pixel Values as File" ), tr( "Write access denied. Adjust the file permissions and try again.\n\n" ) );
    }
  }
}

void QgsRasterTransparencyWidget::pbnImportTransparentPixelValues_clicked()
{
  int myLineCounter = 0;
  bool myImportError = false;
  QString myBadLines;
  const QgsSettings myQSettings;
  const QString myLastDir = myQSettings.value( QStringLiteral( "lastRasterFileFilterDir" ), QDir::homePath() ).toString();
  const QString myFileName = QFileDialog::getOpenFileName( this, tr( "Load Pixel Values from File" ), myLastDir, tr( "Textfile" ) + " (*.txt)" );
  QFile myInputFile( myFileName );

  const thread_local QRegularExpression sRxWhitespace( "\\s+" );

  if ( myInputFile.open( QFile::ReadOnly ) )
  {
    QTextStream myInputStream( &myInputFile );
    QString myInputLine;
    switch ( mCurrentMode )
    {
      case Mode::RgbBands:
      {
        for ( int myTableRunner = tableTransparency->rowCount() - 1; myTableRunner >= 0; myTableRunner-- )
        {
          tableTransparency->removeRow( myTableRunner );
        }

        while ( !myInputStream.atEnd() )
        {
          myLineCounter++;
          myInputLine = myInputStream.readLine();
          if ( !myInputLine.isEmpty() )
          {
            if ( !myInputLine.simplified().startsWith( '#' ) )
            {
              QStringList myTokens = myInputLine.split( sRxWhitespace, Qt::SkipEmptyParts );
              if ( myTokens.count() != 4 )
              {
                myImportError = true;
                myBadLines = myBadLines + QString::number( myLineCounter ) + ":\t[" + myInputLine + "]\n";
              }
              else
              {
                tableTransparency->insertRow( tableTransparency->rowCount() );
                setTransparencyCell( tableTransparency->rowCount() - 1, static_cast<int>( RgbBandTableColumns::Red ), myTokens[0].toDouble() );
                setTransparencyCell( tableTransparency->rowCount() - 1, static_cast<int>( RgbBandTableColumns::Green ), myTokens[1].toDouble() );
                setTransparencyCell( tableTransparency->rowCount() - 1, static_cast<int>( RgbBandTableColumns::Blue ), myTokens[2].toDouble() );
                setTransparencyCell( tableTransparency->rowCount() - 1, static_cast<int>( RgbBandTableColumns::Tolerance ), 0 );
                setTransparencyCell( tableTransparency->rowCount() - 1, static_cast<int>( RgbBandTableColumns::Opacity ), myTokens[3].toDouble() );
              }
            }
          }
        }
        break;
      }
      case Mode::SingleBand:
      {
        for ( int myTableRunner = tableTransparency->rowCount() - 1; myTableRunner >= 0; myTableRunner-- )
        {
          tableTransparency->removeRow( myTableRunner );
        }

        while ( !myInputStream.atEnd() )
        {
          myLineCounter++;
          myInputLine = myInputStream.readLine();
          if ( !myInputLine.isEmpty() )
          {
            if ( !myInputLine.simplified().startsWith( '#' ) )
            {
              QStringList myTokens = myInputLine.split( sRxWhitespace, Qt::SkipEmptyParts );
              if ( myTokens.count() != 3 && myTokens.count() != 2 ) // 2 for QGIS < 1.9 compatibility
              {
                myImportError = true;
                myBadLines = myBadLines + QString::number( myLineCounter ) + ":\t[" + myInputLine + "]\n";
              }
              else
              {
                if ( myTokens.count() == 2 )
                {
                  myTokens.insert( 1, myTokens[0] ); // add 'to' value, QGIS < 1.9 compatibility
                }
                tableTransparency->insertRow( tableTransparency->rowCount() );

                setTransparencyCell( tableTransparency->rowCount() - 1, static_cast<int>( SingleBandTableColumns::From ), myTokens[0].toDouble() );
                setTransparencyCell( tableTransparency->rowCount() - 1, static_cast<int>( SingleBandTableColumns::To ), myTokens[1].toDouble() );
                setTransparencyCell( tableTransparency->rowCount() - 1, static_cast<int>( SingleBandTableColumns::Opacity ), myTokens[2].toDouble() );
              }
            }
          }
        }
        break;
      }
    }

    if ( myImportError )
    {
      QMessageBox::warning( this, tr( "Load Pixel Values from File" ), tr( "The following lines contained errors\n\n%1" ).arg( myBadLines ) );
    }
  }
  else if ( !myFileName.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Load Pixel Values from File" ), tr( "Read access denied. Adjust the file permissions and try again.\n\n" ) );
  }
  //tableTransparency->resizeColumnsToContents();
  //tableTransparency->resizeRowsToContents();
  emit widgetChanged();
}

void QgsRasterTransparencyWidget::pbnRemoveSelectedRow_clicked()
{
  if ( 0 < tableTransparency->rowCount() )
  {
    tableTransparency->removeRow( tableTransparency->currentRow() );
  }
  emit widgetChanged();
}

void QgsRasterTransparencyWidget::apply()
{
  applyToRasterProvider( mRasterLayer->dataProvider() );
  applyToRasterRenderer( mRasterLayer->renderer() );
  mRasterLayer->pipe()->setDataDefinedProperties( mPropertyCollection );
}

void QgsRasterTransparencyWidget::applyToRasterProvider( QgsRasterDataProvider *provider )
{
  //set NoDataValue
  QgsRasterRangeList myNoDataRangeList;
  if ( !leNoDataValue->text().isEmpty() )
  {
    bool myDoubleOk = false;
    const double myNoDataValue = QgsDoubleValidator::toDouble( leNoDataValue->text(), &myDoubleOk );
    if ( myDoubleOk )
    {
      const QgsRasterRange myNoDataRange( myNoDataValue, myNoDataValue );
      myNoDataRangeList << myNoDataRange;
    }
  }
  if ( provider )
  {
    for ( int bandNo = 1; bandNo <= provider->bandCount(); bandNo++ )
    {
      provider->setUserNoDataValue( bandNo, myNoDataRangeList );
      provider->setUseSourceNoDataValue( bandNo, mSrcNoDataValueCheckBox->isChecked() );
    }
  }
}

void QgsRasterTransparencyWidget::applyToRasterRenderer( QgsRasterRenderer *rasterRenderer )
{
  if ( rasterRenderer )
  {
    rasterRenderer->setAlphaBand( cboxTransparencyBand->currentBand() );
    rasterRenderer->setNodataColor( mNodataColorButton->color() );

    //Walk through each row in table and test value. If not valid set to 0.0 and continue building transparency list
    QgsRasterTransparency *rasterTransparency = new QgsRasterTransparency();
    switch ( mCurrentMode )
    {
      case Mode::RgbBands:
      {
        QVector<QgsRasterTransparency::TransparentThreeValuePixel> myTransparentThreeValuePixelList;
        myTransparentThreeValuePixelList.reserve( tableTransparency->rowCount() );
        for ( int myListRunner = 0; myListRunner < tableTransparency->rowCount(); myListRunner++ )
        {
          const double red = transparencyCellValue( myListRunner, static_cast<int>( RgbBandTableColumns::Red ) );
          const double green = transparencyCellValue( myListRunner, static_cast<int>( RgbBandTableColumns::Green ) );
          const double blue = transparencyCellValue( myListRunner, static_cast<int>( RgbBandTableColumns::Blue ) );
          const double opacity = 1.0 - transparencyCellValue( myListRunner, static_cast<int>( RgbBandTableColumns::Opacity ) ) / 100.0;
          const double tolerance = transparencyCellValue( myListRunner, static_cast<int>( RgbBandTableColumns::Tolerance ) );
          myTransparentThreeValuePixelList.append(
            QgsRasterTransparency::TransparentThreeValuePixel( red, green, blue, opacity, !qgsDoubleNear( tolerance, 0 ) ? tolerance : 4 * std::numeric_limits<double>::epsilon(), !qgsDoubleNear( tolerance, 0 ) ? tolerance : 4 * std::numeric_limits<double>::epsilon(), !qgsDoubleNear( tolerance, 0 ) ? tolerance : 4 * std::numeric_limits<double>::epsilon() )
          );
        }
        rasterTransparency->setTransparentThreeValuePixelList( myTransparentThreeValuePixelList );
        break;
      }
      case Mode::SingleBand:
      {
        QVector<QgsRasterTransparency::TransparentSingleValuePixel> myTransparentSingleValuePixelList;
        myTransparentSingleValuePixelList.reserve( tableTransparency->rowCount() );
        for ( int myListRunner = 0; myListRunner < tableTransparency->rowCount(); myListRunner++ )
        {
          const double min = transparencyCellValue( myListRunner, static_cast<int>( SingleBandTableColumns::From ) );
          const double max = transparencyCellValue( myListRunner, static_cast<int>( SingleBandTableColumns::To ) );
          const double opacity = 1.0 - transparencyCellValue( myListRunner, static_cast<int>( SingleBandTableColumns::Opacity ) ) / 100.0;

          myTransparentSingleValuePixelList.append(
            QgsRasterTransparency::TransparentSingleValuePixel( min, max, opacity )
          );
        }
        rasterTransparency->setTransparentSingleValuePixelList( myTransparentSingleValuePixelList );
        break;
      }
    }

    rasterRenderer->setRasterTransparency( rasterTransparency );

    //set global transparency
    rasterRenderer->setOpacity( mOpacityWidget->opacity() );
  }
}

void QgsRasterTransparencyWidget::initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsRasterPipe::Property key )
{
  button->blockSignals( true );
  button->init( static_cast<int>( key ), mPropertyCollection, QgsRasterPipe::propertyDefinitions(), nullptr );
  connect( button, &QgsPropertyOverrideButton::changed, this, &QgsRasterTransparencyWidget::updateProperty );
  button->registerExpressionContextGenerator( this );
  button->blockSignals( false );
}

void QgsRasterTransparencyWidget::updateDataDefinedButtons()
{
  const auto propertyOverrideButtons { findChildren<QgsPropertyOverrideButton *>() };
  for ( QgsPropertyOverrideButton *button : propertyOverrideButtons )
  {
    updateDataDefinedButton( button );
  }
}

void QgsRasterTransparencyWidget::updateDataDefinedButton( QgsPropertyOverrideButton *button )
{
  if ( !button )
    return;

  if ( button->propertyKey() < 0 )
    return;

  const QgsRasterPipe::Property key = static_cast<QgsRasterPipe::Property>( button->propertyKey() );
  whileBlocking( button )->setToProperty( mPropertyCollection.property( key ) );
}

void QgsRasterTransparencyWidget::updateProperty()
{
  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  const QgsRasterPipe::Property key = static_cast<QgsRasterPipe::Property>( button->propertyKey() );
  mPropertyCollection.setProperty( key, button->toProperty() );
  emit widgetChanged();
}

void QgsRasterTransparencyWidget::pixelSelected( const QgsPointXY &canvasPoint )
{
  QgsRasterRenderer *renderer = mRasterLayer->renderer();
  if ( !renderer )
  {
    return;
  }

  //Get the pixel values and add a new entry to the transparency table
  if ( mMapCanvas && mPixelSelectorTool && mRasterLayer->dataProvider() )
  {
    mMapCanvas->unsetMapTool( mPixelSelectorTool );

    const QgsMapSettings &ms = mMapCanvas->mapSettings();
    const QgsPointXY myPoint = ms.mapToLayerCoordinates( mRasterLayer, canvasPoint );

    const QgsRectangle myExtent = ms.mapToLayerCoordinates( mRasterLayer, mMapCanvas->extent() );
    const double mapUnitsPerPixel = mMapCanvas->mapUnitsPerPixel();
    const int myWidth = static_cast<int>( mMapCanvas->extent().width() / mapUnitsPerPixel );
    const int myHeight = static_cast<int>( mMapCanvas->extent().height() / mapUnitsPerPixel );

    const QMap<int, QVariant> myPixelMap = mRasterLayer->dataProvider()->identify( myPoint, Qgis::RasterIdentifyFormat::Value, myExtent, myWidth, myHeight ).results();

    const QList<int> bands = renderer->usesBands();

    QList<double> values;
    for ( int i = 0; i < bands.size(); ++i )
    {
      const int bandNo = bands.value( i );
      if ( myPixelMap.count( bandNo ) == 1 )
      {
        if ( QgsVariantUtils::isNull( myPixelMap.value( bandNo ) ) )
        {
          return; // Don't add nodata, transparent anyway
        }
        const double value = myPixelMap.value( bandNo ).toDouble();
        QgsDebugMsgLevel( QStringLiteral( "value = %1" ).arg( value, 0, 'g', 17 ), 2 );
        values.append( value );
      }
    }

    tableTransparency->insertRow( tableTransparency->rowCount() );

    switch ( mCurrentMode )
    {
      case Mode::SingleBand:
        setTransparencyCell( tableTransparency->rowCount() - 1, static_cast<int>( SingleBandTableColumns::From ), values.value( 0 ) );
        setTransparencyCell( tableTransparency->rowCount() - 1, static_cast<int>( SingleBandTableColumns::To ), values.value( 0 ) );
        setTransparencyCell( tableTransparency->rowCount() - 1, static_cast<int>( SingleBandTableColumns::Opacity ), 100 );
        break;
      case Mode::RgbBands:
        setTransparencyCell( tableTransparency->rowCount() - 1, static_cast<int>( RgbBandTableColumns::Red ), values.value( 0 ) );
        setTransparencyCell( tableTransparency->rowCount() - 1, static_cast<int>( RgbBandTableColumns::Green ), values.value( 1 ) );
        setTransparencyCell( tableTransparency->rowCount() - 1, static_cast<int>( RgbBandTableColumns::Blue ), values.value( 2 ) );
        setTransparencyCell( tableTransparency->rowCount() - 1, static_cast<int>( RgbBandTableColumns::Tolerance ), 0 );
        setTransparencyCell( tableTransparency->rowCount() - 1, static_cast<int>( RgbBandTableColumns::Opacity ), 100 );
        break;
    }
  }

  //tableTransparency->resizeColumnsToContents();
  //tableTransparency->resizeRowsToContents();
}

void QgsRasterTransparencyWidget::populateTransparencyTable( QgsRasterRenderer *renderer )
{
  if ( !mRasterLayer )
  {
    return;
  }

  if ( !renderer )
  {
    return;
  }

  const int nBands = renderer->usesBands().size();
  setupTransparencyTable( nBands );

  const QgsRasterTransparency *rasterTransparency = renderer->rasterTransparency();
  if ( !rasterTransparency )
  {
    return;
  }

  switch ( mCurrentMode )
  {
    case Mode::SingleBand:
    {
      QVector<QgsRasterTransparency::TransparentSingleValuePixel> pixelList = rasterTransparency->transparentSingleValuePixelList();
      for ( int i = 0; i < pixelList.size(); ++i )
      {
        tableTransparency->insertRow( i );
        setTransparencyCell( i, static_cast<int>( SingleBandTableColumns::From ), pixelList[i].min );
        setTransparencyCell( i, static_cast<int>( SingleBandTableColumns::To ), pixelList[i].max );
        setTransparencyCell( i, static_cast<int>( SingleBandTableColumns::Opacity ), 100 * ( 1 - pixelList[i].opacity ) );
        // break synchronization only if values differ
        if ( pixelList[i].min != pixelList[i].max )
        {
          setTransparencyToEdited( i );
        }
      }
      break;
    }
    case Mode::RgbBands:
    {
      QVector<QgsRasterTransparency::TransparentThreeValuePixel> pixelList = rasterTransparency->transparentThreeValuePixelList();
      for ( int i = 0; i < pixelList.size(); ++i )
      {
        tableTransparency->insertRow( i );
        setTransparencyCell( i, static_cast<int>( RgbBandTableColumns::Red ), pixelList[i].red );
        setTransparencyCell( i, static_cast<int>( RgbBandTableColumns::Green ), pixelList[i].green );
        setTransparencyCell( i, static_cast<int>( RgbBandTableColumns::Blue ), pixelList[i].blue );
        setTransparencyCell( i, static_cast<int>( RgbBandTableColumns::Opacity ), 100 * ( 1 - pixelList[i].opacity ) );
        // while the API supports different tolerances for red/green/blue channels, we only expose a single value here
        // If needed, we could expose the three separate tolerances in future... but be wary of UI bloat!
        setTransparencyCell( i, static_cast<int>( RgbBandTableColumns::Tolerance ), !qgsDoubleNear( pixelList[i].fuzzyToleranceRed, 0 ) ? pixelList[i].fuzzyToleranceRed : 0 );
      }
      break;
    }
  }

  tableTransparency->resizeColumnsToContents();
  tableTransparency->resizeRowsToContents();
}

void QgsRasterTransparencyWidget::setupTransparencyTable( int nBands )
{
  tableTransparency->clear();
  tableTransparency->setColumnCount( 0 );
  tableTransparency->setRowCount( 0 );
  mTransparencyToEdited.clear();

  if ( nBands == 3 )
  {
    mCurrentMode = Mode::RgbBands;
    tableTransparency->setColumnCount( static_cast<int>( RgbBandTableColumns::ColumnCount ) );
    tableTransparency->setHorizontalHeaderItem( static_cast<int>( RgbBandTableColumns::Red ), new QTableWidgetItem( tr( "Red" ) ) );
    tableTransparency->setHorizontalHeaderItem( static_cast<int>( RgbBandTableColumns::Green ), new QTableWidgetItem( tr( "Green" ) ) );
    tableTransparency->setHorizontalHeaderItem( static_cast<int>( RgbBandTableColumns::Blue ), new QTableWidgetItem( tr( "Blue" ) ) );
    tableTransparency->setHorizontalHeaderItem( static_cast<int>( RgbBandTableColumns::Tolerance ), new QTableWidgetItem( tr( "Tolerance" ) ) );
    tableTransparency->setHorizontalHeaderItem( static_cast<int>( RgbBandTableColumns::Opacity ), new QTableWidgetItem( tr( "Percent Transparent" ) ) );
  }
  else //1 band
  {
    mCurrentMode = Mode::SingleBand;
    tableTransparency->setColumnCount( static_cast<int>( SingleBandTableColumns::ColumnCount ) );
    tableTransparency->setHorizontalHeaderItem( static_cast<int>( SingleBandTableColumns::From ), new QTableWidgetItem( tr( "From" ) ) );
    tableTransparency->setHorizontalHeaderItem( static_cast<int>( SingleBandTableColumns::To ), new QTableWidgetItem( tr( "To" ) ) );
    tableTransparency->setHorizontalHeaderItem( static_cast<int>( SingleBandTableColumns::Opacity ), new QTableWidgetItem( tr( "Percent Transparent" ) ) );
  }
}

void QgsRasterTransparencyWidget::setTransparencyCell( int row, int column, double value )
{
  QgsDebugMsgLevel( QStringLiteral( "value = %1" ).arg( value, 0, 'g', 17 ), 2 );
  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
  if ( !provider )
    return;

  QLineEdit *lineEdit = new QLineEdit();
  lineEdit->setFrame( false ); // frame looks bad in table
  // Without margins row selection is not displayed (important for delete row)
  lineEdit->setContentsMargins( 1, 1, 1, 1 );

  if ( column == tableTransparency->columnCount() - 1 )
  {
    // transparency
    // Who needs transparency as floating point?
    lineEdit->setValidator( new QIntValidator( nullptr ) );
    lineEdit->setText( QString::number( static_cast<int>( value ) ) );
    connect( lineEdit, &QLineEdit::textEdited, this, &QgsPanelWidget::widgetChanged );
  }
  else
  {
    // value
    QString valueString;
    switch ( provider->sourceDataType( 1 ) )
    {
      case Qgis::DataType::Float32:
      case Qgis::DataType::Float64:
        lineEdit->setValidator( new QgsDoubleValidator( nullptr ) );
        if ( !std::isnan( value ) )
        {
          const double v = QgsRasterBlock::printValue( value ).toDouble();
          valueString = QLocale().toString( v );
        }
        break;
      default:
        lineEdit->setValidator( new QIntValidator( nullptr ) );
        if ( !std::isnan( value ) )
        {
          valueString = QString::number( static_cast<int>( value ) );
        }
        break;
    }
    lineEdit->setText( valueString );
    connect( lineEdit, &QLineEdit::textEdited, this, &QgsPanelWidget::widgetChanged );
  }
  tableTransparency->setCellWidget( row, column, lineEdit );
  adjustTransparencyCellWidth( row, column );

  if ( mCurrentMode == Mode::SingleBand && ( column == static_cast<int>( SingleBandTableColumns::From ) || column == static_cast<int>( SingleBandTableColumns::To ) ) )
  {
    connect( lineEdit, &QLineEdit::textEdited, this, &QgsRasterTransparencyWidget::transparencyCellTextEdited );
  }
  //tableTransparency->resizeColumnsToContents();
  emit widgetChanged();
}

void QgsRasterTransparencyWidget::adjustTransparencyCellWidth( int row, int column )
{
  QLineEdit *lineEdit = dynamic_cast<QLineEdit *>( tableTransparency->cellWidget( row, column ) );
  if ( !lineEdit )
    return;

  int width = std::max( lineEdit->fontMetrics().boundingRect( lineEdit->text() ).width() + 10, 100 );
  width = std::max( width, tableTransparency->columnWidth( column ) );

  lineEdit->setFixedWidth( width );
}

void QgsRasterTransparencyWidget::setTransparencyToEdited( int row )
{
  if ( row >= mTransparencyToEdited.size() )
  {
    mTransparencyToEdited.resize( row + 1 );
  }
  mTransparencyToEdited[row] = true;
}

double QgsRasterTransparencyWidget::transparencyCellValue( int row, int column )
{
  QLineEdit *lineEdit = dynamic_cast<QLineEdit *>( tableTransparency->cellWidget( row, column ) );
  if ( !lineEdit || lineEdit->text().isEmpty() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }
  return QgsDoubleValidator::toDouble( lineEdit->text() );
}

QgsMapToolEmitPoint *QgsRasterTransparencyWidget::pixelSelectorTool() const
{
  return mPixelSelectorTool;
}
