/***************************************************************************
  qgsfeaturepickerwidget.h - QgsFeaturePickerWidget
 ---------------------
 begin                : 03.04.2020
 copyright            : (C) 2020 by Denis Rouzaud
 email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFEATURECHOOSER_H
#define QGSFEATURECHOOSER_H

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QCompleter>

#include "qgsfeature.h"
#include "qgis_gui.h"

class QToolButton;

class QgsVectorLayer;
class QgsFeaturePickerModel;
class QgsAnimatedIcon;
class QgsFilterLineEdit;


/**
 * \ingroup gui
 * \brief This offers a combobox with autocompleter that allows selecting features from a layer.
 *
 * It will show up to 100 entries at a time. The entries can be chosen based on the displayExpression
 * and whenever text is typed into the combobox, the completer and popup will adjust to features matching the typed text.
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsFeaturePickerWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY( QgsVectorLayer *layer READ layer WRITE setLayer NOTIFY layerChanged )
    Q_PROPERTY( QString displayExpression READ displayExpression WRITE setDisplayExpression NOTIFY displayExpressionChanged )
    Q_PROPERTY( QString filterExpression READ filterExpression WRITE setFilterExpression NOTIFY filterExpressionChanged )
    Q_PROPERTY( bool allowNull READ allowNull WRITE setAllowNull NOTIFY allowNullChanged )
    Q_PROPERTY( bool fetchGeometry READ fetchGeometry WRITE setFetchGeometry NOTIFY fetchGeometryChanged )
    Q_PROPERTY( int fetchLimit READ fetchLimit WRITE setFetchLimit NOTIFY fetchLimitChanged )
    Q_PROPERTY( bool showBrowserButtons READ showBrowserButtons WRITE setShowBrowserButtons NOTIFY showBrowserButtonsChanged )

  public:
    /**
     * Create a new QgsFeaturePickerWidget, optionally specifying a \a parent.
     */
    QgsFeaturePickerWidget( QWidget *parent = nullptr );

    /**
     * The layer from which features should be listed.
     */
    QgsVectorLayer *layer() const;

    /**
     * The layer from which features should be listed.
     */
    void setLayer( QgsVectorLayer *layer );

    /**
     * Sets the current index by using the given feature
     */
    void setFeature( QgsFeatureId featureId );

    /**
     * Returns the current feature
     */
    QgsFeature feature() const;

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
     */
    int nullIndex() const;

    /**
     * An additional expression to further restrict the available features.
     * This can be used to integrate additional spatial or other constraints.
     */
    void setFilterExpression( const QString &filterExpression );

    /**
     * Determines if a NULL value should be available in the list.
     */
    bool allowNull() const;

    /**
     * Determines if a NULL value should be available in the list.
     */
    void setAllowNull( bool allowNull );

    /**
     * Returns if the geometry is fetched
     */
    bool fetchGeometry() const;

    /**
     * Defines if the geometry will be fetched
     */
    void setFetchGeometry( bool fetchGeometry );

    /**
     * Returns the feature request fetch limit
     */
    int fetchLimit() const;

    /**
     * Defines the feature request fetch limit
     * If set to 0, no limit is applied when fetching
     */
    void setFetchLimit( int fetchLimit );

    /**
     * Returns if the browsing buttons are shown
     */
    bool showBrowserButtons() const;

    /**
     * Defines if the browsing buttons are shown
     * \note Buttons are browsing the currently fetched features. If a limit is set, there might be more features to be fetched from the layer.
     */
    void setShowBrowserButtons( bool showBrowserButtons );


    /**
     * The index of the currently selected item.
     */
    QModelIndex currentModelIndex() const;

    void focusOutEvent( QFocusEvent *event ) override;

    void keyPressEvent( QKeyEvent *event ) override;

  signals:

    /**
     * The underlying model has been updated.
     */
    void modelUpdated();

    /**
     * The layer from which features should be listed.
     */
    void layerChanged();

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

    //! Sends the feature as soon as it is chosen
    void featureChanged( const QgsFeature &feature );

    /**
     * Determines if a NULL value should be available in the list.
     */
    void allowNullChanged();

    /**
     * Emitted when the fetching of the geometry changes
     */
    void fetchGeometryChanged();

    /**
     * Emitted when the fetching limit for the feature request changes
     */
    void fetchLimitChanged();

    //! Emitted when showing the browser buttons changes
    void showBrowserButtonsChanged();

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
    void browseFeatures( int direction );

  private:
    QComboBox *mComboBox;
    QToolButton *mPreviousButton;
    QToolButton *mNextButton;
    QgsFeaturePickerModel *mModel = nullptr;
    QCompleter *mCompleter = nullptr;
    QgsFilterLineEdit *mLineEdit;
    bool mPopupRequested = false;
    bool mIsCurrentlyEdited = false;
    bool mShowBrowserButtons = false;
};


#endif // QGSFEATURECHOOSER_H
