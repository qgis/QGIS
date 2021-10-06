/***************************************************************************
    qgsrelationreferenceconfigdlg.h
     --------------------------------------
    Date                 : 21.4.2013
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

#ifndef QGSRELATIONREFERENCECONFIGDLGBASE_H
#define QGSRELATIONREFERENCECONFIGDLGBASE_H

#include <QListWidget>

#include "ui_qgsrelationreferenceconfigdlgbase.h"
#include "qgseditorconfigwidget.h"
#include "qgis_gui.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsRelationReferenceConfigDlg
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsRelationReferenceConfigDlg : public QgsEditorConfigWidget, private Ui::QgsRelReferenceConfigDlgBase
{
    Q_OBJECT

  public:
    explicit QgsRelationReferenceConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent );
    QVariantMap config() override;
    void setConfig( const QVariantMap &config ) override;

  private:
    void loadFields();
    void addFilterField( const QString &field );
    void addFilterField( QListWidgetItem *item );
    int indexFromListWidgetItem( QListWidgetItem *item );

    QgsVectorLayer *mReferencedLayer = nullptr;

  private slots:
    void relationChanged( int idx );
    void mAddFilterButton_clicked();
    void mRemoveFilterButton_clicked();

    /**
     * Opens an expression dialog and sets its value as filter expression for the relation reference.
     */
    void mEditExpression_clicked();
};

#endif // QGSRELATIONREFERENCECONFIGDLGBASE_H
