/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfeaturelistcombobox.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsFeatureListComboBox : QComboBox
{
%Docstring
*************************************************************************
qgsfieldlistcombobox.h - QgsFieldListComboBox

---------------------
begin                : 10.3.2017
copyright            : (C) 2017 by Matthias Kuhn
email                : matthias@opengis.ch
**************************************************************************
                                                                         *
   This program is free software; you can redistribute it and/or modify  *
   it under the terms of the GNU General Public License as published by  *
   the Free Software Foundation; either version 2 of the License, or     *
   (at your option) any later version.                                   *
                                                                         *
**************************************************************************
%End

%TypeHeaderCode
#include "qgsfeaturelistcombobox.h"
%End
  public:
    QgsFeatureListComboBox( QWidget *parent = 0 );

    QgsVectorLayer *sourceLayer() const;
%Docstring
 :rtype: QgsVectorLayer
%End
    void setSourceLayer( QgsVectorLayer *sourceLayer );

    QString displayExpression() const;
%Docstring
 :rtype: str
%End
    void setDisplayExpression( const QString &displayExpression );

    QString filterExpression() const;
%Docstring
 :rtype: str
%End
    void setFilterExpression( const QString &filterExpression );

    QVariant identifierValue() const;
%Docstring
 :rtype: QVariant
%End
    void setIdentifierValue( const QVariant &identifierValue );

    QgsFeatureRequest currentFeatureRequest() const;
%Docstring
 :rtype: QgsFeatureRequest
%End

    bool allowNull() const;
%Docstring
 :rtype: bool
%End
    void setAllowNull( bool allowNull );

    QString identifierField() const;
%Docstring
 :rtype: str
%End
    void setIdentifierField( const QString &identifierField );

    QModelIndex currentModelIndex() const;
%Docstring
 :rtype: QModelIndex
%End

    virtual void focusOutEvent( QFocusEvent *event );

    virtual void keyPressEvent( QKeyEvent *event );

  signals:
    void sourceLayerChanged();
    void displayExpressionChanged();
    void filterExpressionChanged();
    void identifierValueChanged();
    void identifierFieldChanged();
    void allowNullChanged();

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfeaturelistcombobox.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
