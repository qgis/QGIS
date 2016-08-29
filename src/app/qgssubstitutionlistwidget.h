/***************************************************************************
    qgssubstitutionlistwidget.h
    ---------------------------
    begin                : August 2016
    copyright            : (C) 2016 Nyall Dawson
    email                : nyall dot dawson at gmail dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSUBSTITUTIONLISTWIDGET_H
#define QGSSUBSTITUTIONLISTWIDGET_H

#include <QDialog>
#include "qgspanelwidget.h"
#include "ui_qgssubstitutionlistwidgetbase.h"
#include "qgsstringutils.h"

/** \class QgsSubstitutionListWidget
 * \ingroup app
 * A widget which allows users to specify a list of substitutions to apply to a string, with
 * options for exporting and importing substitution lists.
 * \note added in QGIS 3.0
 * \see QgsSubstitutionListDialog
 */
class APP_EXPORT QgsSubstitutionListWidget : public QgsPanelWidget, private Ui::QgsSubstitutionListWidgetBase
{
    Q_OBJECT
    Q_PROPERTY( QgsStringReplacementCollection substitutions READ substitutions WRITE setSubstitutions NOTIFY substitutionsChanged )

  public:

    /** Constructor for QgsSubstitutionListWidget.
     * @param parent parent widget
     */
    QgsSubstitutionListWidget( QWidget* parent = nullptr );

    /** Sets the list of substitutions to show in the widget.
     * @param substitutions substitution list
     * @see substitutions()
     */
    void setSubstitutions( const QgsStringReplacementCollection& substitutions );

    /** Returns the list of substitutions currently defined by the widget.
     * @see setSubstitutions()
     */
    QgsStringReplacementCollection substitutions() const;

  signals:

    //! Emitted when the substitution definitions change.
    void substitutionsChanged( const QgsStringReplacementCollection& substitutions );

  private slots:

    void on_mButtonAdd_clicked();
    void on_mButtonRemove_clicked();
    void tableChanged();
    void on_mButtonExport_clicked();
    void on_mButtonImport_clicked();

  private:

    void addSubstitution( const QgsStringReplacement& substitution );

};

/** \class QgsSubstitutionListDialog
 * \ingroup app
 * A dialog which allows users to specify a list of substitutions to apply to a string, with
 * options for exporting and importing substitution lists.
 * \see QgsSubstitutionListWidget
*/
class APP_EXPORT QgsSubstitutionListDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY( QgsStringReplacementCollection substitutions READ substitutions WRITE setSubstitutions )

  public:

    /** Constructor for QgsSubstitutionListDialog.
     * @param parent parent widget
     */
    QgsSubstitutionListDialog( QWidget* parent = nullptr );

    /** Sets the list of substitutions to show in the dialog.
     * @param substitutions substitution list
     * @see substitutions()
     */
    void setSubstitutions( const QgsStringReplacementCollection& substitutions );

    /** Returns the list of substitutions currently defined by the dialog.
     * @see setSubstitutions()
     */
    QgsStringReplacementCollection substitutions() const;


  private:

    QgsSubstitutionListWidget* mWidget;

};

#endif // QGSSUBSTITUTIONLISTWIDGET_H
