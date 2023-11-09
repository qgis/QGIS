/***************************************************************************
qgsmaptoolselectutils.h  -  Utility methods to help with select map tools
---------------------
begin                : May 2010
copyright            : (C) 2010 by Jeremy Palmer
email                : jpalmer at linz dot govt dot nz
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSMAPTOOLSELECTUTILS_H
#define QGSMAPTOOLSELECTUTILS_H

#include <QRect>
#include <QPoint>
#include <QList>
#include <QFutureWatcher>

#include "qgsvectorlayer.h"
#include "qgscoordinatetransform.h"
#include "qgsrendercontext.h"

class QMouseEvent;
class QgsMapCanvas;
class QgsVectorLayer;
class QgsVectorLayerFeatureSource;
class QgsGeometry;
class QgsRubberBand;
class QMenu;
class QgsHighlight;

/**
 * Namespace containing methods which are useful for the select maptool widgets
 */
namespace QgsMapToolSelectUtils
{

  /**
   * Calculates a list of features matching a selection geometry and flags.
   * \param canvas the map canvas used to get the current selected vector layer and
   * for any required geometry transformations
   * \param selectGeometry the geometry to select the layers features. This geometry
   * must be in terms of the canvas coordinate system.
   * \param doContains features will only be selected if fully contained within
   * the selection rubber band (otherwise intersection is enough).
   * \param singleSelect only selects the closest feature to the selectGeometry.
   * \returns list of features which match search geometry and parameters
   * \since QGIS 2.16
   */
  QgsFeatureIds getMatchingFeatures( QgsMapCanvas *canvas, const QgsGeometry &selectGeometry, bool doContains, bool singleSelect );

  /**
   * Selects the features within currently selected layer.
   * \param canvas the map canvas used to get the current selected vector layer and
   * for any required geometry transformations
   * \param selectGeometry the geometry to select the layers features. This geometry
   * must be in terms of the canvas coordinate system.
   * \param selectBehavior behavior of select (ie replace selection, add to selection)
   * \param doContains features will only be selected if fully contained within
   * the selection rubber band (otherwise intersection is enough).
   * \param singleSelect only selects the closest feature to the selectGeometry.
   * \since QGIS 2.16
  */
  void setSelectedFeatures( QgsMapCanvas *canvas,
                            const QgsGeometry &selectGeometry,
                            Qgis::SelectBehavior selectBehavior = Qgis::SelectBehavior::SetSelection,
                            bool doContains = true,
                            bool singleSelect = false );

  /**
   * Selects multiple matching features from within currently selected layer.
   * \param canvas the map canvas used to get the current selected vector layer and
   * for any required geometry transformations
   * \param selectGeometry the geometry to select the layers features. This geometry
   * must be in terms of the canvas coordinate system.
   * \param modifiers Keyboard modifiers are used to determine the current selection
   * operations (add, subtract, contains)
   * \see selectSingleFeature()
   * \since QGIS 2.16
  */
  void selectMultipleFeatures( QgsMapCanvas *canvas, const QgsGeometry &selectGeometry, Qt::KeyboardModifiers modifiers );

  /**
   * Selects a single feature from within currently selected layer.
   * \param canvas the map canvas used to get the current selected vector layer and
   * for any required geometry transformations
   * \param selectGeometry the geometry to select the layers features. This geometry
   * must be in terms of the canvas coordinate system.
   * \param modifiers Keyboard modifiers are used to determine the current selection
   * operations (add, subtract, contains)
   * \see selectMultipleFeatures()
  */
  void selectSingleFeature( QgsMapCanvas *canvas, const QgsGeometry &selectGeometry, Qt::KeyboardModifiers modifiers );

  /**
   * Get the current selected canvas map layer. Returns nullptr if it is not a selectable layer
   * \param canvas The map canvas used for getting the current layer
  */
  QgsMapLayer *getCurrentTargetLayer( QgsMapCanvas *canvas );

  /**
   * Expands a point to a rectangle with minimum size for selection based on the \a layer
   * \param point The point to expand the rectangle around (in map coordinates)
   * \param canvas The map canvas used to transform between canvas and map units
   * \param layer The target layer
   * \returns Expanded rectangle in map units
  */
  QgsRectangle expandSelectRectangle( QgsPointXY mapPoint, QgsMapCanvas *canvas, QgsMapLayer *layer );

  /**
   * Sets a QgsRubberband to rectangle in map units using a rectangle defined in device coords
   * \param canvas The map canvas used to transform the rectangle into map units
   * \param selectRect The input rectangle in device coords
   * \param rubberBand The rubberband that will be set in map units using the input rectangle
  */
  void setRubberBand( QgsMapCanvas *canvas, QRect &selectRect, QgsRubberBand *rubberBand );

  /**
   * Class that handles actions which can be displayed in a context menu related to feature selection.
   * \since QGIS 3.18
   */
  class QgsMapToolSelectMenuActions : public QObject
  {
      Q_OBJECT
    public:

      /**
      * Constructor
      * \param canvas The map canvas where where are the selected features
      * \param vectorLayer The target layer
      * \param behavior behavior of select
      * \param selectionGeometry the geometry used to select the feature
      * \param parent a QObject that owns the instance ot this class
      */
      QgsMapToolSelectMenuActions( QgsMapCanvas *canvas,
                                   QgsVectorLayer *vectorLayer,
                                   Qgis::SelectBehavior behavior,
                                   const QgsGeometry &selectionGeometry,
                                   QObject *parent = nullptr );

      ~QgsMapToolSelectMenuActions();

      /**
      * Populates the \a menu with "All Feature" action and a empty menu that could contain later the "One Feature" actions
      * Starts the search for canditate features to be selected on another thread, actions/menus will be updated at the end of this task
      */
      void populateMenu( QMenu *menu );

    private slots:
      void chooseAllCandidateFeature();
      void highlightAllFeatures();
      void onLayerDestroyed();
      void removeHighlight();
      void onSearchFinished();

    private:
      QgsMapCanvas *mCanvas = nullptr;
      QgsVectorLayer *mVectorLayer = nullptr;
      Qgis::SelectBehavior mBehavior = Qgis::SelectBehavior::SetSelection;
      QgsGeometry mSelectGeometry;
      QAction *mActionChooseAll = nullptr;
      QMenu *mMenuChooseOne = nullptr;
      QFutureWatcher<QgsFeatureIds> *mFutureWatcher = nullptr;
      QgsFeatureIds mAllFeatureIds;
      QList<QgsHighlight *> mHighlight;

      void startFeatureSearch();

      QString textForChooseAll( qint64 featureCount = -1 ) const;
      QString textForChooseOneMenu() const;
      void populateChooseOneMenu( const QgsFeatureIds &ids );

      static QgsFeatureIds filterIds( const QgsFeatureIds &ids,
                                      const QgsFeatureIds &existingSelection,
                                      Qgis::SelectBehavior behavior );

      struct DataForSearchingJob
      {
        bool isCanceled;
        std::unique_ptr<QgsVectorLayerFeatureSource> source;
        QgsGeometry selectGeometry;
        QgsCoordinateTransform ct;
        QgsRenderContext context;
        std::unique_ptr<QgsFeatureRenderer> featureRenderer;
        QString filterString;
        Qgis::SelectBehavior selectBehavior;
        QgsFeatureIds existingSelection;
      };

      std::shared_ptr<DataForSearchingJob> mJobData;

      static QgsFeatureIds search( std::shared_ptr<DataForSearchingJob> data );

      void chooseOneCandidateFeature( QgsFeatureId id );
      void highlightOneFeature( QgsFeatureId id );
  };
}

#endif
