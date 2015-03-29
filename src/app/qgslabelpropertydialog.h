/***************************************************************************
                          qgslabelpropertydialog.h
                          ------------------------
    begin                : 2010-11-12
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLABELPROPERTYDIALOG_H
#define QGSLABELPROPERTYDIALOG_H

#include "ui_qgslabelpropertydialogbase.h"
#include "qgsfeature.h"
#include "qgspallabeling.h"
#include <QDialog>


/**A dialog to enter data defined label attributes*/
class APP_EXPORT QgsLabelPropertyDialog: public QDialog, private Ui::QgsLabelPropertyDialogBase
{
    Q_OBJECT
  public:
    QgsLabelPropertyDialog( const QString& layerId, int featureId, const QFont& labelFont, const QString& labelText, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsLabelPropertyDialog();

    /**Returns properties changed by the user*/
    const QgsAttributeMap& changedProperties() const { return mChangedProperties; }

  signals:

    /** Emitted when dialog settings are applied
     * @note added in QGIS 2.9
     */
    void applied();

  private slots:
    void on_buttonBox_clicked( QAbstractButton * button );
    void on_mShowLabelChkbx_toggled( bool chkd );
    void on_mAlwaysShowChkbx_toggled( bool chkd );
    void on_mMinScaleSpinBox_valueChanged( int i );
    void on_mMaxScaleSpinBox_valueChanged( int i );
    void on_mLabelDistanceSpinBox_valueChanged( double d );
    void on_mXCoordSpinBox_valueChanged( double d );
    void on_mYCoordSpinBox_valueChanged( double d );
    void on_mFontFamilyCmbBx_currentFontChanged( const QFont& f );
    void on_mFontStyleCmbBx_currentIndexChanged( const QString & text );
    void on_mFontUnderlineBtn_toggled( bool ckd );
    void on_mFontStrikethroughBtn_toggled( bool ckd );
    void on_mFontBoldBtn_toggled( bool ckd );
    void on_mFontItalicBtn_toggled( bool ckd );
    void on_mFontSizeSpinBox_valueChanged( double d );
    void on_mBufferSizeSpinBox_valueChanged( double d );
    void on_mRotationSpinBox_valueChanged( double d );
    void on_mFontColorButton_colorChanged( const QColor &color );
    void on_mBufferColorButton_colorChanged( const QColor &color );
    void on_mHaliComboBox_currentIndexChanged( const QString& text );
    void on_mValiComboBox_currentIndexChanged( const QString& text );
    void on_mLabelTextLineEdit_textChanged( const QString& text );

  private:
    /**Sets activation / values to the gui elements depending on the label settings and feature values*/
    void init( const QString& layerId, int featureId, const QString& labelText );
    void disableGuiElements();
    /**Block / unblock all input element signals*/
    void blockElementSignals( bool block );

    void setDataDefinedValues( QgsPalLayerSettings layerSettings, QgsVectorLayer* vlayer );
    void enableDataDefinedWidgets( QgsVectorLayer* vlayer );

    /** Updates font when family or style is updated */
    void updateFont( const QFont& font, bool block = true );

    /** Updates combobox with named styles of font */
    void populateFontStyleComboBox();

    void fillHaliComboBox();
    void fillValiComboBox();

    /**Insert changed value into mChangedProperties*/
    void insertChangedValue( QgsPalLayerSettings::DataDefinedProperties p, QVariant value );

    QgsAttributeMap mChangedProperties;
    QMap< QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* > mDataDefinedProperties;
    QFont mLabelFont;

    QFontDatabase mFontDB;

    /**Label field for the current layer (or -1 if none)*/
    int mCurLabelField;

    /** Current feature */
    QgsFeature mCurLabelFeat;
};

#endif // QGSLAYERPROPERTYDIALOG_H
