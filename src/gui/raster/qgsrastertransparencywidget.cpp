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
#include "qgsrasterlayer.h"
#include "qgsraster.h"
#include "qgsrasterlayerrenderer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrastertransparency.h"
#include "qgsmaptoolemitpoint.h"
#include "qgsmapsettings.h"
#include "qgsrectangle.h"
#include "qgsmapcanvas.h"
#include "qgsrasteridentifyresult.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsdoublevalidator.h"
#include "qgsexpressioncontextutils.h"
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
  mNodataColorButton->setColorDialogTitle( tr( "Select No Data Color" ) );
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

  initializeDataDefinedButton( mOpacityDDBtn, QgsRasterPipe::RendererOpacity );
}

void QgsRasterTransparencyWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mContext = context;
}

QgsExpressionContext QgsRasterTransparencyWidget::createExpressionContext() const
{
  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
             << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
             << QgsExpressionContextUtils::atlasScope( nullptr );

  if ( QgsMapCanvas *canvas = mContext.mapCanvas() )
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( canvas->mapSettings() )
               << new QgsExpressionContextScope( canvas->expressionContextScope() );
    if ( const QgsExpressionContextScopeGenerator *generator = dynamic_cast< const QgsExpressionContextScopeGenerator * >( canvas->temporalController() ) )
    {
      expContext << generator->createExpressionContextScope();
    }
  }
  else
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
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
  }

  if ( mRasterLayer->dataProvider()->sourceHasNoDataValue( 1 ) )
  {
    lblSrcNoDataValue->setText( QgsRasterBlock::printValue( mRasterLayer->dataProvider()->sourceNoDataValue( 1 ) ) );
  }
  else
  {
    lblSrcNoDataValue->setText( tr( "not defined" ) );
  }

  mSrcNoDataValueCheckBox->setChecked( mRasterLayer->dataProvider()->useSourceNoDataValue( 1 ) );

  const bool enableSrcNoData = mRasterLayer->dataProvider()->sourceHasNoDataValue( 1 ) && !std::isnan( mRasterLayer->dataProvider()->sourceNoDataValue( 1 ) );

  mSrcNoDataValueCheckBox->setEnabled( enableSrcNoData );
  lblSrcNoDataValue->setEnabled( enableSrcNoData );

  if ( renderer )
  {
    if ( renderer->nodataColor().isValid() )
      mNodataColorButton->setColor( renderer->nodataColor() );
    else
      mNodataColorButton->setToNull();

    mOpacityWidget->setOpacity( renderer->opacity() );

    cboxTransparencyBand->setBand( renderer->alphaBand() );
  }

  const QgsRasterRangeList noDataRangeList = mRasterLayer->dataProvider()->userNoDataValues( 1 );
  QgsDebugMsg( QStringLiteral( "noDataRangeList.size = %1" ).arg( noDataRangeList.size() ) );
  if ( !noDataRangeList.isEmpty() )
  {
    const double v = QgsRasterBlock::printValue( noDataRangeList.value( 0 ).min() ).toDouble();
    leNoDataValue->setText( QLocale().toString( v ) );
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
  QgsDebugMsg( QStringLiteral( "text = %1" ).arg( text ) );
  QgsRasterRenderer *renderer = mRasterLayer->renderer();
  if ( !renderer )
  {
    return;
  }
  const int nBands = renderer->usesBands().size();
  if ( nBands == 1 )
  {
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>( sender() );
    if ( !lineEdit ) return;
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
      if ( row != -1 ) break;
    }
    QgsDebugMsg( QStringLiteral( "row = %1 column =%2" ).arg( row ).arg( column ) );

    if ( column == 0 )
    {
      QLineEdit *toLineEdit = dynamic_cast<QLineEdit *>( tableTransparency->cellWidget( row, 1 ) );
      if ( !toLineEdit ) return;
      const bool toChanged = mTransparencyToEdited.value( row );
      QgsDebugMsg( QStringLiteral( "toChanged = %1" ).arg( toChanged ) );
      if ( !toChanged )
      {
        toLineEdit->setText( lineEdit->text() );
      }
    }
    else if ( column == 1 )
    {
      setTransparencyToEdited( row );
    }
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

  int n = renderer->usesBands().size();
  if ( n == 1 ) n++;

  for ( int i = 0; i < n; i++ )
  {
    setTransparencyCell( tableTransparency->rowCount() - 1, i, std::numeric_limits<double>::quiet_NaN() );
  }

  setTransparencyCell( tableTransparency->rowCount() - 1, n, 100 );

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
      if ( rasterIsMultiBandColor() )
      {
        myOutputStream << "#\n#\n# " << tr( "Red" ) << "\t" << tr( "Green" ) << "\t" << tr( "Blue" ) << "\t" << tr( "Percent Transparent" );
        for ( int myTableRunner = 0; myTableRunner < tableTransparency->rowCount(); myTableRunner++ )
        {
          myOutputStream << '\n' << QString::number( transparencyCellValue( myTableRunner, 0 ) ) << "\t"
                         << QString::number( transparencyCellValue( myTableRunner, 1 ) ) << "\t"
                         << QString::number( transparencyCellValue( myTableRunner, 2 ) ) << "\t"
                         << QString::number( transparencyCellValue( myTableRunner, 3 ) );
        }
      }
      else
      {
        myOutputStream << "#\n#\n# " << tr( "Value" ) << "\t" << tr( "Percent Transparent" );

        for ( int myTableRunner = 0; myTableRunner < tableTransparency->rowCount(); myTableRunner++ )
        {
          myOutputStream << '\n' << QString::number( transparencyCellValue( myTableRunner, 0 ) ) << "\t"
                         << QString::number( transparencyCellValue( myTableRunner, 1 ) ) << "\t"
                         << QString::number( transparencyCellValue( myTableRunner, 2 ) );
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
  if ( myInputFile.open( QFile::ReadOnly ) )
  {
    QTextStream myInputStream( &myInputFile );
    QString myInputLine;
    if ( rasterIsMultiBandColor() )
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
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            QStringList myTokens = myInputLine.split( QRegExp( "\\s+" ), QString::SkipEmptyParts );
#else
            QStringList myTokens = myInputLine.split( QRegularExpression( "\\s+" ), Qt::SkipEmptyParts );
#endif
            if ( myTokens.count() != 4 )
            {
              myImportError = true;
              myBadLines = myBadLines + QString::number( myLineCounter ) + ":\t[" + myInputLine + "]\n";
            }
            else
            {
              tableTransparency->insertRow( tableTransparency->rowCount() );
              for ( int col = 0; col < 4; col++ )
              {
                setTransparencyCell( tableTransparency->rowCount() - 1, col, myTokens[col].toDouble() );
              }
            }
          }
        }
      }
    }
    else
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
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            QStringList myTokens = myInputLine.split( QRegExp( "\\s+" ), QString::SkipEmptyParts );
#else
            QStringList myTokens = myInputLine.split( QRegularExpression( "\\s+" ), Qt::SkipEmptyParts );
#endif
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
              for ( int col = 0; col < 3; col++ )
              {
                setTransparencyCell( tableTransparency->rowCount() - 1, col, myTokens[col].toDouble() );
              }
            }
          }
        }
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

bool QgsRasterTransparencyWidget::rasterIsMultiBandColor()
{
  return mRasterLayer && nullptr != dynamic_cast<QgsMultiBandColorRenderer *>( mRasterLayer->renderer() );
}

void QgsRasterTransparencyWidget::apply()
{
  //set NoDataValue
  QgsRasterRangeList myNoDataRangeList;
  if ( "" != leNoDataValue->text() )
  {
    bool myDoubleOk = false;
    const double myNoDataValue = QgsDoubleValidator::toDouble( leNoDataValue->text(), &myDoubleOk );
    if ( myDoubleOk )
    {
      const QgsRasterRange myNoDataRange( myNoDataValue, myNoDataValue );
      myNoDataRangeList << myNoDataRange;
    }
  }
  for ( int bandNo = 1; bandNo <= mRasterLayer->dataProvider()->bandCount(); bandNo++ )
  {
    mRasterLayer->dataProvider()->setUserNoDataValue( bandNo, myNoDataRangeList );
    mRasterLayer->dataProvider()->setUseSourceNoDataValue( bandNo, mSrcNoDataValueCheckBox->isChecked() );
  }

  //transparency settings
  QgsRasterRenderer *rasterRenderer = mRasterLayer->renderer();
  if ( rasterRenderer )
  {
    rasterRenderer->setAlphaBand( cboxTransparencyBand->currentBand() );
    rasterRenderer->setNodataColor( mNodataColorButton->color() );

    //Walk through each row in table and test value. If not valid set to 0.0 and continue building transparency list
    QgsRasterTransparency *rasterTransparency = new QgsRasterTransparency();
    if ( tableTransparency->columnCount() == 4 )
    {
      QgsRasterTransparency::TransparentThreeValuePixel myTransparentPixel;
      QList<QgsRasterTransparency::TransparentThreeValuePixel> myTransparentThreeValuePixelList;
      myTransparentThreeValuePixelList.reserve( tableTransparency->rowCount() );
      for ( int myListRunner = 0; myListRunner < tableTransparency->rowCount(); myListRunner++ )
      {
        myTransparentPixel.red = transparencyCellValue( myListRunner, 0 );
        myTransparentPixel.green = transparencyCellValue( myListRunner, 1 );
        myTransparentPixel.blue = transparencyCellValue( myListRunner, 2 );
        myTransparentPixel.percentTransparent = transparencyCellValue( myListRunner, 3 );
        myTransparentThreeValuePixelList.append( myTransparentPixel );
      }
      rasterTransparency->setTransparentThreeValuePixelList( myTransparentThreeValuePixelList );
    }
    else if ( tableTransparency->columnCount() == 3 )
    {
      QgsRasterTransparency::TransparentSingleValuePixel myTransparentPixel;
      QList<QgsRasterTransparency::TransparentSingleValuePixel> myTransparentSingleValuePixelList;
      myTransparentSingleValuePixelList.reserve( tableTransparency->rowCount() );
      for ( int myListRunner = 0; myListRunner < tableTransparency->rowCount(); myListRunner++ )
      {
        myTransparentPixel.min = transparencyCellValue( myListRunner, 0 );
        myTransparentPixel.max = transparencyCellValue( myListRunner, 1 );
        myTransparentPixel.percentTransparent = transparencyCellValue( myListRunner, 2 );

        myTransparentSingleValuePixelList.append( myTransparentPixel );
      }
      rasterTransparency->setTransparentSingleValuePixelList( myTransparentSingleValuePixelList );
    }

    rasterRenderer->setRasterTransparency( rasterTransparency );

    //set global transparency
    rasterRenderer->setOpacity( mOpacityWidget->opacity() );
  }

  mRasterLayer->pipe()->setDataDefinedProperties( mPropertyCollection );
}

void QgsRasterTransparencyWidget::initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsRasterPipe::Property key )
{
  button->blockSignals( true );
  button->init( key, mPropertyCollection, QgsRasterPipe::propertyDefinitions(), nullptr );
  connect( button, &QgsPropertyOverrideButton::changed, this, &QgsRasterTransparencyWidget::updateProperty );
  button->registerExpressionContextGenerator( this );
  button->blockSignals( false );
}

void QgsRasterTransparencyWidget::updateDataDefinedButtons()
{
  const auto propertyOverrideButtons { findChildren< QgsPropertyOverrideButton * >() };
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

  const QgsRasterPipe::Property key = static_cast< QgsRasterPipe::Property >( button->propertyKey() );
  whileBlocking( button )->setToProperty( mPropertyCollection.property( key ) );
}

void QgsRasterTransparencyWidget::updateProperty()
{
  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  const QgsRasterPipe::Property key = static_cast<  QgsRasterPipe::Property >( button->propertyKey() );
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
  if ( mMapCanvas && mPixelSelectorTool )
  {
    mMapCanvas->unsetMapTool( mPixelSelectorTool );

    const QgsMapSettings &ms = mMapCanvas->mapSettings();
    const QgsPointXY myPoint = ms.mapToLayerCoordinates( mRasterLayer, canvasPoint );

    const QgsRectangle myExtent = ms.mapToLayerCoordinates( mRasterLayer, mMapCanvas->extent() );
    const double mapUnitsPerPixel = mMapCanvas->mapUnitsPerPixel();
    const int myWidth = mMapCanvas->extent().width() / mapUnitsPerPixel;
    const int myHeight = mMapCanvas->extent().height() / mapUnitsPerPixel;

    const QMap<int, QVariant> myPixelMap = mRasterLayer->dataProvider()->identify( myPoint, QgsRaster::IdentifyFormatValue, myExtent, myWidth, myHeight ).results();

    const QList<int> bands = renderer->usesBands();

    QList<double> values;
    for ( int i = 0; i < bands.size(); ++i )
    {
      const int bandNo = bands.value( i );
      if ( myPixelMap.count( bandNo ) == 1 )
      {
        if ( myPixelMap.value( bandNo ).isNull() )
        {
          return; // Don't add nodata, transparent anyway
        }
        const double value = myPixelMap.value( bandNo ).toDouble();
        QgsDebugMsg( QStringLiteral( "value = %1" ).arg( value, 0, 'g', 17 ) );
        values.append( value );
      }
    }
    if ( bands.size() == 1 )
    {
      // Set 'to'
      values.insert( 1, values.value( 0 ) );
    }
    tableTransparency->insertRow( tableTransparency->rowCount() );
    for ( int i = 0; i < values.size(); i++ )
    {
      setTransparencyCell( tableTransparency->rowCount() - 1, i, values.value( i ) );
    }
    setTransparencyCell( tableTransparency->rowCount() - 1, tableTransparency->columnCount() - 1, 100 );
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

  if ( nBands == 1 )
  {
    QList<QgsRasterTransparency::TransparentSingleValuePixel> pixelList = rasterTransparency->transparentSingleValuePixelList();
    for ( int i = 0; i < pixelList.size(); ++i )
    {
      tableTransparency->insertRow( i );
      setTransparencyCell( i, 0, pixelList[i].min );
      setTransparencyCell( i, 1, pixelList[i].max );
      setTransparencyCell( i, 2, pixelList[i].percentTransparent );
      // break synchronization only if values differ
      if ( pixelList[i].min != pixelList[i].max )
      {
        setTransparencyToEdited( i );
      }
    }
  }
  else if ( nBands == 3 )
  {
    QList<QgsRasterTransparency::TransparentThreeValuePixel> pixelList = rasterTransparency->transparentThreeValuePixelList();
    for ( int i = 0; i < pixelList.size(); ++i )
    {
      tableTransparency->insertRow( i );
      setTransparencyCell( i, 0, pixelList[i].red );
      setTransparencyCell( i, 1, pixelList[i].green );
      setTransparencyCell( i, 2, pixelList[i].blue );
      setTransparencyCell( i, 3, pixelList[i].percentTransparent );
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
    tableTransparency->setColumnCount( 4 );
    tableTransparency->setHorizontalHeaderItem( 0, new QTableWidgetItem( tr( "Red" ) ) );
    tableTransparency->setHorizontalHeaderItem( 1, new QTableWidgetItem( tr( "Green" ) ) );
    tableTransparency->setHorizontalHeaderItem( 2, new QTableWidgetItem( tr( "Blue" ) ) );
    tableTransparency->setHorizontalHeaderItem( 3, new QTableWidgetItem( tr( "Percent Transparent" ) ) );
  }
  else //1 band
  {
    tableTransparency->setColumnCount( 3 );
// Is it important to distinguish the header? It becomes difficult with range.
#if 0
    if ( QgsRasterLayer::PalettedColor != mRasterLayer->drawingStyle() &&
         QgsRasterLayer::PalettedSingleBandGray != mRasterLayer->drawingStyle() &&
         QgsRasterLayer::PalettedSingleBandPseudoColor != mRasterLayer->drawingStyle() &&
         QgsRasterLayer::PalettedMultiBandColor != mRasterLayer->drawingStyle() )
    {
      tableTransparency->setHorizontalHeaderItem( 0, new QTableWidgetItem( tr( "Gray" ) ) );
    }
    else
    {
      tableTransparency->setHorizontalHeaderItem( 0, new QTableWidgetItem( tr( "Indexed Value" ) ) );
    }
#endif
    tableTransparency->setHorizontalHeaderItem( 0, new QTableWidgetItem( tr( "From" ) ) );
    tableTransparency->setHorizontalHeaderItem( 1, new QTableWidgetItem( tr( "To" ) ) );
    tableTransparency->setHorizontalHeaderItem( 2, new QTableWidgetItem( tr( "Percent Transparent" ) ) );
  }
}

void QgsRasterTransparencyWidget::setTransparencyCell( int row, int column, double value )
{
  QgsDebugMsg( QStringLiteral( "value = %1" ).arg( value, 0, 'g', 17 ) );
  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
  if ( !provider ) return;

  QgsRasterRenderer *renderer = mRasterLayer->renderer();
  if ( !renderer ) return;
  const int nBands = renderer->usesBands().size();

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

  if ( nBands == 1 && ( column == 0 || column == 1 ) )
  {
    connect( lineEdit, &QLineEdit::textEdited, this, &QgsRasterTransparencyWidget::transparencyCellTextEdited );
  }
  //tableTransparency->resizeColumnsToContents();
  emit widgetChanged();
}

void QgsRasterTransparencyWidget::adjustTransparencyCellWidth( int row, int column )
{
  QLineEdit *lineEdit = dynamic_cast<QLineEdit *>( tableTransparency->cellWidget( row, column ) );
  if ( !lineEdit ) return;

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
