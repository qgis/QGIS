/***************************************************************************
    qgsrelationeditorwidget.h
     --------------------------------------
    Date                 : 17.5.2013
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

#ifndef QGSRELATIONEDITORWIDGET_H
#define QGSRELATIONEDITORWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QButtonGroup>
#include <QGridLayout>
#include "qobjectuniqueptr.h"

#include "ui_qgsrelationeditorconfigwidgetbase.h"

#include "qgsabstractrelationeditorwidget.h"
#include "qobjectuniqueptr.h"
#include "qgsattributeeditorcontext.h"
#include "qgsdualview.h"
#include "qgsrelation.h"
#include "qgsvectorlayerselectionmanager.h"
#include "qgis_gui.h"

class QTreeWidget;
class QTreeWidgetItem;
class QgsFeature;
class QgsVectorLayer;
class QgsVectorLayerTools;
class QgsMapTool;
class QgsMapToolDigitizeFeature;

#ifdef SIP_RUN
% ModuleHeaderCode
// fix to allow compilation with sip that for some reason
// doesn't add this include to the file where the code from
// ConvertToSubClassCode goes.
#include <qgsrelationeditorwidget.h>
% End
#endif


/// @cond PRIVATE
#ifndef SIP_RUN

/**
 * This class is used to filter the current vector layer selection to features matching the given request.
 * Relation editor widget use it in order to get selected feature for the current relation.
 */
class QgsFilteredSelectionManager : public QgsVectorLayerSelectionManager
{
    Q_OBJECT

  public:
    QgsFilteredSelectionManager( QgsVectorLayer *layer, const QgsFeatureRequest &request, QObject *parent = nullptr );

    const QgsFeatureIds &selectedFeatureIds() const override;
    int selectedFeatureCount() override;

  private slots:

    void onSelectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &deselected, bool clearAndSelect ) override;

  private:

    QgsFeatureRequest mRequest;
    QgsFeatureIds mSelectedFeatureIds;
};
#endif
/// @endcond


/**
 * The default relation widget in QGIS.
 * \ingroup gui
 * \class QgsRelationEditorWidget
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsRelationEditorWidget : public QgsAbstractRelationEditorWidget
{

    Q_OBJECT
    Q_PROPERTY( QgsDualView::ViewMode viewMode READ viewMode WRITE setViewMode )
    Q_PROPERTY( Buttons visibleButtons READ visibleButtons WRITE setVisibleButtons )

  public:

    /**
     * Possible buttons shown in the relation editor
     * \since QGIS 3.18
     */
    enum Button
    {
      NoButton = 0, //!< No button (since QGIS 3.20)
      Link = 1 << 1, //!< Link button
      Unlink = 1 << 2, //!< Unlink button
      SaveChildEdits = 1 << 3, //!< Save child edits button
      AddChildFeature = 1 << 4, //!< Add child feature (as in some projects we only want to allow linking/unlinking existing features)
      DuplicateChildFeature = 1 << 5, //!< Duplicate child feature
      DeleteChildFeature = 1 << 6, //!< Delete child feature button
      ZoomToChildFeature = 1 << 7, //!< Zoom to child feature
      AllButtons = Link | Unlink | SaveChildEdits | AddChildFeature | DuplicateChildFeature | DeleteChildFeature | ZoomToChildFeature //!< All buttons
    };
    Q_ENUM( Button )
    Q_DECLARE_FLAGS( Buttons, Button )
    Q_FLAG( Buttons )

    /**
     * Constructor
     * \param config widget configuration
     * \param parent parent widget
     */
    QgsRelationEditorWidget( const QVariantMap &config, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! Define the view mode for the dual view
    void setViewMode( QgsDualView::ViewMode mode );

    //! Gets the view mode for the dual view
    QgsDualView::ViewMode viewMode() {return mViewMode;}

    /**
     * The feature selection manager is responsible for the selected features
     * which are currently being edited.
     */
    QgsIFeatureSelectionManager *featureSelectionManager();

    /**
     * Sets the editor \a context
     * \note if context cadDockWidget is null, it won't be possible to digitize
     * the geometry of a referencing feature from this widget
     */
    void setEditorContext( const QgsAttributeEditorContext &context ) override;

    /**
     * Defines the buttons which are shown
     */
    void setVisibleButtons( const Buttons &buttons );

    /**
     * Returns the buttons which are shown
     */
    Buttons visibleButtons() const;

    /**
     * Duplicates a feature
     * \deprecated since QGIS 3.18, use duplicateSelectedFeatures() instead
     */
    Q_DECL_DEPRECATED void duplicateFeature() SIP_DEPRECATED;

    /**
     * Duplicates the selected features
     * \since QGIS 3.18
     */
    void duplicateSelectedFeatures();

    /**
     * Unlinks the selected features from the relation
     */
    void unlinkSelectedFeatures();

    /**
     * Deletes the currently selected features
     */
    void deleteSelectedFeatures();

    /**
     * Zooms to the selected features
     */
    void zoomToSelectedFeatures();

    /**
     * Returns the current configuration
     */
    QVariantMap config() const override;

    /**
     * Defines the current configuration
     */
    void setConfig( const QVariantMap &config ) override;

  public slots:
    void parentFormValueChanged( const QString &attribute, const QVariant &newValue ) override;

  protected:
    virtual void updateUi() override;
    void beforeSetRelationFeature( const QgsRelation &newRelation, const QgsFeature &newFeature ) override;
    void afterSetRelationFeature() override;
    void beforeSetRelations( const QgsRelation &newRelation, const QgsRelation &newNmRelation ) override;
    void afterSetRelations() override;

  private slots:
    void setViewMode( int mode ) {setViewMode( static_cast<QgsDualView::ViewMode>( mode ) );}
    void updateButtons();

    void addFeature();
    void addFeatureGeometry();
    void toggleEditing( bool state );
    void showContextMenu( QgsActionMenu *menu, QgsFeatureId fid );
    void mapToolDeactivated();
    void onKeyPressed( QKeyEvent *e );
    void onDigitizingCompleted( const QgsFeature &feature );

    void multiEditItemSelectionChanged();

  private:

    enum class MultiEditFeatureType : int
    {
      Parent,
      Child
    };

    enum class MultiEditTreeWidgetRole : int
    {
      FeatureType = Qt::UserRole + 1,
      FeatureId = Qt::UserRole + 2
    };

    void initDualView( QgsVectorLayer *layer, const QgsFeatureRequest &request );
    void setMapTool( QgsMapTool *mapTool );
    void unsetMapTool();
    QgsFeatureIds selectedChildFeatureIds() const;
    void updateUiSingleEdit();
    void updateUiMultiEdit();
    QTreeWidgetItem *createMultiEditTreeWidgetItem( const QgsFeature &feature, QgsVectorLayer *layer, MultiEditFeatureType type );

    QgsDualView *mDualView = nullptr;
    QPointer<QgsMessageBarItem> mMessageBarItem;
    QgsDualView::ViewMode mViewMode = QgsDualView::AttributeEditor;

    QToolButton *mToggleEditingButton = nullptr;
    QToolButton *mSaveEditsButton = nullptr;
    QToolButton *mAddFeatureButton = nullptr;
    QToolButton *mDuplicateFeatureButton = nullptr;
    QToolButton *mDeleteFeatureButton = nullptr;
    QToolButton *mLinkFeatureButton = nullptr;
    QToolButton *mUnlinkFeatureButton = nullptr;
    QToolButton *mZoomToFeatureButton = nullptr;
    QToolButton *mFormViewButton = nullptr;
    QToolButton *mTableViewButton = nullptr;
    QToolButton *mAddFeatureGeometryButton = nullptr;
    QLabel *mMultiEditInfoLabel = nullptr;
    QStackedWidget *mStackedWidget = nullptr;
    QWidget *mMultiEditStackedWidgetPage = nullptr;
    QTreeWidget *mMultiEditTreeWidget = nullptr;
    QObjectUniquePtr<QgsMapToolDigitizeFeature> mMapToolDigitize;
    QButtonGroup *mViewModeButtonGroup = nullptr;
    QgsVectorLayerSelectionManager *mFeatureSelectionMgr = nullptr;

    Buttons mButtonsVisibility = Button::AllButtons;
    bool mShowFirstFeature = true;

    QList<QTreeWidgetItem *> mMultiEditPreviousSelectedItems;
    QgsFeatureIds mMultiEdit1NJustAddedIds;

    friend class TestQgsRelationEditorWidget;
};


/**
 * \ingroup gui
 * \class QgsRelationEditorConfigWidget
 * \brief Creates a new configuration widget for the relation editor widget
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsRelationEditorConfigWidget : public QgsAbstractRelationEditorConfigWidget, private Ui::QgsRelationEditorConfigWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Create a new configuration widget
     *
     * \param relation    The relation for which the configuration dialog will be created
     * \param parent      A parent widget
     */
    explicit QgsRelationEditorConfigWidget( const QgsRelation &relation, QWidget *parent SIP_TRANSFERTHIS );

    /**
     * \brief Create a configuration from the current GUI state
     *
     * \returns A widget configuration
     */
    QVariantMap config();

    /**
     * \brief Update the configuration widget to represent the given configuration.
     *
     * \param config The configuration which should be represented by this widget
     */
    void setConfig( const QVariantMap &config );

};


#ifndef SIP_RUN

/**
 * Factory class for creating a relation editor widget and the respective config widget.
 * \ingroup gui
 * \class QgsRelationEditorWidgetFactory
 * \note not available in Python bindings
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsRelationEditorWidgetFactory : public QgsAbstractRelationEditorWidgetFactory
{
  public:
    QgsRelationEditorWidgetFactory();

    QString type() const override;

    QString name() const override;

    QgsAbstractRelationEditorWidget *create( const QVariantMap &config, QWidget *parent = nullptr ) const override;

    QgsAbstractRelationEditorConfigWidget *configWidget( const QgsRelation &relation, QWidget *parent ) const override;

};
#endif


#endif // QGSRELATIONEDITORWIDGET_H
