/***************************************************************************
                          qgsvectorlayersaveasdialog.h
 Dialog to select destination, type and crs for ogr layers
                             -------------------
    begin                : Mon Mar 22 2010
    copyright            : (C) 2010 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslogger.h"
#include "qgsvectorlayersaveasdialog.h"
#include "qgsgenericprojectionselector.h"
#include "qgsvectordataprovider.h"
#include "qgscoordinatereferencesystem.h"

#include <QSettings>
#include <QFileDialog>
#include <QTextCodec>

QgsVectorLayerSaveAsDialog::QgsVectorLayerSaveAsDialog( long srsid, QWidget* parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
    , mCRS( srsid )
{
  setup();
}

QgsVectorLayerSaveAsDialog::QgsVectorLayerSaveAsDialog( long srsid, const QgsRectangle& layerExtent, bool layerHasSelectedFeatures, int options, QWidget* parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
    , mCRS( srsid )
    , mLayerExtent( layerExtent )
{
  setup();
  if ( !( options & Symbology ) )
  {
    mSymbologyExportLabel->hide();
    mSymbologyExportComboBox->hide();
    mScaleLabel->hide();
    mScaleSpinBox->hide();
  }

  mSelectedOnly->setEnabled( layerHasSelectedFeatures );
}

void QgsVectorLayerSaveAsDialog::setup()
{
  setupUi( this );
  QSettings settings;
  restoreGeometry( settings.value( "/Windows/VectorLayerSaveAs/geometry" ).toByteArray() );

  QMap<QString, QString> map = QgsVectorFileWriter::ogrDriverList();
  mFormatComboBox->blockSignals( true );
  for ( QMap< QString, QString>::const_iterator it = map.constBegin(); it != map.constEnd(); ++it )
  {
    mFormatComboBox->addItem( it.key(), it.value() );
  }

  QString format = settings.value( "/UI/lastVectorFormat", "ESRI Shapefile" ).toString();
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( format ) );
  mFormatComboBox->blockSignals( false );

  mEncodingComboBox->addItems( QgsVectorDataProvider::availableEncodings() );

  QString enc = settings.value( "/UI/encoding", "System" ).toString();
  int idx = mEncodingComboBox->findText( enc );
  if ( idx < 0 )
  {
    mEncodingComboBox->insertItem( 0, enc );
    idx = 0;
  }

  mCRSSelection->clear();
  mCRSSelection->addItems( QStringList() << tr( "Layer CRS" ) << tr( "Project CRS" ) << tr( "Selected CRS" ) );

  QgsCoordinateReferenceSystem srs( mCRS, QgsCoordinateReferenceSystem::InternalCrsId );
  leCRS->setText( srs.description() );

  mEncodingComboBox->setCurrentIndex( idx );
  on_mFormatComboBox_currentIndexChanged( mFormatComboBox->currentIndex() );

  //symbology export combo box
  mSymbologyExportComboBox->addItem( tr( "No symbology" ), QgsVectorFileWriter::NoSymbology );
  mSymbologyExportComboBox->addItem( tr( "Feature symbology" ), QgsVectorFileWriter::FeatureSymbology );
  mSymbologyExportComboBox->addItem( tr( "Symbol layer symbology" ), QgsVectorFileWriter::SymbolLayerSymbology );
  on_mSymbologyExportComboBox_currentIndexChanged( mSymbologyExportComboBox->currentText() );

  // extent group box
  mExtentGroupBox->setOutputCrs( srs );
  mExtentGroupBox->setOriginalExtent( mLayerExtent, srs );
  mExtentGroupBox->setOutputExtentFromOriginal();
  mExtentGroupBox->setCheckable( true );
  mExtentGroupBox->setChecked( false );
  mExtentGroupBox->setCollapsed( true );
}

QList<QPair<QLabel*, QWidget*> > QgsVectorLayerSaveAsDialog::createControls( const QMap<QString, QgsVectorFileWriter::Option*>& options )
{
  QList<QPair<QLabel*, QWidget*> > controls;
  QMap<QString, QgsVectorFileWriter::Option*>::ConstIterator it;

  for ( it = options.constBegin(); it != options.constEnd(); ++it )
  {
    QgsVectorFileWriter::Option *option = it.value();
    QLabel *label = new QLabel( it.key() );
    QWidget *control = 0;
    switch ( option->type )
    {
      case QgsVectorFileWriter::Int:
      {
        QgsVectorFileWriter::IntOption* opt = dynamic_cast<QgsVectorFileWriter::IntOption*>( option );
        QSpinBox* sb = new QSpinBox();
        sb->setObjectName( it.key() );
        sb->setValue( opt->defaultValue );
        control = sb;
        break;
      }

      case QgsVectorFileWriter::Set:
      {
        QgsVectorFileWriter::SetOption* opt = dynamic_cast<QgsVectorFileWriter::SetOption*>( option );
        QComboBox* cb = new QComboBox();
        cb->setObjectName( it.key() );
        Q_FOREACH( const QString& val, opt->values )
        {
          cb->addItem( val, val );
        }
        if ( opt->allowNone )
          cb->addItem( tr( "<Default>" ), QVariant( QVariant::String ) );
        int idx = cb->findText( opt->defaultValue );
        if ( idx == -1 )
          idx = cb->findData( QVariant( QVariant::String ) );
        cb->setCurrentIndex( idx );
        control = cb;
        break;
      }

      case QgsVectorFileWriter::String:
      {
        QgsVectorFileWriter::StringOption* opt = dynamic_cast<QgsVectorFileWriter::StringOption*>( option );
        QLineEdit* le = new QLineEdit( opt->defaultValue );
        le->setObjectName( it.key() );
        control = le;
        break;
      }

      case QgsVectorFileWriter::Hidden:
        control = 0;
        break;
    }

    if ( control )
    {
      // Pack the tooltip in some html element, so it gets linebreaks.
      label->setToolTip( QString( "<p>%1</p>" ).arg( option->docString ) );
      control->setToolTip( QString( "<p>%1</p>" ).arg( option->docString ) );

      controls << QPair<QLabel*, QWidget*>( label, control );
    }
  }

  return controls;
}

QgsVectorLayerSaveAsDialog::~QgsVectorLayerSaveAsDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/VectorLayerSaveAs/geometry", saveGeometry() );
}

void QgsVectorLayerSaveAsDialog::accept()
{
  QSettings settings;
  settings.setValue( "/UI/lastVectorFileFilterDir", QFileInfo( filename() ).absolutePath() );
  settings.setValue( "/UI/lastVectorFormat", format() );
  settings.setValue( "/UI/encoding", encoding() );
  QDialog::accept();
}

void QgsVectorLayerSaveAsDialog::on_mCRSSelection_currentIndexChanged( int idx )
{
  leCRS->setEnabled( idx == 2 );

  QgsCoordinateReferenceSystem crs;
  if ( mCRSSelection->currentIndex() == 0 )
  {
    crs = mLayerCrs;
  }
  else if ( mCRSSelection->currentIndex() == 1 )
  {
    crs = mExtentGroupBox->currentCrs();
  }
  else // custom CRS
  {
    crs.createFromId( mCRS, QgsCoordinateReferenceSystem::InternalCrsId );
  }
  mExtentGroupBox->setOutputCrs( crs );
}

void QgsVectorLayerSaveAsDialog::on_mFormatComboBox_currentIndexChanged( int idx )
{
  Q_UNUSED( idx );

  browseFilename->setEnabled( true );
  leFilename->setEnabled( true );

  if ( format() == "KML" )
  {
    mEncodingComboBox->setCurrentIndex( mEncodingComboBox->findText( "UTF-8" ) );
    mEncodingComboBox->setDisabled( true );
    mSkipAttributeCreation->setEnabled( true );
  }
  else if ( format() == "DXF" )
  {
    mSkipAttributeCreation->setChecked( true );
    mSkipAttributeCreation->setDisabled( true );
  }
  else
  {
    mEncodingComboBox->setEnabled( true );
    mSkipAttributeCreation->setEnabled( true );
  }

  QgsVectorFileWriter::MetaData driverMetaData;

  while ( mDatasourceOptionsGroupBox->layout()->count() )
  {
    QLayoutItem* item = mDatasourceOptionsGroupBox->layout()->takeAt( 0 );
    delete item->widget();
    delete item;
  }

  while ( mLayerOptionsGroupBox->layout()->count() )
  {
    QLayoutItem* item = mLayerOptionsGroupBox->layout()->takeAt( 0 );
    delete item->widget();
    delete item;
  }

  // workaround so the Q_FOREACH macro does not get confused by the ','
  typedef QPair<QLabel*, QWidget*> LabelControlPair;

  if ( QgsVectorFileWriter::driverMetadata( format(), driverMetaData ) )
  {
    if ( driverMetaData.driverOptions.size() != 0 )
    {
      mDatasourceOptionsGroupBox->setVisible( true );
      QList<QPair<QLabel*, QWidget*> > controls = createControls( driverMetaData.driverOptions );

      QFormLayout* datasourceLayout = dynamic_cast<QFormLayout*>( mDatasourceOptionsGroupBox->layout() );

      Q_FOREACH( const LabelControlPair& control, controls )
      {
        datasourceLayout->addRow( control.first, control.second );
      }
    }
    else
    {
      mDatasourceOptionsGroupBox->setVisible( false );
    }

    if ( driverMetaData.layerOptions.size() != 0 )
    {
      mLayerOptionsGroupBox->setVisible( true );
      QList<QPair<QLabel*, QWidget*> > controls = createControls( driverMetaData.layerOptions );

      QFormLayout* layerOptionsLayout = dynamic_cast<QFormLayout*>( mLayerOptionsGroupBox->layout() );

      Q_FOREACH( const LabelControlPair& control, controls )
      {
        layerOptionsLayout->addRow( control.first, control.second );
      }
    }
    else
    {
      mLayerOptionsGroupBox->setVisible( false );
    }
  }
}

void QgsVectorLayerSaveAsDialog::on_browseFilename_clicked()
{
  QSettings settings;
  QString dirName = leFilename->text().isEmpty() ? settings.value( "/UI/lastVectorFileFilterDir", "." ).toString() : leFilename->text();
  QString filterString = QgsVectorFileWriter::filterForDriver( format() );
  QString outputFile = QFileDialog::getSaveFileName( 0, tr( "Save layer as..." ), dirName, filterString );
  if ( !outputFile.isNull() )
  {
    leFilename->setText( outputFile );
  }
}

void QgsVectorLayerSaveAsDialog::on_browseCRS_clicked()
{
  QgsGenericProjectionSelector * mySelector = new QgsGenericProjectionSelector();
  if ( mCRS >= 0 )
    mySelector->setSelectedCrsId( mCRS );
  mySelector->setMessage( tr( "Select the coordinate reference system for the vector file. "
                              "The data points will be transformed from the layer coordinate reference system." ) );

  if ( mySelector->exec() )
  {
    QgsCoordinateReferenceSystem srs( mySelector->selectedCrsId(), QgsCoordinateReferenceSystem::InternalCrsId );
    mCRS = srs.srsid();
    leCRS->setText( srs.description() );
    mCRSSelection->setCurrentIndex( 2 );

    mExtentGroupBox->setOutputCrs( srs );
  }

  delete mySelector;
}

QString QgsVectorLayerSaveAsDialog::filename() const
{
  return leFilename->text();
}

QString QgsVectorLayerSaveAsDialog::encoding() const
{
  return mEncodingComboBox->currentText();
}

QString QgsVectorLayerSaveAsDialog::format() const
{
  return mFormatComboBox->itemData( mFormatComboBox->currentIndex() ).toString();
}

long QgsVectorLayerSaveAsDialog::crs() const
{
  if ( mCRSSelection->currentIndex() == 0 )
  {
    return -1; // Layer CRS
  }
  else if ( mCRSSelection->currentIndex() == 1 )
  {
    return -2; // Project CRS
  }
  else
  {
    return mCRS;
  }
}

QStringList QgsVectorLayerSaveAsDialog::datasourceOptions() const
{
  QStringList options;

  QgsVectorFileWriter::MetaData driverMetaData;

  if ( QgsVectorFileWriter::driverMetadata( format(), driverMetaData ) )
  {
    QMap<QString, QgsVectorFileWriter::Option*>::ConstIterator it;

    for ( it = driverMetaData.driverOptions.constBegin(); it != driverMetaData.driverOptions.constEnd(); ++it )
    {
      switch ( it.value()->type )
      {
        case QgsVectorFileWriter::Int:
        {
          QSpinBox* sb = mDatasourceOptionsGroupBox->findChild<QSpinBox*>( it.key() );
          if ( sb )
            options << QString( "%1=%2" ).arg( it.key() ).arg( sb->value() );
          break;
        }

        case QgsVectorFileWriter::Set:
        {
          QComboBox* cb = mDatasourceOptionsGroupBox->findChild<QComboBox*>( it.key() );
          if ( cb && !cb->itemData( cb->currentIndex() ).isNull() )
            options << QString( "%1=%2" ).arg( it.key() ).arg( cb->currentText() );
          break;
        }

        case QgsVectorFileWriter::String:
        {
          QLineEdit* le = mDatasourceOptionsGroupBox->findChild<QLineEdit*>( it.key() );
          if ( le )
            options << QString( "%1=%2" ).arg( it.key() ).arg( le->text() );
          break;
        }

        case QgsVectorFileWriter::Hidden:
        {
          QgsVectorFileWriter::HiddenOption *opt =
            dynamic_cast<QgsVectorFileWriter::HiddenOption*>( it.value() );
          options << QString( "%1=%2" ).arg( it.key() ).arg( opt->mValue );
          break;
        }
      }
    }
  }

  return options + mOgrDatasourceOptions->toPlainText().split( "\n" );
}

QStringList QgsVectorLayerSaveAsDialog::layerOptions() const
{
  QStringList options;

  QgsVectorFileWriter::MetaData driverMetaData;

  if ( QgsVectorFileWriter::driverMetadata( format(), driverMetaData ) )
  {
    QMap<QString, QgsVectorFileWriter::Option*>::ConstIterator it;

    for ( it = driverMetaData.layerOptions.constBegin(); it != driverMetaData.layerOptions.constEnd(); ++it )
    {
      switch ( it.value()->type )
      {
        case QgsVectorFileWriter::Int:
        {
          QSpinBox* sb = mLayerOptionsGroupBox->findChild<QSpinBox*>( it.key() );
          options << QString( "%1=%2" ).arg( it.key() ).arg( sb->value() );
          break;
        }

        case QgsVectorFileWriter::Set:
        {
          QComboBox* cb = mLayerOptionsGroupBox->findChild<QComboBox*>( it.key() );
          options << QString( "%1=%2" ).arg( it.key() ).arg( cb->currentText() );
          break;
        }

        case QgsVectorFileWriter::String:
        {
          QLineEdit* le = mLayerOptionsGroupBox->findChild<QLineEdit*>( it.key() );
          options << QString( "%1=%2" ).arg( it.key() ).arg( le->text() );
          break;
        }

        case QgsVectorFileWriter::Hidden:
        {
          QgsVectorFileWriter::HiddenOption *opt =
            dynamic_cast<QgsVectorFileWriter::HiddenOption*>( it.value() );
          options << QString( "%1=%2" ).arg( it.key() ).arg( opt->mValue );
          break;
        }
      }
    }
  }

  return options + mOgrLayerOptions->toPlainText().split( "\n" );
}

bool QgsVectorLayerSaveAsDialog::skipAttributeCreation() const
{
  return mSkipAttributeCreation->isChecked();
}

bool QgsVectorLayerSaveAsDialog::addToCanvas() const
{
  return mAddToCanvas->isChecked();
}

int QgsVectorLayerSaveAsDialog::symbologyExport() const
{
  return mSymbologyExportComboBox->itemData( mSymbologyExportComboBox->currentIndex() ).toInt();
}

double QgsVectorLayerSaveAsDialog::scaleDenominator() const
{
  return mScaleSpinBox->value();
}

void QgsVectorLayerSaveAsDialog::setCanvasExtent( const QgsRectangle& canvasExtent, const QgsCoordinateReferenceSystem& canvasCrs )
{
  mExtentGroupBox->setCurrentExtent( canvasExtent, canvasCrs );
}

bool QgsVectorLayerSaveAsDialog::hasFilterExtent() const
{
  return mExtentGroupBox->isChecked();
}

QgsRectangle QgsVectorLayerSaveAsDialog::filterExtent() const
{
  return mExtentGroupBox->outputExtent();
}

bool QgsVectorLayerSaveAsDialog::onlySelected() const
{
  return mSelectedOnly->isChecked();
}

void QgsVectorLayerSaveAsDialog::on_mSymbologyExportComboBox_currentIndexChanged( const QString& text )
{
  bool scaleEnabled = true;
  if ( text == tr( "No symbology" ) )
  {
    scaleEnabled = false;
  }
  mScaleSpinBox->setEnabled( scaleEnabled );
  mScaleLabel->setEnabled( scaleEnabled );
}
