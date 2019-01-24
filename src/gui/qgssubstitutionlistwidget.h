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
#include "qgis_sip.h"
#include "qgspanelwidget.h"
#include "ui_qgssubstitutionlistwidgetbase.h"
#include "qgsstringutils.h"
#include "qgis_gui.h"

/**
 * \class QgsSubstitutionListWidget
 * \ingroup gui
 * A widget which allows users to specify a list of substitutions to apply to a string, with
 * options for exporting and importing substitution lists.
 * \see QgsSubstitutionListDialog
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsSubstitutionListWidget : public QgsPanelWidget, private Ui::QgsSubstitutionListWidgetBase
{
    Q_OBJECT
    Q_PROPERTY( QgsStringReplacementCollection substitutions READ substitutions WRITE setSubstitutions NOTIFY substitutionsChanged )

  public:

    /**
     * Constructor for QgsSubstitutionListWidget.
     * \param parent parent widget
     */
    QgsSubstitutionListWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the list of substitutions to show in the widget.
     * \param substitutions substitution list
     * \see substitutions()
     */
    void setSubstitutions( const QgsStringReplacementCollection &substitutions );

    /**
     * Returns the list of substitutions currently defined by the widget.
     * \see setSubstitutions()
     */
    QgsStringReplacementCollection substitutions() const;

  signals:

    //! Emitted when the substitution definitions change.
    void substitutionsChanged( const QgsStringReplacementCollection &substitutions );

  private slots:

    void mButtonAdd_clicked();
    void mButtonRemove_clicked();
    void tableChanged();
    void mButtonExport_clicked();
    void mButtonImport_clicked();

  private:

    void addSubstitution( const QgsStringReplacement &substitution );

};

/**
 * \class QgsSubstitutionListDialog
 * \ingroup gui
 * A dialog which allows users to specify a list of substitutions to apply to a string, with
 * options for exporting and importing substitution lists.
 * \see QgsSubstitutionListWidget
 * \since QGIS 3.0
*/
class GUI_EXPORT QgsSubstitutionListDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY( QgsStringReplacementCollection substitutions READ substitutions WRITE setSubstitutions )

  public:

    /**
     * Constructor for QgsSubstitutionListDialog.
     * \param parent parent widget
     */
    QgsSubstitutionListDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the list of substitutions to show in the dialog.
     * \param substitutions substitution list
     * \see substitutions()
     */
    void setSubstitutions( const QgsStringReplacementCollection &substitutions );

    /**
     * Returns the list of substitutions currently defined by the dialog.
     * \see setSubstitutions()
     */
    QgsStringReplacementCollection substitutions() const;


  private:

    QgsSubstitutionListWidget *mWidget = nullptr;

};

#endif // QGSSUBSTITUTIONLISTWIDGET_H
