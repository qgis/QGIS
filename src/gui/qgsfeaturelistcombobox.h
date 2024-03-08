/***************************************************************************
  qgsfeaturelistcombobox.h - QgsFeatureListComboBox
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
#include "qgis_gui.h"

class QgsVectorLayer;
class QgsFeatureFilterModel;
class QgsAnimatedIcon;
class QgsFilterLineEdit;
class QgsFeatureRequest;

/**
 * \ingroup gui
 * \brief This offers a combobox with autocompleter that allows selecting features from a layer.
 *
 * It will show up to 100 entries at a time. The entries can be chosen based on the displayExpression
 * and whenever text is typed into the combobox, the completer and popup will adjust to features matching the typed text.
 *
 */
class GUI_EXPORT QgsFeatureListComboBox : public QComboBox
{
    Q_OBJECT

    Q_PROPERTY( QgsVectorLayer *sourceLayer READ sourceLayer WRITE setSourceLayer NOTIFY sourceLayerChanged )
    Q_PROPERTY( QString displayExpression READ displayExpression WRITE setDisplayExpression NOTIFY displayExpressionChanged )
    Q_PROPERTY( QString filterExpression READ filterExpression WRITE setFilterExpression NOTIFY filterExpressionChanged )
    Q_PROPERTY( QVariant identifierValue READ identifierValue WRITE setIdentifierValue NOTIFY identifierValueChanged )
    Q_PROPERTY( QVariantList identifierValues READ identifierValues WRITE setIdentifierValues NOTIFY identifierValueChanged )
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
     * Sets the current index by using the given feature
     * \since QGIS 3.10
     */
    void setCurrentFeature( const QgsFeature &feature );

    /**
     * The display expression will be used to display features as well as
     * the value to match the typed text against.
     */
    QString displayExpression() const;

    /**
     * The display expression will be used to display features as well as
     * the value to match the typed text against.
     */
    void setDisplayExpression( const QString &displayExpression );

    /**
     * An additional expression to further restrict the available features.
     * This can be used to integrate additional spatial or other constraints.
     */
    QString filterExpression() const;

    /**
     * Returns the current index of the NULL value, or -1 if NULL values are
     * not allowed.
     *
     * \since QGIS 3.2
     */
    int nullIndex() const;

    /**
     * An additional expression to further restrict the available features.
     * This can be used to integrate additional spatial or other constraints.
     */
    void setFilterExpression( const QString &filterExpression );

    /**
     * The identifier value of the currently selected feature. A value from the
     * identifierField.
     * \deprecated since QGIS 3.10
     */
    Q_DECL_DEPRECATED QVariant identifierValue() const SIP_DEPRECATED;

    /**
     * The identifier values of the currently selected feature. A value from the
     * identifierField.
     * \since QGIS 3.10
     */
    QVariantList identifierValues() const;


    /**
     * The identifier value of the currently selected feature. A value from the
     * identifierField.
     * \deprecated since QGIS 3.10 use setIdentifierValues
     */
    Q_DECL_DEPRECATED void setIdentifierValue( const QVariant &identifierValue ) SIP_DEPRECATED;

    /**
     * The identifier values of the currently selected feature. A value from the
     * identifierFields.
     * \since QGIS 3.10
     */
    void setIdentifierValues( const QVariantList &identifierValues );

    /**
     * Sets the identifier values of the currently selected feature to NULL value(s).
     * \since QGIS 3.10
     */
    void setIdentifierValuesToNull();

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
     * Returns the feature request fetch limit
     * \since QGIS 3.32
     */
    int fetchLimit() const;

    /**
     * Defines the feature request fetch limit
     * If set to 0, no limit is applied when fetching
     * \since QGIS 3.32
     */
    void setFetchLimit( int fetchLimit );

    /**
     * Field name that will be used to uniquely identify the current feature.
     * Normally the primary key of the layer.
     * \deprecated since QGIS 3.10
     */
    Q_DECL_DEPRECATED QString identifierField() const SIP_DEPRECATED;

    /**
     * Field name that will be used to uniquely identify the current feature.
     * Normally the primary key of the layer.
     * \since QGIS 3.10
     */
    QStringList identifierFields() const;

    /**
     * Field name that will be used to uniquely identify the current feature.
     * Normally the primary key of the layer.
     * \deprecated since QGIS 3.10
     */
    Q_DECL_DEPRECATED void setIdentifierField( const QString &identifierField ) SIP_DEPRECATED;

    /**
     * Field name that will be used to uniquely identify the current feature.
     * Normally the primary key of the layer.
     * \since QGIS 3.10
     */
    void setIdentifierFields( const QStringList &identifierFields );

    /**
     * The index of the currently selected item.
     */
    QModelIndex currentModelIndex() const;

    void focusOutEvent( QFocusEvent *event ) override;

    void keyPressEvent( QKeyEvent *event ) override;

  signals:

    /**
     * The underlying model has been updated.
     *
     * \since QGIS 3.2
     */
    void modelUpdated();

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

    /**
     * Emitted when the current feature changes
     * \since QGIS 3.16.5
     */
    void currentFeatureChanged();

  private slots:
    void onCurrentTextChanged( const QString &text );
    void onFilterLineEditCleared();
    void onFilterUpdateCompleted();
    void onLoadingChanged();
    void onItemSelected( const QModelIndex &index );
    void onCurrentIndexChanged( int i );
    void onActivated( QModelIndex index );
    void storeLineEditState();
    void restoreLineEditState();
    void onDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>() );

  private:
    QgsFeatureFilterModel *mModel = nullptr;
    QCompleter *mCompleter = nullptr;
    QgsFilterLineEdit *mLineEdit;
    bool mPopupRequested = false;
    bool mIsCurrentlyEdited = false;

    friend class TestQgsFeatureListComboBox;
};



#endif // QGSFIELDLISTCOMBOBOX_H
