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
/* $Id */
#include "qgsgcplistmodel.h"

#include "qgsgeorefdatapoint.h"
#include "qgsgeoreftransform.h"

#include <cmath>
using namespace std;

template <class T> class QNumericItem : public QStandardItem {
public:
  QNumericItem(T value) : QStandardItem(QString("%1").arg(value)), mValue(value)
  {
  }

  bool operator < (const QStandardItem &other) const
  {
    const QNumericItem<T> *otherD = dynamic_cast<const QNumericItem<T> *>(&other);
    if (otherD == NULL)
      return false;
    return mValue < otherD->mValue;
  }
private:
  T mValue;
};

QgsGCPListModel::QgsGCPListModel(QObject *parent) : QStandardItemModel(parent), mGCPList(0), mGeorefTransform(0)
{
}

void QgsGCPListModel::setGCPList(QgsGCPList *theGCPList)
{
  mGCPList = theGCPList;
  updateModel(true);
}

void QgsGCPListModel::setGeorefTransform(QgsGeorefTransform *theGeorefTransform)
{
  mGeorefTransform = theGeorefTransform;
  updateModel(true);
}

void QgsGCPListModel::onGCPListModified()
{
}

void QgsGCPListModel::onTransformationModified()
{
}

template <class T> QNumericItem<T> *create_item(const T value, bool isEditable = false)
{
  QNumericItem<T> *item = new QNumericItem<T>(value);
  item->setEditable(isEditable);
  return item;
}

QStandardItem *create_std_item(const QString &S, bool isEditable = false)
{
  QStandardItem *std_item = new QStandardItem(S);
  std_item->setEditable(isEditable);
  return std_item;
}

void QgsGCPListModel::updateModel(bool gcpsDirty)
{
  clear();
  if (!mGCPList)
    return;

  // Setup table header
  QStringList itemLabels;
  // Set column headers
  itemLabels<<"id"<<"srcX"<<"srcY"<<"dstX"<<"dstY"<<"dX"<<"dY"<<"residual";
  setColumnCount(itemLabels.size());
  setHorizontalHeaderLabels(itemLabels);

  setRowCount(mGCPList->size());


  if (gcpsDirty && mGeorefTransform) 
  {
    vector<QgsPoint> rC, mC;
    // TODO: move this vector extraction snippet into QgsGCPList
    for (int i = 0; i < mGCPList->size(); i++) {
      rC.push_back((*mGCPList)[i]->pixelCoords());
      mC.push_back((*mGCPList)[i]->mapCoords());
    }

    // TODO: the parameters should probable be updated externally (by user interaction)
    mGeorefTransform->updateParametersFromGCPs(mC, rC);
  }

  for (int i = 0; i < mGCPList->size(); i++) 
  {
    int j = 0;
    QgsGeorefDataPoint &p = *(*mGCPList)[i];
        
    setItem(i, j++, create_item<int>(i));
    setItem(i, j++, create_item<double>( p.pixelCoords().x() ));
    setItem(i, j++, create_item<double>(-p.pixelCoords().y() ));
    setItem(i, j++, create_item<double>( p.mapCoords().x() ));
    setItem(i, j++, create_item<double>( p.mapCoords().y() ));

    double residual = -1.f;
    double dX, dY;
    // Calculate residual if transform is available and up-to-date
    if (mGeorefTransform && mGeorefTransform->parametersInitialized()) 
    {
      QgsPoint dst; 
      // Transform from world to raster coordinate:
      // This is the transform direction used by the warp operation.
      // As transforms of order >=2 are not invertible, we are only
      // interested in the residual in this direction
      mGeorefTransform->transformWorldToRaster(p.mapCoords(), dst);
      dX = (dst.x() - p.pixelCoords().x());
      dY = (dst.y() - p.pixelCoords().y());
      residual = sqrt(dX*dX + dY*dY);
    }
    if (residual >= 0.f) {
      setItem(i, j++, create_item<double>(dX));
      setItem(i, j++, create_item<double>(-dY));
      setItem(i, j++, create_item<double>(residual));
    }
    else {
      setItem(i, j++, create_std_item("n/a"));
      setItem(i, j++, create_std_item("n/a"));
      setItem(i, j++, create_std_item("n/a"));
    }
  }
  //sort();  // Sort data
  //reset(); // Signal to views that the model has changed
}
