/***************************************************************************
                         qgsbasicrelationwidget.h
                         ----------------------
    begin                : October 2020
    copyright            : (C) 2020 by Ivan Ivanov
    email                : ivan@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBASICRELATIONWIDGET_H
#define QGSBASICRELATIONWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QButtonGroup>
#include <QGridLayout>
#include "qobjectuniqueptr.h"

#include "ui_qgsrelationeditorconfigwidgetbase.h"

#include "qgsrelationwidget.h"
#include "qgsrelationconfigwidget.h"
#include "qgsrelationwidgetfactory.h"
#include "qgsrelationeditorwidget.h"
#include "qobjectuniqueptr.h"
#include "qgsattributeeditorcontext.h"
#include "qgscollapsiblegroupbox.h"
#include "qgsdualview.h"
#include "qgsrelation.h"
#include "qgsvectorlayerselectionmanager.h"
#include "qgis_gui.h"

class QgsFeature;
class QgsVectorLayer;
class QgsVectorLayerTools;
class QgsMapTool;
class QgsMapToolDigitizeFeature;


/**
 * The default relation widget in QGIS. Successor of the now deprecated {\see QgsRelationEditorWidget}.
 * \ingroup gui
 * \class QgsBasicRelationWidget
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsBasicRelationWidget : public QgsRelationWidget
{

    Q_OBJECT
    Q_PROPERTY( QgsDualView::ViewMode viewMode READ viewMode WRITE setViewMode )
    Q_PROPERTY( Buttons visibleButtons READ visibleButtons WRITE setVisibleButtons )

  public:

    /**
     * Possible buttons shown in the relation editor
     */
    enum Button
    {
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
    QgsBasicRelationWidget( const QVariantMap &config, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! Define the view mode for the dual view
    void setViewMode( QgsDualView::ViewMode mode );

    //! Gets the view mode for the dual view
    QgsDualView::ViewMode viewMode() {return mViewMode;}

    /**
     * Sets the editor \a context
     * \note if context cadDockWidget is null, it won't be possible to digitize
     * the geometry of a referencing feature from this widget
     */
    void setEditorContext( const QgsAttributeEditorContext &context );

    /**
     * Defines the buttons which are shown
     */
    void setVisibleButtons( const Buttons &buttons );

    /**
     * Returns the buttons which are shown
     */
    Buttons visibleButtons() const;

    /**
     * Returns the current configuration
     */
    QVariantMap config() const override;

    /**
     * Defines the current configuration
     */
    void setConfig( const QVariantMap &config ) override;

    /**
      * Sets the title of the root groupbox
      */
    void setTitle( const QString &title ) override;

  public slots:
    void parentFormValueChanged( const QString &attribute, const QVariant &newValue ) override;

  private slots:
    void setViewMode( int mode ) {setViewMode( static_cast<QgsDualView::ViewMode>( mode ) );}
    void updateButtons();

    void addFeatureGeometry();
    void toggleEditing( bool state );
    void onCollapsedStateChanged( bool collapsed );
    void showContextMenu( QgsActionMenu *menu, QgsFeatureId fid );
    void mapToolDeactivated();
    void onKeyPressed( QKeyEvent *e );
    void onDigitizingCompleted( const QgsFeature &feature );

  private:
    void updateUi() override;
    void initDualView( QgsVectorLayer *layer, const QgsFeatureRequest &request );
    void setMapTool( QgsMapTool *mapTool );
    void unsetMapTool();

    QgsCollapsibleGroupBox *mRootCollapsibleGroupBox = nullptr;
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
    QGridLayout *mRelationLayout = nullptr;
    QObjectUniquePtr<QgsMapToolDigitizeFeature> mMapToolDigitize;
    QButtonGroup *mViewModeButtonGroup = nullptr;

    Buttons mButtonsVisibility = Button::AllButtons;
    bool mVisible = true;

    void beforeSetRelationFeature( const QgsRelation &newRelation, const QgsFeature &newFeature ) override;
    void afterSetRelationFeature() override;
    void beforeSetRelations( const QgsRelation &newRelation, const QgsRelation &newNmRelation ) override;
    void afterSetRelations() override;
};


#ifndef SIP_RUN

/**
 * Factory class for creating a basic relation widget and the respective config widget.
 * \ingroup gui
 * \class QgsBasicRelationWidgetFactory
 * \note not available in Python bindings
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsBasicRelationWidgetFactory : public QgsRelationWidgetFactory
{
  public:
    QgsBasicRelationWidgetFactory();

    QString type() const override;

    QString name() const override;

    QgsRelationWidget *create( const QVariantMap &config, QWidget *parent = nullptr ) const override;

    QgsRelationConfigWidget *configWidget( const QgsRelation &relation, QWidget *parent ) const override;

};

/**
 * \ingroup gui
 * \class QgsBasicRelationConfigWidget
 * Creates a new configuration widget for the basic relation widget
 * \since QGIS 3.18
 */
class QgsBasicRelationConfigWidget : public QgsRelationConfigWidget, private Ui::QgsRelationEditorConfigWidgetBase
{
  public:

    /**
     * Create a new configuration widget
     *
     * \param relation    The relation for which the configuration dialog will be created
     * \param parent      A parent widget
     */
    explicit QgsBasicRelationConfigWidget( const QgsRelation &relation, QWidget *parent SIP_TRANSFERTHIS );

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
#endif

#endif // QGSBASICRELATIONWIDGET_H
