/***************************************************************************
                         qgslegendmodel.h  -  description
                         -----------------
    begin                : June 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLEGENDMODEL_H
#define QGSLEGENDMODEL_H

#include <QStandardItemModel>
#include <QStringList>
#include <QSet>

class QDomDocument;
class QDomElement;
class QgsMapLayer;
class QgsSymbol;

/** \ingroup MapComposer
 * A model that provides layers as root items. The classification items are 
 * children of the layer items.
 */
class QgsLegendModel: public QStandardItemModel
{
  Q_OBJECT

 public:
  QgsLegendModel();
  ~QgsLegendModel();

  void setLayerSet(const QStringList& layerIds);
  /**Apply added or removed layers to the model*/
  void updateLayerEntries(const QStringList& newLayerIds);

  bool writeXML(QDomElement& composerLegendElem, QDomDocument& doc);
  bool readXML(const QDomElement& legendModelElem, const QDomDocument& doc);

 public slots:
  void removeLayer(const QString& layerId);
  void addLayer(QgsMapLayer* theMapLayer);

 signals:
  void layersChanged();

 private:
  /**Adds classification items of vector layers
   @return 0 in case of success*/
  int addVectorLayerItems(QStandardItem* layerItem, QgsMapLayer* vlayer);

  /**Adds item of raster layer
   @return 0 in case of success*/
  int addRasterLayerItem(QStandardItem* layerItem, QgsMapLayer* rlayer);

  /**Insert a symbol into QgsLegendModel symbol storage*/
  void insertSymbol(QgsSymbol* s);
  /**Removes and deletes a symbol*/
  void removeSymbol(QgsSymbol* s);
  /**Removes and deletes all stored symbols*/
  void removeAllSymbols();

  /**Keep track of copied symbols to delete them if not used anymore*/
  QSet<QgsSymbol*> mSymbols;

 protected:
  QStringList mLayerIds;
};

#endif
