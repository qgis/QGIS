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

#ifndef QGSRELATIONEDITOR_H
#define QGSRELATIONEDITOR_H

#include <QWidget>
#include <QToolButton>
#include <QButtonGroup>
#include <QGridLayout>

#include "qgsattributeeditorcontext.h"
#include "qgscollapsiblegroupbox.h"
#include "qgsdualview.h"
#include "qgsrelation.h"
#include "qgis_gui.h"

class QgsFeature;
class QgsGenericFeatureSelectionManager;
class QgsVectorLayer;
class QgsVectorLayerTools;

#ifdef SIP_RUN
% ModuleHeaderCode
// fix to allow compilation with sip that for some reason
// doesn't add this include to the file where the code from
// ConvertToSubClassCode goes.
#include <qgsrelationeditorwidget.h>
% End
#endif

/**
 * \ingroup gui
 * \class QgsRelationEditorWidget
 */
class GUI_EXPORT QgsRelationEditorWidget : public QgsCollapsibleGroupBox
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsRelationEditorWidget *>( sipCpp ) )
      sipType = sipType_QgsRelationEditorWidget;
    else
      sipType = NULL;
    SIP_END
#endif



    Q_OBJECT
    Q_PROPERTY( QgsDualView::ViewMode viewMode READ viewMode WRITE setViewMode )
    Q_PROPERTY( bool showLabel READ showLabel WRITE setShowLabel )

  public:

    /**
     * \param parent parent widget
     */
    QgsRelationEditorWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! Define the view mode for the dual view
    void setViewMode( QgsDualView::ViewMode mode );

    //! Get the view mode for the dual view
    QgsDualView::ViewMode viewMode() {return mViewMode;}

    void setRelationFeature( const QgsRelation &relation, const QgsFeature &feature );

    /**
     * Set the relation(s) for this widget
     * If only one relation is set, it will act as a simple 1:N relation widget
     * If both relations are set, it will act as an N:M relation widget
     * inserting and deleting entries on the intermediate table as required.
     *
     * \param relation    Relation referencing the edited table
     * \param nmrelation  Optional reference from the referencing table to a 3rd N:M table
     */
    void setRelations( const QgsRelation &relation, const QgsRelation &nmrelation );

    void setFeature( const QgsFeature &feature );

    void setEditorContext( const QgsAttributeEditorContext &context );

    /**
     * The feature selection manager is responsible for the selected features
     * which are currently being edited.
     */
    QgsIFeatureSelectionManager *featureSelectionManager();

    /**
     * Defines if a title label should be shown for this widget.
     *
     * \since QGIS 2.18
     */
    bool showLabel() const;

    /**
     * Defines if a title label should be shown for this widget.
     *
     * \since QGIS 2.18
     */
    void setShowLabel( bool showLabel );

    /**
     * Determines if the "link feature" button should be shown
     *
     * \since QGIS 2.18
     */
    bool showLinkButton() const;

    /**
     * Determines if the "link feature" button should be shown
     *
     * \since QGIS 2.18
     */
    void setShowLinkButton( bool showLinkButton );

    /**
     * Determines if the "unlink feature" button should be shown
     *
     * \since QGIS 2.18
     */
    bool showUnlinkButton() const;

    /**
     * Determines if the "unlink feature" button should be shown
     *
     * \since QGIS 2.18
     */
    void setShowUnlinkButton( bool showUnlinkButton );

  private slots:
    void setViewMode( int mode ) {setViewMode( static_cast<QgsDualView::ViewMode>( mode ) );}
    void updateButtons();

    void addFeature();
    void linkFeature();
    void deleteFeature();
    void unlinkFeature();
    void saveEdits();
    void toggleEditing( bool state );
    void onCollapsedStateChanged( bool collapsed );

  private:
    void updateUi();

    QgsDualView *mDualView = nullptr;
    QgsDualView::ViewMode mViewMode = QgsDualView::AttributeEditor;
    QgsGenericFeatureSelectionManager *mFeatureSelectionMgr = nullptr;
    QgsAttributeEditorContext mEditorContext;
    QgsRelation mRelation;
    QgsRelation mNmRelation;
    QgsFeature mFeature;

    QToolButton *mToggleEditingButton = nullptr;
    QToolButton *mSaveEditsButton = nullptr;
    QToolButton *mAddFeatureButton = nullptr;
    QToolButton *mDeleteFeatureButton = nullptr;
    QToolButton *mLinkFeatureButton = nullptr;
    QToolButton *mUnlinkFeatureButton = nullptr;
    QToolButton *mFormViewButton = nullptr;
    QToolButton *mTableViewButton = nullptr;
    QGridLayout *mRelationLayout = nullptr;
    QButtonGroup *mViewModeButtonGroup = nullptr;

    bool mShowLabel = true;
    bool mVisible = false;
};

#endif // QGSRELATIONEDITOR_H
