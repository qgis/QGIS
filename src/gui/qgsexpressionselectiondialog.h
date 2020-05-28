/***************************************************************************
    qgisexpressionselectiondialog.h
     --------------------------------------
    Date                 : 24.1.2013
    Copyright            : (C) 2013 by Matthias kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXPRESSIONSELECTIONDIALOG_H
#define QGSEXPRESSIONSELECTIONDIALOG_H

#include "ui_qgsexpressionselectiondialogbase.h"
#include "qgis_sip.h"
#include "qgshelp.h"

#include <QDialog>
#include "qgis_gui.h"

class QgsMapCanvas;
class QgsMessageBar;

/**
 * \ingroup gui
 * This class offers a dialog to change feature selections.
 * To do so, a QgsExpressionBuilderWidget is shown in a dialog.
 * It offers the possibilities to create a new selection, add to the current selection
 * remove from the current selection or select within the current selection.
 */
class GUI_EXPORT QgsExpressionSelectionDialog : public QDialog, private Ui::QgsExpressionSelectionDialogBase
{
    Q_OBJECT

  public:

    /**
     * Creates a new selection dialog.
     * \param layer     The layer on which the selection is to be performed.
     * \param startText A default expression text to be applied (Defaults to empty)
     * \param parent parent object (owner)
     */
    QgsExpressionSelectionDialog( QgsVectorLayer *layer, const QString &startText = QString(), QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * The builder widget that is used by the dialog
     * \returns The builder widget that is used by the dialog
     */
    QgsExpressionBuilderWidget *expressionBuilder();

    /**
     * Sets the current expression text
     * \param text the expression text to set
     */
    void setExpressionText( const QString &text );

    /**
     * Returns the current expression text
     * \returns The expression text
     */
    QString expressionText();

    /**
     *Sets geometry calculator used in distance/area calculations.
     */
    void setGeomCalculator( const QgsDistanceArea &da );

    /**
     * Sets the message bar to display feedback from the dialog. This is used when zooming to
     * features to display the count of selected features.
     * \param messageBar target message bar
     * \since QGIS 3.0
     */
    void setMessageBar( QgsMessageBar *messageBar );

    /**
     * Sets a map canvas associated with the dialog.
     * \since QGIS 3.0
     */
    void setMapCanvas( QgsMapCanvas *canvas );

  private slots:
    void mActionSelect_triggered();
    void mActionAddToSelection_triggered();
    void mActionRemoveFromSelection_triggered();
    void mActionSelectIntersect_triggered();
    void mButtonZoomToFeatures_clicked();
    void mPbnClose_clicked();
    void showHelp();

  protected:

    /**
     * Implementation for closeEvent
     * Saves the window geometry
     * \param closeEvent Event object. Unused.
     */
    void closeEvent( QCloseEvent *closeEvent ) override;

    /**
     * Implementation for done (default behavior when pressing esc)
     * Calls close, so the window geometry gets saved and the object deleted.
     * \param r   Result value. Unused.
     */
    void done( int r ) override;

  private:
    void saveRecent();
    void pushSelectedFeaturesMessage();
    QgsVectorLayer *mLayer = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;

};

#endif
