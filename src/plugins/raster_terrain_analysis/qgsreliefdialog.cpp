#include "qgsreliefdialog.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayer.h"
#include <QColorDialog>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QSettings>
#include "cpl_string.h"
#include "gdal.h"

QgsReliefDialog::QgsReliefDialog( DisplayMode mode, QWidget * parent, Qt::WindowFlags f ): QDialog( parent, f )
{
  setupUi( this );

  if ( mode == HillshadeInput )
  {
    mReliefColorsGroupBox->setVisible( false );
    mLightAzimuthAngleSpinBox->setValue( 300 );
    mLightVerticalAngleSpinBox->setValue( 40 );
  }
  else if ( mode == ReliefInput )
  {
    mIlluminationGroupBox->setVisible( false );
  }
  else //no parameters
  {
    mReliefColorsGroupBox->setVisible( false );
    mIlluminationGroupBox->setVisible( false );
  }

  mZFactorLineEdit->setText( "1.0" );
  mZFactorLineEdit->setValidator( new QDoubleValidator( this ) );

  //insert available raster layers
  //enter available layers into the combo box
  QMap<QString, QgsMapLayer*> mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  QMap<QString, QgsMapLayer*>::iterator layer_it = mapLayers.begin();

  //insert available input layers
  for ( ; layer_it != mapLayers.end(); ++layer_it )
  {
    QgsRasterLayer* rl = qobject_cast<QgsRasterLayer *>( layer_it.value() );
    if ( rl )
    {
      mElevationLayerComboBox->addItem( rl->name(), QVariant( rl->id() ) );
    }
  }

  //insert available drivers that support the create() operation
  GDALAllRegister();

  int nDrivers = GDALGetDriverCount();
  for ( int i = 0; i < nDrivers; ++i )
  {
    GDALDriverH driver = GDALGetDriver( i );
    if ( driver != NULL )
    {
      char** driverMetadata = GDALGetMetadata( driver, NULL );
      if ( CSLFetchBoolean( driverMetadata, GDAL_DCAP_CREATE, false ) )
      {
        mOutputFormatComboBox->addItem( GDALGetDriverLongName( driver ), QVariant( GDALGetDriverShortName( driver ) ) );

        //store the driver shortnames and the corresponding extensions
        //(just in case the user does not give an extension for the output file name)
        int index = 0;
        while (( driverMetadata ) && driverMetadata[index] != 0 )
        {
          QStringList metadataTokens = QString( driverMetadata[index] ).split( "=", QString::SkipEmptyParts );
          if ( metadataTokens.size() < 1 )
          {
            break;
          }

          if ( metadataTokens[0] == "DMD_EXTENSION" )
          {
            if ( metadataTokens.size() < 2 )
            {
              ++index;
              continue;
            }
            mDriverExtensionMap.insert( QString( GDALGetDriverShortName( driver ) ), metadataTokens[1] );
            break;
          }
          ++index;
        }

      }
    }
  }

  //and set last used driver in combo box
  QSettings s;
  QString lastUsedDriver = s.value( "/RasterTerrainAnalysis/lastOutputFormat", "GeoTIFF" ).toString();
  int lastDriverIndex = mOutputFormatComboBox->findText( lastUsedDriver );
  if ( lastDriverIndex != -1 )
  {
    mOutputFormatComboBox->setCurrentIndex( lastDriverIndex );
  }

  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
}

QgsReliefDialog::~QgsReliefDialog()
{
}

QList< QgsRelief::ReliefColor > QgsReliefDialog::reliefColors() const
{
  QList< QgsRelief::ReliefColor > reliefColorList;

  for ( int i = 0; i < mReliefClassTreeWidget->topLevelItemCount(); ++i )
  {
    QTreeWidgetItem* reliefItem = mReliefClassTreeWidget->topLevelItem( i );
    if ( reliefItem )
    {
      QgsRelief::ReliefColor rc( reliefItem->background( 2 ).color(), reliefItem->text( 0 ).toDouble(), reliefItem->text( 1 ).toDouble() );
      reliefColorList.push_back( rc );
    }
  }

  return reliefColorList;
}

QString QgsReliefDialog::inputFile() const
{
  QgsMapLayer* inputLayer = QgsMapLayerRegistry::instance()->mapLayer( mElevationLayerComboBox->itemData( mElevationLayerComboBox->currentIndex() ).toString() );
  if ( !inputLayer )
  {
    return "";
  }

  QString inputFilePath = inputLayer->source();
  return inputFilePath;
}

QString QgsReliefDialog::outputFile() const
{
  return mOutputLayerLineEdit->text();
}

QString QgsReliefDialog::outputFormat() const
{
  int index = mOutputFormatComboBox->currentIndex();
  if ( index == -1 )
  {
    return "";
  }
  return mOutputFormatComboBox->itemData( index ).toString();
}

bool QgsReliefDialog::addResultToProject() const
{
  return mAddResultToProjectCheckBox->isChecked();
}

double QgsReliefDialog::zFactor() const
{
  return mZFactorLineEdit->text().toDouble();
}

void QgsReliefDialog::on_mOutputLayerLineEdit_textChanged( const QString& text )
{
  bool enabled = false;

  QFileInfo fi( text );
  if ( fi.absoluteDir().exists() &&  mElevationLayerComboBox->count() > 0 )
  {
    enabled = true;
  }
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}

void QgsReliefDialog::on_mAutomaticColorButton_clicked()
{
  QgsRelief relief( inputFile(), outputFile(), outputFormat() );
  QList< QgsRelief::ReliefColor > reliefColorList = relief.calculateOptimizedReliefClasses();
  QList< QgsRelief::ReliefColor >::iterator it = reliefColorList.begin();

  mReliefClassTreeWidget->clear();
  for ( ; it != reliefColorList.end(); ++it )
  {
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText( 0, QString::number( it->minElevation ) );
    item->setText( 1, QString::number( it->maxElevation ) );
    item->setBackground( 2, QBrush( it->color ) );
    mReliefClassTreeWidget->addTopLevelItem( item );
  }
}

void QgsReliefDialog::on_mExportToCsvButton_clicked()
{
  QString file = QFileDialog::getSaveFileName( 0, tr("Export Frequency distribution as csv") );
  if( file.isEmpty() )
  {
    return;
  }

  QgsRelief relief( inputFile(), outputFile(), outputFormat() );
  relief.exportFrequencyDistributionToCsv( file );
}

void QgsReliefDialog::on_mOutputLayerToolButton_clicked()
{
  QSettings s;
  QString lastDir = s.value( "/RasterTerrainAnalysis/lastOutputDir" ).toString();
  QString saveFileName = QFileDialog::getSaveFileName( 0, tr( "Enter result file" ), lastDir );
  if ( !saveFileName.isNull() )
  {
    mOutputLayerLineEdit->setText( saveFileName );
  }
}

double QgsReliefDialog::lightAzimuth() const
{
  return mLightAzimuthAngleSpinBox->value();
}

double QgsReliefDialog::lightAngle() const
{
  return mLightVerticalAngleSpinBox->value();
}

void QgsReliefDialog::on_mRemoveClassButton_clicked()
{
  QList<QTreeWidgetItem*> selectedItems = mReliefClassTreeWidget->selectedItems();
  QList<QTreeWidgetItem*>::iterator itemIt = selectedItems.begin();
  for(; itemIt != selectedItems.end(); ++itemIt )
  {
    delete *itemIt;
  }
}

void QgsReliefDialog::on_mReliefClassTreeWidget_itemDoubleClicked( QTreeWidgetItem* item, int column )
{
  if( !item )
  {
    return;
  }

  if( column == 0 )
  {
    bool ok;
    double d = QInputDialog::getDouble(0, tr("Enter lower elevation class bound"), tr("Elevation"), item->text( 0 ).toDouble(), -2147483647,
                                        2147483647, 2, &ok );
    if( ok )
    {
      item->setText( 0, QString::number( d ) );
    }
  }
  else if( column == 1 )
  {
    bool ok;
    double d = QInputDialog::getDouble(0, tr("Enter upper elevation class bound"), tr("Elevation"), item->text( 1 ).toDouble(), -2147483647,
                                        2147483647, 2, &ok );
    if( ok )
    {
      item->setText( 1, QString::number( d ) );
    }
  }
  else if( column == 2 )
  {
    QColor c = QColorDialog::getColor( item->background( 2 ).color(), 0, tr("Select color for relief class") );
    if( c.isValid() )
    {
      item->setBackground( 2, QBrush( c ) );
    }
  }
}
