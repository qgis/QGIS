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
#include "qgis_app.h"


//! A dialog to enter data defined label attributes
class APP_EXPORT QgsLabelPropertyDialog: public QDialog, private Ui::QgsLabelPropertyDialogBase
{
    Q_OBJECT
  public:
    QgsLabelPropertyDialog( const QString &layerId,
                            const QString &providerId,
                            QgsFeatureId featureId,
                            const QFont &labelFont,
                            const QString &labelText,
                            bool isPinned,
                            const QgsPalLayerSettings &layerSettings,
                            QWidget *parent = nullptr,
                            Qt::WindowFlags f = nullptr );

    //! Returns properties changed by the user
    const QgsAttributeMap &changedProperties() const { return mChangedProperties; }

    void setMapCanvas( QgsMapCanvas *canvas );

  signals:

    /**
     * Emitted when dialog settings are applied
     * \since QGIS 2.9
     */
    void applied();

  private slots:
    void buttonBox_clicked( QAbstractButton *button );
    void mShowLabelChkbx_toggled( bool chkd );
    void mAlwaysShowChkbx_toggled( bool chkd );
    void labelAllPartsToggled( bool checked );
    void showCalloutToggled( bool chkd );
    void minScaleChanged( double scale );
    void maxScaleChanged( double scale );
    void mLabelDistanceSpinBox_valueChanged( double d );
    void mXCoordSpinBox_valueChanged( double d );
    void mYCoordSpinBox_valueChanged( double d );
    void mFontFamilyCmbBx_currentFontChanged( const QFont &f );
    void mFontStyleCmbBx_currentIndexChanged( const QString &text );
    void mFontUnderlineBtn_toggled( bool ckd );
    void mFontStrikethroughBtn_toggled( bool ckd );
    void mFontBoldBtn_toggled( bool ckd );
    void mFontItalicBtn_toggled( bool ckd );
    void mFontSizeSpinBox_valueChanged( double d );
    void mBufferSizeSpinBox_valueChanged( double d );
    void mRotationSpinBox_valueChanged( double d );
    void mFontColorButton_colorChanged( const QColor &color );
    void mBufferColorButton_colorChanged( const QColor &color );
    void mMultiLineAlignComboBox_currentIndexChanged( int index );
    void mHaliComboBox_currentIndexChanged( int index );
    void mValiComboBox_currentIndexChanged( int index );
    void mLabelTextLineEdit_textChanged( const QString &text );

  private:
    //! Sets activation / values to the gui elements depending on the label settings and feature values
    void init( const QString &layerId, const QString &providerId, QgsFeatureId featureId, const QString &labelText );
    void disableGuiElements();
    //! Block / unblock all input element signals
    void blockElementSignals( bool block );

    void setDataDefinedValues( QgsVectorLayer *vlayer );
    void enableDataDefinedWidgets( QgsVectorLayer *vlayer );

    //! Updates font when family or style is updated
    void updateFont( const QFont &font, bool block = true );

    //! Updates combobox with named styles of font
    void populateFontStyleComboBox();

    void fillMultiLineAlignComboBox();
    void fillHaliComboBox();
    void fillValiComboBox();

    //! Insert changed value into mChangedProperties
    void insertChangedValue( QgsPalLayerSettings::Property p, const QVariant &value );

    void enableWidgetsForPinnedLabels();

    QgsAttributeMap mChangedProperties;
    QgsPropertyCollection mDataDefinedProperties;
    QFont mLabelFont;

    QFontDatabase mFontDB;

    //! Label field for the current layer (or -1 if none)
    int mCurLabelField = -1;

    //! Current feature
    QgsFeature mCurLabelFeat;

    bool mIsPinned = false;
    bool mCanSetHAlignment = false;
    bool mCanSetVAlignment = false;
};

#endif // QGSLAYERPROPERTYDIALOG_H
