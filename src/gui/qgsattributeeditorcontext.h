/***************************************************************************
    qgsattributeeditorcontext.h
     --------------------------------------
    Date                 : 30.7.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTEEDITORCONTEXT_H
#define QGSATTRIBUTEEDITORCONTEXT_H

#include <QMap>
#include <QWidget>
#include <QMetaEnum>

#include "qgsdistancearea.h"
#include "qgsvectorlayertools.h"
#include "qgsvectorlayer.h"
#include "qgis_gui.h"
#include "qgsproject.h"

class QgsMapCanvas;
class QgsAdvancedDigitizingDockWidget;
class QgsMessageBar;

/**
 * \ingroup gui
 * \brief This class contains context information for attribute editor widgets.
 * It will be passed to embedded widgets whenever this occurs (e.g. when
 * showing an embedded form due to relations)
 */

class GUI_EXPORT QgsAttributeEditorContext
{
    Q_GADGET

  public:

    //! modes
    enum Mode
    {
      SingleEditMode, //!< Single edit mode, for editing a single feature
      AddFeatureMode, /*!< Add feature mode, for setting attributes for a new feature. In this mode the dialog will be editable even with an invalid feature and
      will add a new feature when the form is accepted. */
      FixAttributeMode, //!< Fix feature mode, for modifying the feature attributes without saving. The updated feature is available via `feature()` after `save()`
      MultiEditMode, //!< Multi edit mode, for editing fields of multiple features at once
      SearchMode, //!< Form values are used for searching/filtering the layer
      AggregateSearchMode, //!< Form is in aggregate search mode, show each widget in this mode
      IdentifyMode //!< Identify the feature
    };
    Q_ENUM( Mode )

    /**
     * Determines in which direction a relation was resolved.
     */
    enum RelationMode
    {
      Undefined,  //!< This context is not defined by a relation
      Multiple,   //!< When showing a list of features (e.g. houses as an embedded form in a district form)
      Single      //!< When showing a single feature (e.g. district information when looking at the form of a house)
    };

    enum FormMode
    {
      Embed,            //!< A form was embedded as a widget on another form
      StandaloneDialog, //!< A form was opened as a new dialog
      Popup             //!< A widget was opened as a popup (e.g. attribute table editor widget)
    };

    QgsAttributeEditorContext() = default;

    QgsAttributeEditorContext( const QgsAttributeEditorContext &parentContext, FormMode formMode )
      : mParentContext( &parentContext )
      , mVectorLayerTools( parentContext.mVectorLayerTools )
      , mMapCanvas( parentContext.mMapCanvas )
      , mMainMessageBar( parentContext.mMainMessageBar )
      , mCadDockWidget( parentContext.mCadDockWidget )
      , mDistanceArea( parentContext.mDistanceArea )
      , mFormFeature( parentContext.mFormFeature )
      , mFormMode( formMode )
    {
      Q_ASSERT( parentContext.vectorLayerTools() );
    }

    QgsAttributeEditorContext( const QgsAttributeEditorContext &parentContext, const QgsRelation &relation, RelationMode relationMode, FormMode widgetMode )
      : mParentContext( &parentContext )
      , mVectorLayerTools( parentContext.mVectorLayerTools )
      , mMapCanvas( parentContext.mMapCanvas )
      , mMainMessageBar( parentContext.mMainMessageBar )
      , mCadDockWidget( parentContext.mCadDockWidget )
      , mDistanceArea( parentContext.mDistanceArea )
      , mRelation( relation )
      , mRelationMode( relationMode )
      , mFormMode( widgetMode )
    {
      Q_ASSERT( parentContext.vectorLayerTools() );
    }

    /**
     * Sets distance area object, \a distanceArea, for area/length calculations
     * \see distanceArea()
     */
    inline void setDistanceArea( const QgsDistanceArea &distanceArea )
    {
      if ( mLayer )
      {
        mDistanceArea = distanceArea;
        mDistanceArea.setSourceCrs( mLayer->crs(), QgsProject::instance()->transformContext() );
      }
    }

    /**
     * Returns the distance area object used for area/length calculations.
     * \see setDistanceArea()
     */
    inline const QgsDistanceArea &distanceArea() const { return mDistanceArea; }

    /**
     * Sets the associated map canvas, \a mapCanvas, (e.g. to zoom to related features).
     * \see mapCanvas()
     * \since QGIS 3.2
     */
    inline void setMapCanvas( QgsMapCanvas *mapCanvas ) { mMapCanvas = mapCanvas; }

    /**
     * Returns the associated map canvas (e.g. to zoom to related features).
     * \see setMapCanvas()
     * \since QGIS 3.2
     */
    inline QgsMapCanvas *mapCanvas() const { return mMapCanvas; }

    /**
     * Sets the associated CAD dock widget, \a cadDockWidget, (e.g. to be used in map tools).
     * \note Unstable API. This method is unstable API and may be modified or removed at any time.
     * \see cadDockWidget()
     * \since QGIS 3.10
     */
    void setCadDockWidget( QgsAdvancedDigitizingDockWidget *cadDockWidget );

    /**
     * Returns the associated CAD dock widget (e.g. to be used in map tools).
     * \note Unstable API. This method is unstable API and may be modified or removed at any time.
     * \see setCadDockWidget()
     * \since QGIS 3.10
     */
    QgsAdvancedDigitizingDockWidget *cadDockWidget() const { return mCadDockWidget; }

    /**
     * Sets the associated vector layer tools.
     * \param vlTools vector layer tools
     * \see vectorLayerTools()
     */
    inline void setVectorLayerTools( QgsVectorLayerTools *vlTools ) { mVectorLayerTools = vlTools; }
    // TODO QGIS 4.0 - rename vlTools to tools

    /**
     * Returns the associated vector layer tools.
     * \see setVectorLayerTools()
     */
    inline const QgsVectorLayerTools *vectorLayerTools() const { return mVectorLayerTools; }

    /**
     * Set attribute relation and mode
     * \param relation relation
     * \param mode relation mode
     * \see relation()
     * \see relationMode()
     */
    inline void setRelation( const QgsRelation &relation, RelationMode mode ) { mRelation = relation; mRelationMode = mode; }

    /**
     * Returns the attribute relation.
     * \see setRelation()
     * \see relationMode()
     */
    inline const QgsRelation &relation() const { return mRelation; }

    /**
     * Returns the attribute relation mode.
     * \see setRelation()
     * \see relation()
     */
    inline RelationMode relationMode() const { return mRelationMode; }

    /**
     * Returns the form mode.
     * \see setFormMode()
     */
    inline FormMode formMode() const { return mFormMode; }

    /**
     * Sets the form mode.
     * \param mode form mode
     * \see formMode()
     */
    inline void setFormMode( FormMode mode ) { mFormMode = mode; }

    /**
     * Returns TRUE if the attribute editor should permit use of custom UI forms.
     * \see setAllowCustomUi()
     */
    bool allowCustomUi() const { return mAllowCustomUi; }

    /**
     * Sets whether the attribute editor should permit use of custom UI forms.
     * \param allow set to TRUE to allow custom UI forms, or FALSE to disable them and use default generated
     * QGIS forms
     * \see allowCustomUi()
     */
    void setAllowCustomUi( bool allow ) { mAllowCustomUi = allow; }

    inline const QgsAttributeEditorContext *parentContext() const { return mParentContext; }

    /**
     * Returns current feature from the currently edited form or table row
     * \see setFormFeature()
     * \since QGIS 3.2
     */
    QgsFeature formFeature() const { return mFormFeature; }

    /**
     * Set current \a feature for the currently edited form or table row
     * \see formFeature()
     * \since QGIS 3.2
     */
    void setFormFeature( const QgsFeature &feature ) { mFormFeature = feature ; }

    /**
     * Returns the feature of the currently edited parent form in its actual state
     * \see setParentFormFeature()
     * \since QGIS 3.14
     */
    QgsFeature parentFormFeature() const { return mParentFormFeature; }

    /**
     * Sets the \a feature of the currently edited parent form
     * \see parentFormFeature()
     * \since QGIS 3.14
     */
    void setParentFormFeature( const QgsFeature &feature ) { mParentFormFeature = feature ; }

    /**
     * Returns current attributeFormMode
     * \since QGIS 3.4
     */
    Mode attributeFormMode() const { return mAttributeFormMode; }

    /**
     * Set \a attributeFormMode for the edited form
     * \since QGIS 3.4
     */
    void setAttributeFormMode( const Mode &attributeFormMode ) { mAttributeFormMode = attributeFormMode; }

    /**
     * Returns given \a attributeFormMode as string
     * \since QGIS 3.4
     */
    QString attributeFormModeString() const
    {
      const QMetaEnum metaEnum( QMetaEnum::fromType<Mode>() );
      return metaEnum.valueToKey( static_cast<int>( mAttributeFormMode ) );
    }

    /**
     * Set current \a messageBar as the main message bar
     * \since QGIS 3.12
     */
    void setMainMessageBar( QgsMessageBar *messageBar ) { mMainMessageBar = messageBar; }

    /**
     * Returns the main message bar
     * \since QGIS 3.12
     */
    QgsMessageBar *mainMessageBar() { return mMainMessageBar; }

  private:
    const QgsAttributeEditorContext *mParentContext = nullptr;
    QgsVectorLayer *mLayer = nullptr;
    QgsVectorLayerTools *mVectorLayerTools = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;
    QgsMessageBar *mMainMessageBar = nullptr;
    QgsAdvancedDigitizingDockWidget *mCadDockWidget = nullptr;
    QgsDistanceArea mDistanceArea;
    QgsRelation mRelation;
    RelationMode mRelationMode = Undefined;
    //! Store the values of the currently edited form or table row
    QgsFeature mFormFeature;
    //! Store the values of the currently edited parent form or table row
    QgsFeature mParentFormFeature;
    FormMode mFormMode = Embed;
    bool mAllowCustomUi = true;
    Mode mAttributeFormMode = SingleEditMode;
};

#endif // QGSATTRIBUTEEDITORCONTEXT_H
