/***************************************************************************
    qgisexpressionselectiondialog.h
     --------------------------------------
    Date                 : 24.1.2013
    Copyright            : (C) 2013 by Matthias kuhn
    Email                : matthias dot kuhn at gmx dot ch
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

#include <QDialog>
#include "qgsdistancearea.h"
#include "ui_qgsexpressionselectiondialogbase.h"

/**
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
     * @param layer     The layer on which the selection is to be performed.
     * @param startText A default expression text to be applied (Defaults to empty)
     * @param parent parent object (owner)
     */
    QgsExpressionSelectionDialog( QgsVectorLayer* layer, QString startText = QString(), QWidget* parent = 0 );

    /**
     * The builder widget that is used by the dialog
     * @return The builder widget that is used by the dialog
     */
    QgsExpressionBuilderWidget* expressionBuilder();

    /**
     * Sets the current expression text
     * @param text the expression text to set
     */
    void setExpressionText( const QString& text );

    /**
     * Returns the current expression text
     * @return The expression text
     */
    QString expressionText();

    /**
     *Sets geometry calculator used in distance/area calculations.
     */
    void setGeomCalculator( const QgsDistanceArea & da );

  public slots:
    void on_mActionSelect_triggered();
    void on_mActionAddToSelection_triggered();
    void on_mActionRemoveFromSelection_triggered();
    void on_mActionSelectInstersect_triggered();
    void on_mPbnClose_clicked();

  protected:
    /**
     * Implementation for closeEvent
     * Saves the window geometry
     * @param closeEvent Event object. Unused.
     */
    virtual void closeEvent( QCloseEvent *closeEvent );

    /**
     * Implementation for done (default behavior when pressing esc)
     * Calls close, so the window geometry gets saved and the object deleted.
     * @param r   Result value. Unused.
     */
    virtual void done( int r );

  private:
    void saveRecent();
    QgsVectorLayer* mLayer;
};

#endif
