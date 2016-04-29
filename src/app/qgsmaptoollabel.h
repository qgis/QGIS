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

/** Base class for map tools that modify label properties*/
class APP_EXPORT QgsMapToolLabel: public QgsMapTool
{
    Q_OBJECT

  public:
    QgsMapToolLabel( QgsMapCanvas* canvas );
    ~QgsMapToolLabel();

    /** Returns true if label move can be applied to a layer
        @param xCol out: index of the attribute for data defined x coordinate
        @param yCol out: index of the attribute for data defined y coordinate
        @return true if labels of layer can be moved*/
    bool labelMoveable( QgsVectorLayer* vlayer, int& xCol, int& yCol ) const;
    bool labelMoveable( QgsVectorLayer* vlayer, const QgsPalLayerSettings& settings, int& xCol, int& yCol ) const;
    /** Returns true if diagram move can be applied to a layer
        @param xCol out: index of the attribute for data defined x coordinate
        @param yCol out: index of the attribute for data defined y coordinate
        @return true if labels of layer can be moved*/
    bool diagramMoveable( QgsVectorLayer* vlayer, int& xCol, int& yCol ) const;
    /** Returns true if layer has attribute fields set up
        @param xCol out: index of the attribute for data defined x coordinate
        @param yCol out: index of the attribute for data defined y coordinate
        @return true if layer fields set up and exist*/
    bool layerCanPin( QgsVectorLayer* vlayer, int& xCol, int& yCol ) const;
    /** Returns true if layer has attribute field set up for diagrams
      @param showCol out: attribute column for data defined diagram showing
      @note added in QGIS 2.16 */
    bool diagramCanShowHide( QgsVectorLayer* vlayer, int& showCol ) const;
    /** Returns true if layer has attribute field set up
      @param showCol out: attribute column for data defined label showing*/
    bool labelCanShowHide( QgsVectorLayer* vlayer, int& showCol ) const;
    /** Checks if labels in a layer can be rotated
      @param rotationCol out: attribute column for data defined label rotation*/
    bool layerIsRotatable( QgsVectorLayer *layer, int& rotationCol ) const;
    bool labelIsRotatable( QgsVectorLayer *layer, const QgsPalLayerSettings& settings, int& rotationCol ) const;

  protected:
    QgsRubberBand* mLabelRubberBand;
    QgsRubberBand* mFeatureRubberBand;
    /** Shows label fixpoint (left/bottom by default)*/
    QgsRubberBand* mFixPointRubberBand;

    struct LabelDetails
    {
      LabelDetails(): valid( false ), layer( nullptr ) {}
      explicit LabelDetails( const QgsLabelPosition& p );
      bool valid;
      QgsLabelPosition pos;
      QgsVectorLayer* layer;
      QgsPalLayerSettings settings;
    };

    /** Currently dragged label position*/
    LabelDetails mCurrentLabel;


    /** Returns label position for mouse click location
      @param e mouse event
      @param p out: label position
      @return true in case of success, false if no label at this location*/
    bool labelAtPosition( QMouseEvent* e, QgsLabelPosition& p );

    /** Finds out rotation point of current label position
      @param ignoreUpsideDown treat label as right-side-up
      @return true in case of success*/
    bool currentLabelRotationPoint( QgsPoint& pos, bool ignoreUpsideDown = false, bool rotatingUnpinned = false );

    /** Creates label / feature / fixpoint rubber bands for the current label position*/
    void createRubberBands();

    /** Removes label / feature / fixpoint rubber bands*/
    void deleteRubberBands();

    /** Returns current label's text
      @param trunc number of chars to truncate to, with ... added */
    QString currentLabelText( int trunc = 0 );

    void currentAlignment( QString& hali, QString& vali );

    /** Gets vector feature for current label pos
      @return true in case of success*/
    bool currentFeature( QgsFeature& f, bool fetchGeom = false );

    /** Returns the font for the current feature (considering default font and data defined properties)*/
    QFont currentLabelFont();

    /** Returns a data defined attribute column name for particular property or empty string if not defined */
    QString dataDefinedColumnName( QgsPalLayerSettings::DataDefinedProperties p, const QgsPalLayerSettings& labelSettings ) const;

    /** Returns a data defined attribute column index
      @return -1 if column does not exist or an expression is used instead */
    int dataDefinedColumnIndex( QgsPalLayerSettings::DataDefinedProperties p, const QgsPalLayerSettings& labelSettings, const QgsVectorLayer* vlayer ) const;

    /** Returns whether to preserve predefined rotation data during label pin/unpin operations*/
    bool currentLabelPreserveRotation();

    /** Get data defined position of current label
      @param x out: data defined x-coordinate
      @param xSuccess out: false if attribute value is NULL
      @param y out: data defined y-coordinate
      @param ySuccess out: false if attribute value is NULL
      @param xCol out: index of the x position column
      @param yCol out: index of the y position column
      @return false if layer does not have data defined label position enabled*/
    bool currentLabelDataDefinedPosition( double& x, bool& xSuccess, double& y, bool& ySuccess, int& xCol, int& yCol ) const;

    /** Returns data defined rotation of current label
      @param rotation out: rotation value
      @param rotationSuccess out: false if rotation value is NULL
      @param rCol out: index of the rotation column
      @param ignoreXY ignore that x and y are required to be data-defined
      @return true if data defined rotation is enabled on the layer
      */
    bool currentLabelDataDefinedRotation( double& rotation, bool& rotationSuccess, int& rCol, bool ignoreXY = false ) const;

    /** Returns data defined show/hide of a feature.
      @param vlayer vector layer
      @param featureId feature identification integer
      @param show out: show/hide value
      @param showSuccess out: false if show/hide value is NULL
      @param showCol out: index of the show label column
      @return true if data defined show/hide is enabled on the layer
      */
    bool dataDefinedShowHide( QgsVectorLayer* vlayer, QgsFeatureId featureId, int& show, bool& showSuccess, int& showCol ) const;

    /** Returns the pin status for the current label/diagram
      @return true if the label/diagram is pinned, false otherwise
      @note added in QGIS 2.16
      */
    bool isPinned();
};

#endif // QGSMAPTOOLLABEL_H
