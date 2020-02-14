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

#include "ui_qgsattributerelationedit.h"

#include "qgseditorconfigwidget.h"
#include "qgsvectordataprovider.h"
#include "qgshelp.h"
#include "qgis_app.h"
#include <QWidget>

class APP_EXPORT QgsAttributeRelationEdit: public QWidget, private Ui::QgsAttributeRelationEdit
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
     * Getter for combo cardinality
     */
    QVariant cardinality();

    QString mRelationId;
  private:

    //Ui::QgsAttributeRelationEdit *ui;
};

#endif // QGSATTRIBUTERELATIONEDIT_H
