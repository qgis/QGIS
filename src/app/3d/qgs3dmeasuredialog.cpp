#include <QtMath>
#include <QCloseEvent>
#include <QPushButton>

#include "qgsproject.h"
#include "qgs3dmeasuredialog.h"
#include "qgs3dmaptoolmeasureline.h"
#include "qdebug.h"

Qgs3DMeasureDialog::Qgs3DMeasureDialog( Qgs3DMapToolMeasureLine *tool, Qt::WindowFlags f )
  : QDialog( tool->canvas()->topLevelWidget(), f )
  , mTool( tool )
{
  setupUi( this );

  // New button
  QPushButton *newButton = new QPushButton( tr( "&New" ) );
  buttonBox->addButton( newButton, QDialogButtonBox::ActionRole );
  connect( newButton, &QAbstractButton::clicked, this, &Qgs3DMeasureDialog::restart );

  // Initialize unit combo box
  repopulateComboBoxUnits();

  // Choose unit
  if ( mUseMapUnits )
    mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( QgsUnitTypes::DistanceUnknownUnit ) );
  else
    mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( QgsProject::instance()->distanceUnits() ) );


  qInfo() << "3D Measure Dialog created";
  connect( buttonBox, &QDialogButtonBox::rejected, this, &Qgs3DMeasureDialog::reject );
  connect( mUnitsCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &Qgs3DMeasureDialog::unitsChanged );
}

void Qgs3DMeasureDialog::saveWindowLocation()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/3DMeasure/geometry" ), saveGeometry() );
  const QString &key = "/Windows/3DMeasure/h";
  settings.setValue( key, height() );
}

void Qgs3DMeasureDialog::restorePosition()
{
  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/3DMeasure/geometry" ) ).toByteArray() );
  int wh = settings.value( QStringLiteral( "Windows/3DMeasure/h" ), 200 ).toInt();
  resize( width(), wh );
//    updateUi();
}

void Qgs3DMeasureDialog::addPoint()
{
  int numPoints = mTool->points().size();
  qInfo() << "Add point. Current num points: " << numPoints;
  if ( numPoints > 1 )
  {
    if ( !mTool->done() )
    {
      QTreeWidgetItem *item = new QTreeWidgetItem( QStringList( QLocale().toString( 0.0, 'f', mDecimalPlaces ) ) );
      item->setTextAlignment( 0, Qt::AlignRight );
      mTable->addTopLevelItem( item );
      mTable->scrollToItem( item );


      item->setText( 0, QString::number( lastDistance() ) );
      mTotal += lastDistance();
      editTotal->setText( QString::number( mTotal ) );
    }
  }
  else
  {
    editTotal->setText( QString::number( mTotal ) );
  }
}

double Qgs3DMeasureDialog::lastDistance()
{
  QgsPoint lastPoint = mTool->points().rbegin()[0];
  QgsPoint secondLastPoint = mTool->points().rbegin()[1];
  // Euclidean distance
  qInfo() << "Last line " << lastPoint.x() << lastPoint.y() << lastPoint.z();
  qInfo() << "Last line " << secondLastPoint.x() << secondLastPoint.y() << secondLastPoint.z();
  return qSqrt(
           ( lastPoint.x() - secondLastPoint.x() ) * ( lastPoint.x() - secondLastPoint.x() ) +
           ( lastPoint.y() - secondLastPoint.y() ) * ( lastPoint.y() - secondLastPoint.y() ) +
           ( lastPoint.z() - secondLastPoint.z() ) * ( lastPoint.z() - secondLastPoint.z() )
         );
}

void Qgs3DMeasureDialog::updateUi()
{
  editTotal->setText( QString::number( mTotal ) );
}

void Qgs3DMeasureDialog::repopulateComboBoxUnits()
{
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceMeters ), QgsUnitTypes::DistanceMeters );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceKilometers ), QgsUnitTypes::DistanceKilometers );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceFeet ), QgsUnitTypes::DistanceFeet );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceYards ), QgsUnitTypes::DistanceYards );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceMiles ), QgsUnitTypes::DistanceMiles );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceNauticalMiles ), QgsUnitTypes::DistanceNauticalMiles );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceCentimeters ), QgsUnitTypes::DistanceCentimeters );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceMillimeters ), QgsUnitTypes::DistanceMillimeters );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceDegrees ), QgsUnitTypes::DistanceDegrees );
  mUnitsCombo->addItem( tr( "map units" ), QgsUnitTypes::DistanceUnknownUnit );
}

void Qgs3DMeasureDialog::reject()
{
  saveWindowLocation();
  restart();
  QDialog::close();
}

void Qgs3DMeasureDialog::restart()
{
  mTool->restart();

  mTable->clear();
  mTotal = 0.;
  updateUi();
}

void Qgs3DMeasureDialog::closeEvent( QCloseEvent *e )
{
  reject();
  e->accept();
}

void Qgs3DMeasureDialog::unitsChanged( int index )
{
  mDistanceUnits = static_cast< QgsUnitTypes::DistanceUnit >( mUnitsCombo->itemData( index ).toInt() );
  if ( mDistanceUnits == QgsUnitTypes::DistanceUnknownUnit )
  {
    mUseMapUnits = true;
    mDistanceUnits = mMapDistanceUnits;
  }
  else
  {
    mUseMapUnits = false;
  }
  mTable->clear();
  mTotal = 0.;
  updateUi();
}
