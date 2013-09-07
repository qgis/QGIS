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
class APP_EXPORT QgsMapToolLabel: public QgsMapTool
{
  public:
    QgsMapToolLabel( QgsMapCanvas* canvas );
    ~QgsMapToolLabel();

    /**Returns true if label move can be applied to a layer
        @param xCol out: index of the attribute for data defined x coordinate
        @param yCol out: index of the attribute for data defined y coordinate
        @return true if labels of layer can be moved*/
    bool labelMoveable( const QgsMapLayer* ml, int& xCol, int& yCol ) const;
    /**Returns true if diagram move can be applied to a layer
        @param xCol out: index of the attribute for data defined x coordinate
        @param yCol out: index of the attribute for data defined y coordinate
        @return true if labels of layer can be moved*/
    bool diagramMoveable( const QgsMapLayer* ml, int& xCol, int& yCol ) const;
    /**Returns true if layer has attribute fields set up
        @param xCol out: index of the attribute for data defined x coordinate
        @param yCol out: index of the attribute for data defined y coordinate
        @return true if layer fields set up and exist*/
    bool layerCanPin( const QgsMapLayer* ml, int& xCol, int& yCol ) const;
    /**Returns true if layer has attribute field set up
      @param showCol out: attribute column for data defined label showing*/
    bool layerCanShowHide( const QgsMapLayer* layer, int& showCol ) const;
    /**Checks if labels in a layer can be rotated
      @param rotationCol out: attribute column for data defined label rotation*/
    bool layerIsRotatable( const QgsMapLayer* layer, int& rotationCol ) const;

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
      @param ignoreUpsideDown treat label as right-side-up
      @return true in case of success*/
    bool rotationPoint( QgsPoint& pos, bool ignoreUpsideDown = false, bool rotatingUnpinned = false );

    /**Creates label / feature / fixpoint rubber bands for the current label position*/
    void createRubberBands();

    /**Removes label / feature / fixpoint rubber bands*/
    void deleteRubberBands();

    /**Returns vector layer for current label position*/
    QgsVectorLayer* currentLayer();

    /**Returns layer settings of current label position*/
    QgsPalLayerSettings& currentLabelSettings( bool* ok );
//    const QgsPalLayerSettings& currentLabelSettings( bool* ok ) const;

    /**Returns current label's text
      @param trunc number of chars to truncate to, with ... added (added in 1.9)*/
    QString currentLabelText( int trunc = 0 );

    void currentAlignment( QString& hali, QString& vali );

    /**Gets vector feature for current label pos
      @return true in case of success*/
    bool currentFeature( QgsFeature& f, bool fetchGeom = false );

    /**Returns the font for the current feature (considering default font and data defined properties)*/
    QFont labelFontCurrentFeature();

//    /**Returns a data defined attribute column (added in 1.9)
//      @return invalid QVariant if one does not exist or an expression is used instead */
//    QVariant dataDefinedColumn( QgsPalLayerSettings::DataDefinedProperties p );

//    /**Returns a data defined attribute column - overloaded variation (added in 1.9)
//      @return invalid QVariant if one does not exist or an expression is used instead */
//    QVariant dataDefinedColumn( QgsPalLayerSettings::DataDefinedProperties p, QgsPalLayerSettings& labelSettings );

    /**Returns a data defined attribute column index (added in 1.9)
      @return -1 if column does not exist or an expression is used instead */
//    int dataDefinedColumnIndex( QgsPalLayerSettings::DataDefinedProperties p, const QgsPalLayerSettings& labelSettings, const QgsVectorLayer* vlayer ) const;

    int dataDefinedColumnIndex( QgsPalLayerSettings::DataDefinedProperties p, const QgsVectorLayer* vlayer ) const;

    /**Returns whether to preserve predefined rotation data during label pin/unpin operations*/
    bool preserveRotation();

    /**Get data defined position of a feature
      @param vlayer vector layer
      @param featureId feature identification integer
      @param x out: data defined x-coordinate
      @param xSuccess out: false if attribute value is NULL
      @param y out: data defined y-coordinate
      @param ySuccess out: false if attribute value is NULL
      @param xCol out: index of the x position column
      @param yCol out: index of the y position column
      @return false if layer does not have data defined label position enabled*/
    bool dataDefinedPosition( QgsVectorLayer* vlayer, int featureId, double& x, bool& xSuccess, double& y, bool& ySuccess, int& xCol, int& yCol ) const;

    /**Returns data defined rotation of a feature.
      @param vlayer vector layer
      @param featureId feature identification integer
      @param rotation out: rotation value
      @param rotationSuccess out: false if rotation value is NULL
      @param ignoreXY ignore that x and y are required to be data-defined
      @return true if data defined rotation is enabled on the layer
      */
    bool dataDefinedRotation( QgsVectorLayer* vlayer, int featureId, double& rotation, bool& rotationSuccess, bool ignoreXY = false ) const;

    /**Returns data defined show/hide of a feature.
      @param vlayer vector layer
      @param featureId feature identification integer
      @param show out: show/hide value
      @param showSuccess out: false if show/hide value is NULL
      @param showCol out: index of the show label column
      @return true if data defined show/hide is enabled on the layer
      */
    bool dataDefinedShowHide( QgsVectorLayer* vlayer, int featureId, int& show, bool& showSuccess, int& showCol ) const;

  private:
    QgsPalLayerSettings mInvalidLabelSettings;
};

#endif // QGSMAPTOOLLABEL_H
