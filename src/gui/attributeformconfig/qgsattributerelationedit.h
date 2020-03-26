/***************************************************************************
    qgsattributerelationedit.h
    ---------------------
    begin                : October 2017
    copyright            : (C) 2017 by David Signer
    email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTERELATIONEDIT_H
#define QGSATTRIBUTERELATIONEDIT_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "ui_qgsattributerelationedit.h"

#include "qgseditorconfigwidget.h"
#include "qgsvectordataprovider.h"
#include "qgshelp.h"
#include "qgis_gui.h"
#include <QWidget>

class GUI_EXPORT QgsAttributeRelationEdit: public QWidget, private Ui::QgsAttributeRelationEdit
{
    Q_OBJECT

  public:
    explicit QgsAttributeRelationEdit( const QString &relationid, QWidget *parent = nullptr );

    /**
     * Setter for combo cardinality item
     */
    void setCardinalityCombo( const QString &cardinalityComboItem, const QVariant &auserData = QVariant() );

    /**
     * Setter for combo cardinality
     */
    void setCardinality( const QVariant &auserData = QVariant() );

    /**
     * Sets force suppress form popup status to \a forceSuppressFormPopup.
     *
     * This flag will override the layer and general settings regarding the automatic
     * opening of the attribute form dialog when digitizing is completed.
     *
     * \since QGIS 3.14
     */
    void setForceSuppressFormPopup( bool forceSuppressFormPopup );

    /**
     * Getter for combo cardinality
     */
    QVariant cardinality();

    /**
     * Returns force suppress form popup status.
     *
     * \returns TRUE if force suppress form popup is set.
     * \since QGIS 3.14
     */
    bool forceSuppressFormPopup();

    QString mRelationId;
  private:

    //Ui::QgsAttributeRelationEdit *ui;
};

#endif // QGSATTRIBUTERELATIONEDIT_H
