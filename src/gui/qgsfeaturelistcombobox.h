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

/**
 * \ingroup gui
 * This offers a combobox with autocompleter that allows selecting features from a layer.
 *
 * It will show up to 100 entries at a time. The entries can be chosen based on the displayExpression
 * and whenever text is typed into the combobox, the completer and popup will adjust to features matching the typed text.
 *
 * \since QGIS 3.0
 */
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

    /**
     * Create a new QgsFeatureListComboBox, optionally specifying a \a parent.
     */
    QgsFeatureListComboBox( QWidget *parent = nullptr );

    /**
     * The layer from which features should be listed.
     */
    QgsVectorLayer *sourceLayer() const;

    /**
     * The layer from which features should be listed.
     */
    void setSourceLayer( QgsVectorLayer *sourceLayer );

    /**
     * The display expression will be used to display features as well as
     * the the value to match the typed text against.
     */
    QString displayExpression() const;

    /**
     * The display expression will be used to display features as well as
     * the the value to match the typed text against.
     */
    void setDisplayExpression( const QString &displayExpression );

    /**
     * An additional expression to further restrict the available features.
     * This can be used to integrate additional spatial or other constraints.
     */
    QString filterExpression() const;

    /**
     * An additional expression to further restrict the available features.
     * This can be used to integrate additional spatial or other constraints.
     *
     * TODO!
     */
    void setFilterExpression( const QString &filterExpression );

    /**
     * The identifier value of the currently selected feature. A value from the
     * identifierField.
     */
    QVariant identifierValue() const;

    /**
     * The identifier value of the currently selected feature. A value from the
     * identifierField.
     */
    void setIdentifierValue( const QVariant &identifierValue );

    /**
     * Shorthand for getting a feature request to query the currently selected
     * feature.
     */
    QgsFeatureRequest currentFeatureRequest() const;

    /**
     * Determines if a NULL value should be available in the list.
     */
    bool allowNull() const;

    /**
     * Determines if a NULL value should be available in the list.
     */
    void setAllowNull( bool allowNull );

    /**
     * Field name that will be used to uniquely identify the current feature.
     * Normally the primary key of the layer.
     */
    QString identifierField() const;

    /**
     * Field name that will be used to uniquely identify the current feature.
     * Normally the primary key of the layer.
     */
    void setIdentifierField( const QString &identifierField );

    /**
     * The index of the currently selected item.
     */
    QModelIndex currentModelIndex() const;

    virtual void focusOutEvent( QFocusEvent *event ) override;

    virtual void keyPressEvent( QKeyEvent *event ) override;

  signals:

    /**
     * The layer from which features should be listed.
     */
    void sourceLayerChanged();

    /**
     * The display expression will be used to display features as well as
     * the the value to match the typed text against.
     */
    void displayExpressionChanged();

    /**
     * An additional expression to further restrict the available features.
     * This can be used to integrate additional spatial or other constraints.
     */
    void filterExpressionChanged();

    /**
     * The identifier value of the currently selected feature. A value from the
     * identifierField.
     */
    void identifierValueChanged();

    /**
     * Field name that will be used to uniquely identify the current feature.
     * Normally the primary key of the layer.
     */
    void identifierFieldChanged();

    /**
     * Determines if a NULL value should be available in the list.
     */
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
    void onDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>() );

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

    QgsFeatureFilterModel *mModel = nullptr;
    QCompleter *mCompleter = nullptr;
    QString mDisplayExpression;
    QgsFilterLineEdit *mLineEdit;
    bool mPopupRequested = false;
    bool mIsCurrentlyEdited = false;
    LineEditState mLineEditState;
};

#endif // QGSFIELDLISTCOMBOBOX_H
