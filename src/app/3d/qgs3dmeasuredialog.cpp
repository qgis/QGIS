#include <QtMath>

#include "qgs3dmeasuredialog.h"
#include "qgs3dmaptoolmeasureline.h"
#include "qdebug.h"

Qgs3DMeasureDialog::Qgs3DMeasureDialog( Qgs3DMapToolMeasureLine *tool, Qt::WindowFlags f )
  : QDialog( tool->canvas()->topLevelWidget(), f )
  , mTool( tool )
{
  setupUi( this );
  qInfo() << "3D Measure Dialog created";
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
