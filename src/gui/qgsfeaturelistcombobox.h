/***************************************************************************
  qgsfieldlistcombobox.h - QgsFieldListComboBox

 ---------------------
 begin                : 10.3.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFIELDLISTCOMBOBOX_H
#define QGSFIELDLISTCOMBOBOX_H

#include <QComboBox>

#include "qgsfeature.h"
#include "qgsfeaturerequest.h"
#include "qgis_gui.h"

class QgsVectorLayer;
class QgsFeatureFilterModel;
class QgsAnimatedIcon;
class QgsFilterLineEdit;

class GUI_EXPORT QgsFeatureListComboBox : public QComboBox
{
    Q_OBJECT

    Q_PROPERTY( QgsVectorLayer *sourceLayer READ sourceLayer WRITE setSourceLayer NOTIFY sourceLayerChanged )
    Q_PROPERTY( QString displayExpression READ displayExpression WRITE setDisplayExpression NOTIFY displayExpressionChanged )
    Q_PROPERTY( QString filterExpression READ filterExpression WRITE setFilterExpression NOTIFY filterExpressionChanged )
    Q_PROPERTY( QVariant identifierValue READ identifierValue WRITE setIdentifierValue NOTIFY identifierValueChanged )
    Q_PROPERTY( QString identifierField READ identifierField WRITE setIdentifierField NOTIFY identifierFieldChanged )
    Q_PROPERTY( bool allowNull READ allowNull WRITE setAllowNull NOTIFY allowNullChanged )

  public:
    QgsFeatureListComboBox( QWidget *parent = nullptr );

    QgsVectorLayer *sourceLayer() const;
    void setSourceLayer( QgsVectorLayer *sourceLayer );

    QString displayExpression() const;
    void setDisplayExpression( const QString &displayExpression );

    QString filterExpression() const;
    void setFilterExpression( const QString &filterExpression );

    QVariant identifierValue() const;
    void setIdentifierValue( const QVariant &identifierValue );

    QgsFeatureRequest currentFeatureRequest() const;

    bool allowNull() const;
    void setAllowNull( bool allowNull );

    QString identifierField() const;
    void setIdentifierField( const QString &identifierField );

    QModelIndex currentModelIndex() const;

    virtual void focusOutEvent( QFocusEvent *event ) override;

    virtual void keyPressEvent( QKeyEvent *event ) override;

  signals:
    void sourceLayerChanged();
    void displayExpressionChanged();
    void filterExpressionChanged();
    void identifierValueChanged();
    void identifierFieldChanged();
    void allowNullChanged();

  private slots:
    void onCurrentTextChanged( const QString &text );
    void onFilterUpdateCompleted();
    void onLoadingChanged();
    void onItemSelected( const QModelIndex &index );
    void onCurrentIndexChanged( int i );
    void onActivated( QModelIndex index );
    void storeLineEditState();
    void restoreLineEditState();

  private:
    struct LineEditState
    {
      void store( QLineEdit *lineEdit );
      void restore( QLineEdit *lineEdit ) const;

      QString text;
      int selectionStart;
      int selectionLength;
      int cursorPosition;
    };

    QgsFeatureFilterModel *mModel;
    QCompleter *mCompleter;
    QString mDisplayExpression;
    QgsFilterLineEdit *mLineEdit;
    bool mAllowNull = true;
    bool mPopupRequested = false;
    bool mIsCurrentlyEdited = false;
    LineEditState mLineEditState;
};

#endif // QGSFIELDLISTCOMBOBOX_H
