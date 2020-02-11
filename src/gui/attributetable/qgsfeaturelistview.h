/***************************************************************************
     QgsAttributeListView.h
     --------------------------------------
    Date                 : Jan 2012
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

#ifndef QGSFEATURELISTVIEW_H
#define QGSFEATURELISTVIEW_H

#include <QListView>
#include "qgis_sip.h"
#include <qdebug.h>
#include "qgsactionmenu.h"

#include "qgsfeature.h" // For QgsFeatureIds
#include "qgis_gui.h"

class QgsAttributeTableFilterModel;
class QgsFeatureListModel;
class QgsFeatureSelectionModel;
class QgsAttributeTableModel;
class QgsIFeatureSelectionManager;
class QgsVectorLayer;
class QgsVectorLayerCache;
class QgsFeatureListViewDelegate;
class QRect;

/**
 * \ingroup gui
 * Shows a list of features and renders a edit button next to each feature.
 *
 * Accepts a display expression to define the way, features are rendered.
 * Uses a QgsFeatureListModel as source model.
 *
 */
class GUI_EXPORT QgsFeatureListView : public QListView
{
    Q_OBJECT

  public:

    /**
     * Creates a feature list view
     *
     * \param parent   owner
     */
    explicit QgsFeatureListView( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the layer cache
     * \returns the layer cache used as backend
     */
    QgsVectorLayerCache *layerCache();

    /**
     * Set the QgsFeatureListModel which is used to retrieve information
     *
     * \param featureListModel  The model to use
     */
    virtual void setModel( QgsFeatureListModel *featureListModel );

    /**
     * Gets the featureListModel used by this view
     *
     * \returns The model in use
     */
    QgsFeatureListModel *featureListModel() { return mModel; }

    /**
     * The display expression is an expression used to render the fields into a single string
     * which is displaied.
     *
     * \param displayExpression  The expression used to render the feature
     *
     * \see QgsExpression
     */
    bool setDisplayExpression( const QString &displayExpression );

    /**
     * Returns the expression which is currently used to render the features.
     *
     * \returns A string containing the currend display expression
     *
     * \see QgsExpression
     */
    const QString displayExpression() const;

    /**
     * Returns a detailed message about errors while parsing a QgsExpression.
     *
     * \returns A message containing information about the parser error.
     */
    QString parserErrorString();

    /**
     * Gets the currentEditSelection
     *
     * \returns A list of edited feature ids
     */
    QgsFeatureIds currentEditSelection();

    /**
     * Sets if the currently shown form has received any edit events so far.
     *
     * \param state The state
     */
    void setCurrentFeatureEdited( bool state );

    /**
     * \brief setFeatureSelectionManager
     * \param featureSelectionManager
     */
    void setFeatureSelectionManager( QgsIFeatureSelectionManager *featureSelectionManager );

  protected:
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;
    void contextMenuEvent( QContextMenuEvent *event ) override;

  signals:

    /**
     * Emitted whenever the current edit selection has been changed.
     * \param feat the feature, which will be edited.
     */
    void currentEditSelectionChanged( QgsFeature &feat );

    /**
     * Emitted whenever the current edit selection has been changed.
     * \param progress the position of the feature in the list
     * \param count the number of features in the list
     * \since QGIS 3.8
     */
    void currentEditSelectionProgressChanged( int progress, int count );

    /**
     * Emitted whenever the display expression is successfully changed
     * \param expression The expression that was applied
     */
    void displayExpressionChanged( const QString &expression );

    //! \note not available in Python bindings
    void aboutToChangeEditSelection( bool &ok ) SIP_SKIP;

    /**
     * Emitted when the context menu is created to add the specific actions to it
     * \param menu is the already created context menu
     * \param atIndex is the position of the current feature in the model
     */
    void willShowContextMenu( QgsActionMenu *menu, const QModelIndex &atIndex );

  public slots:

    /**
     * Set the feature(s) to be edited
     *
     * \param fids  A list of features to be edited
     */
    void setEditSelection( const QgsFeatureIds &fids );

    /**
     * Set the feature(s) to be edited
     *
     * \param index The selection to set
     * \param command selection update mode
     */
    void setEditSelection( const QModelIndex &index, QItemSelectionModel::SelectionFlags command );

    /**
     * Select all currently visible features
     */
    void selectAll() override;

    void repaintRequested( const QModelIndexList &indexes );
    void repaintRequested();

    /**
     * editFirstFeature will try to edit the first feature of the list
     * \since QGIS 3.8
     */
    void editFirstFeature() {editOtherFeature( First );}

    /**
     * editNextFeature will try to edit next feature of the list
     * \since QGIS 3.8
     */
    void editNextFeature() {editOtherFeature( Next );}

    /**
     * editPreviousFeature will try to edit previous feature of the list
     * \since QGIS 3.8
     */
    void editPreviousFeature() {editOtherFeature( Previous );}

    /**
     * editLastFeature will try to edit the last feature of the list
     * \since QGIS 3.8
     */
    void editLastFeature() {editOtherFeature( Last );}



  private slots:
    void editSelectionChanged( const QItemSelection &deselected, const QItemSelection &selected );

    /**
     * Make sure, there is an edit selection. If there is none, choose the first item.
     * If \a inSelection is set to TRUE, the edit selection is done in selected entries if
     * there is a selected entry visible.
     *
     */
    void ensureEditSelection( bool inSelection = false );

  private:
    void selectRow( const QModelIndex &index, bool anchor );

    enum PositionInList
    {
      First,
      Next,
      Previous,
      Last
    };

    void editOtherFeature( PositionInList positionInList );


    QgsFeatureListModel *mModel = nullptr;
    QItemSelectionModel *mCurrentEditSelectionModel = nullptr;
    QgsFeatureSelectionModel *mFeatureSelectionModel = nullptr;
    QgsIFeatureSelectionManager *mOwnedFeatureSelectionManager = nullptr;
    QgsIFeatureSelectionManager *mFeatureSelectionManager = nullptr;
    QgsFeatureListViewDelegate *mItemDelegate = nullptr;
    bool mEditSelectionDrag = false; // Is set to true when the user initiated a left button click over an edit button and still keeps pressing //!< TODO
    int mRowAnchor = 0;
    QItemSelectionModel::SelectionFlags mCtrlDragSelectionFlag;

    QTimer mUpdateEditSelectionTimer;

    friend class QgsDualView;
};

#endif
