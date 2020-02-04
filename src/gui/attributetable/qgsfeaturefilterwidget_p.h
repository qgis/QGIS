/***************************************************************************
    qgsfeaturefilterwidget.h
     --------------------------------------
    Date                 : 20.9.2019
    Copyright            : (C) 2019 Julien Cabieces
    Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFEATUREFILTERWIDGET_P_H
#define QGSFEATUREFILTERWIDGET_P_H

#define SIP_NO_FILE

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "ui_qgsfeaturefilterwidget.h"

#include "qgsattributeform.h"

#include "qgis_gui.h"

class QgsVectorLayer;
class QgsAttributeEditorContext;
class QgsSearchWidgetWrapper;
class QgsDualView;
class QgsMessageBar;

/**
 * \ingroup gui
 * \class QgsFeatureFilterWidget
 */
class GUI_EXPORT QgsFeatureFilterWidget : public QWidget, private Ui::QgsFeatureFilterWidget
{
    Q_OBJECT

  public:

    //! Constructor for QgsFeatureFilterWidget
    explicit QgsFeatureFilterWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    void init( QgsVectorLayer *layer, const QgsAttributeEditorContext &context, QgsDualView *mainView,
               QgsMessageBar *messageBar, int messagebarTimeout );

    /**
     * Sets the filter expression to filter visible features
     * \param filterString filter query string. QgsExpression compatible.
     */
    void setFilterExpression( const QString &filterString,
                              QgsAttributeForm::FilterType type = QgsAttributeForm::ReplaceFilter,
                              bool alwaysShowFilter = false );

  public slots:
    void filterShowAll();
    void filterSelected();
    void filterVisible();


  private slots:

    //! Initialize column box
    void columnBoxInit();

    //! Initialize storedexpression box e.g after adding/deleting/edditing stored expression
    void storedFilterExpressionBoxInit();
    //! Functionalities of store expression button changes regarding the status of it
    void storeExpressionButtonInit();

    void filterExpressionBuilder();
    void filterEdited();
    void filterQueryChanged( const QString &query );
    void filterQueryAccepted();

    /**
     * Starts timer with timeout 300 ms.
     */
    void onFilterQueryTextChanged( const QString &value );

    /**
    * Handles the expression (save or delete) when the bookmark button for stored
    * filter expressions is triggered.
    */
    void handleStoreFilterExpression();

    /**
    * Opens dialog and give the possibility to save the expression with a name.
    */
    void saveAsStoredFilterExpression();

    /**
    * Opens dialog and give the possibility to edit the name and the expression
    * of the stored expression.
    */
    void editStoredFilterExpression();

    /**
     * Updates the bookmark button and it's actions regarding the stored filter
     * expressions according to the values
     */
    void updateCurrentStoredFilterExpression( );

    void filterColumnChanged( QAction *filterAction );

  private:

    /* replace the search widget with a new one */
    void replaceSearchWidget( QWidget *oldw, QWidget *neww );

    QMenu *mFilterColumnsMenu = nullptr;
    QMenu *mStoredFilterExpressionMenu = nullptr;
    QTimer mFilterQueryTimer;
    QgsSearchWidgetWrapper *mCurrentSearchWidgetWrapper = nullptr;
    QgsDualView *mMainView = nullptr;
    QgsVectorLayer *mLayer = nullptr;
    QgsAttributeEditorContext mEditorContext;
    QgsMessageBar *mMessageBar = nullptr;
    int mMessageBarTimeout = 0;
};

#endif // QGSFEATUREFILTERWIDGET_P_H

/// @endcond
