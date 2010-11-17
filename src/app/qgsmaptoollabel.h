/***************************************************************************
                          qgsmaptoollabel.h
                          --------------------
    begin                : 2010-11-03
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLLABEL_H
#define QGSMAPTOOLLABEL_H

#include "qgsmaptool.h"
#include "qgsmaprenderer.h"
#include "qgspallabeling.h"
#include "qgspoint.h"
class QgsRubberBand;

/**Base class for map tools that modify label properties*/
class QgsMapToolLabel: public QgsMapTool
{
  public:
    QgsMapToolLabel( QgsMapCanvas* canvas );
    ~QgsMapToolLabel();

  protected:
    QgsRubberBand* mLabelRubberBand;
    QgsRubberBand* mFeatureRubberBand;
    /**Shows label fixpoint (left/bottom by default)*/
    QgsRubberBand* mFixPointRubberBand;

    /**Currently dragged label position*/
    QgsLabelPosition mCurrentLabelPos;

    /**Returns label position for mouse click location
      @param e mouse event
      @param p out: label position
      @return true in case of success, false if no label at this location*/
    bool labelAtPosition( QMouseEvent* e, QgsLabelPosition& p );

    /**Finds out rotation point of current label position
      @return true in case of success*/
    bool rotationPoint( QgsPoint& pos );

    /**Creates label / feature / fixpoint rubber bands for the current label position*/
    void createRubberBands();

    /**Removes label / feature / fixpoint rubber bands*/
    void deleteRubberBands();

    /**Returns vector layer for current label position*/
    QgsVectorLayer* currentLayer();

    /**Returns layer settings of current label position*/
    QgsPalLayerSettings& currentLabelSettings( bool* ok );

    QString currentLabelText();

    void currentAlignment( QString& hali, QString& vali );

    /**Gets vector feature for current label pos
      @return true in case of success*/
    bool currentFeature( QgsFeature& f, bool fetchGeom = false );

    /**Returns the font for the current feature (considering default font and data defined properties*/
    QFont labelFontCurrentFeature();

  private:
    QgsPalLayerSettings mInvalidLabelSettings;
};

#endif // QGSMAPTOOLLABEL_H
