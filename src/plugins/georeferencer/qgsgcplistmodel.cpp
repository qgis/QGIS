/***************************************************************************
    qgsgcplistmodel.cpp - Model implementation of GCPList Model/View
     --------------------------------------
    Date                 : 27-Feb-2009
    Copyright            : (c) 2009 by Manuel Massing
    Email                : m.massing at warped-space.de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsgcplist.h"
#include "qgsgcplistmodel.h"
#include "qgis.h"
#include "qgsgeorefdatapoint.h"
#include "qgsgeoreftransform.h"

#include <cmath>
using namespace std;

class QgsStandardItem : public QStandardItem
{
  public:
    QgsStandardItem( QString text ) : QStandardItem( text ) { init(); }
    QgsStandardItem( int value ) : QStandardItem( QString::number( value ) ) { init(); }
    QgsStandardItem( double value ) : QStandardItem( QString::number( value, 'f', 2 ) ) { init(); }

  private:
    void init()
    {
      setTextAlignment( Qt::AlignCenter );
    }
};

#define QGSSTANDARDITEM(value) (new QgsStandardItem(value))

#if 0
template <class T> class QNumericItem : public QStandardItem
{
  public:
    QNumericItem( T value ) : QStandardItem( QString( "%1" ).arg( value ) ), mValue( value )
    {
    }

    bool operator < ( const QStandardItem &other ) const
    {
      const QNumericItem<T> *otherD = dynamic_cast<const QNumericItem<T> *>( &other );
      if ( otherD == NULL )
        return false;
      return mValue < otherD->mValue;
    }
  private:
    T mValue;
};
#endif

QgsGCPListModel::QgsGCPListModel( QObject *parent )
    : QStandardItemModel( parent )
    , mGCPList( 0 )
    , mGeorefTransform( 0 )
{
}

void QgsGCPListModel::setGCPList( QgsGCPList *theGCPList )
{
  mGCPList = theGCPList;
  updateModel();
}

// ------------------------------- public ---------------------------------- //
void QgsGCPListModel::setGeorefTransform( QgsGeorefTransform *theGeorefTransform )
{
  mGeorefTransform = theGeorefTransform;
  updateModel();
}

void QgsGCPListModel::updateModel()
{
  clear();
  if ( !mGCPList )
    return;

  bool bTransformUpdated = false;
  bool wldTransform = false;
  double wldScaleX, wldScaleY, rotation;
  QgsPoint origin;

  if ( mGeorefTransform )
  {
    vector<QgsPoint> mapCoords, pixelCoords;
    mGCPList->createGCPVectors( mapCoords, pixelCoords );

    // TODO: the parameters should probable be updated externally (by user interaction)
    bTransformUpdated = mGeorefTransform->updateParametersFromGCPs( mapCoords, pixelCoords );
    //transformation that involves only scaling and rotation (linear or helmert) ?
    wldTransform = mGeorefTransform->getOriginScaleRotation( origin, wldScaleX, wldScaleY, rotation );
    if ( wldTransform && !doubleNear( rotation, 0.0 ) )
    {
      wldScaleX *= cos( rotation );
      wldScaleY *= cos( rotation );
    }
    if ( wldTransform )
    {

    }
  }

  //  // Setup table header
  QStringList itemLabels;
  QString unitType = wldTransform ? tr( "map units" ) : tr ("pixels");
  itemLabels << "on/off" << "id" << "srcX" << "srcY" << "dstX" << "dstY" << QString( "dX[" ) + unitType + "]" << QString( "dY[" ) + unitType + "]" << "residual[" + unitType + "]";

  setHorizontalHeaderLabels( itemLabels );
  setRowCount( mGCPList->size() );

  for ( int i = 0; i < mGCPList->sizeAll(); ++i )
  {
    int j = 0;
    QgsGeorefDataPoint *p = mGCPList->at( i );
    p->setId( i );

    QStandardItem *si = new QStandardItem();
    si->setTextAlignment( Qt::AlignCenter );
    si->setCheckable( true );
    if ( p->isEnabled() )
      si->setCheckState( Qt::Checked );
    else
      si->setCheckState( Qt::Unchecked );

    setItem( i, j++, si );
    setItem( i, j++, QGSSTANDARDITEM( i ) /*create_item<int>(i)*/ );
    setItem( i, j++, QGSSTANDARDITEM( p->pixelCoords().x() ) /*create_item<double>( p->pixelCoords().x() )*/ );
    setItem( i, j++, QGSSTANDARDITEM( -p->pixelCoords().y() ) /*create_item<double>(-p->pixelCoords().y() )*/ );
    setItem( i, j++, QGSSTANDARDITEM( p->mapCoords().x() ) /*create_item<double>( p->mapCoords().x() )*/ );
    setItem( i, j++, QGSSTANDARDITEM( p->mapCoords().y() ) /*create_item<double>( p->mapCoords().y() )*/ );

    double residual;
    double dX, dY;
    // Calculate residual if transform is available and up-to-date
    if ( mGeorefTransform && bTransformUpdated && mGeorefTransform->parametersInitialized() )
    {
      QgsPoint dst;
      // Transform from world to raster coordinate:
      // This is the transform direction used by the warp operation.
      // As transforms of order >=2 are not invertible, we are only
      // interested in the residual in this direction
      mGeorefTransform->transformWorldToRaster( p->mapCoords(), dst );
      dX = ( dst.x() - p->pixelCoords().x() );
      dY = -( dst.y() - p->pixelCoords().y() );
      if ( wldTransform )
      {
        dX *= wldScaleX;
        dY *= wldScaleY;
      }
      residual = sqrt( dX * dX + dY * dY );
    }
    else
    {
      dX = dY = residual = 0;
    }

    if ( p )
    {
      p->setResidual( QPointF( dX, dY ) );
    }

    if ( residual >= 0.f )
    {
      setItem( i, j++, QGSSTANDARDITEM( dX ) /*create_item<double>(dX)*/ );
      setItem( i, j++, QGSSTANDARDITEM( dY ) /*create_item<double>(-dY)*/ );
      setItem( i, j++, QGSSTANDARDITEM( residual ) /*create_item<double>(residual)*/ );
    }
    else
    {
      setItem( i, j++, QGSSTANDARDITEM( "n/a" ) /*create_std_item("n/a")*/ );
      setItem( i, j++, QGSSTANDARDITEM( "n/a" ) /*create_std_item("n/a")*/ );
      setItem( i, j++, QGSSTANDARDITEM( "n/a" ) /*create_std_item("n/a")*/ );
    }
  }
  //sort();  // Sort data
  //reset(); // Signal to views that the model has changed
}

// --------------------------- public slots -------------------------------- //
void QgsGCPListModel::replaceDataPoint( QgsGeorefDataPoint *newDataPoint, int i )
{
  mGCPList->replace( i, newDataPoint );
}

void QgsGCPListModel::onGCPListModified()
{
}

void QgsGCPListModel::onTransformationModified()
{
}

#if 0
template <class T> QNumericItem<T> *create_item( const T value, bool isEditable = true )
{
  QNumericItem<T> *item = new QNumericItem<T>( value );
  item->setEditable( isEditable );
  return item;
}

QStandardItem *create_std_item( const QString &S, bool isEditable = false )
{
  QStandardItem *std_item = new QStandardItem( S );
  std_item->setEditable( isEditable );
  return std_item;
}
#endif
