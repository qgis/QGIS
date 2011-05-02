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

class QgsMapRenderer;

/**A dialog to enter data defined label attributes*/
class QgsLabelPropertyDialog: public QDialog, private Ui::QgsLabelPropertyDialogBase
{
    Q_OBJECT
  public:
    QgsLabelPropertyDialog( const QString& layerId, int featureId, QgsMapRenderer* renderer, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsLabelPropertyDialog();

    /**Returns properties changed by the user*/
    const QgsAttributeMap& changedProperties() const { return mChangedProperties; }

  private slots:
    void on_mLabelDistanceSpinBox_valueChanged( double d );
    void on_mXCoordSpinBox_valueChanged( double d );
    void on_mYCoordSpinBox_valueChanged( double d );
    void on_mFontSizeSpinBox_valueChanged( double d );
    void on_mBufferSizeSpinBox_valueChanged( double d );
    void on_mRotationSpinBox_valueChanged( double d );
    void on_mFontPushButton_clicked();
    void on_mFontColorButton_clicked();
    void on_mBufferColorButton_clicked();
    void on_mHaliComboBox_currentIndexChanged( const QString& text );
    void on_mValiComboBox_currentIndexChanged( const QString& text );
    void on_mLabelTextLineEdit_textChanged( const QString& text );

  private:
    /**Sets activation / values to the gui elements depending on the label settings and feature values*/
    void init( const QString& layerId, int featureId );
    void disableGuiElements();
    /**Block / unblock all input element signals*/
    void blockElementSignals( bool block );

    void fillHaliComboBox();
    void fillValiComboBox();

    /**Insert changed value into mChangedProperties*/
    void insertChangedValue( QgsPalLayerSettings::DataDefinedProperties p, QVariant value );

    /**Returns true if any font related setting is contained in mDataDefinedProperties*/
    bool labelFontEditingPossible() const;

    QgsMapRenderer* mMapRenderer;

    QgsAttributeMap mChangedProperties;
    QMap< QgsPalLayerSettings::DataDefinedProperties, int > mDataDefinedProperties;
    QFont mLabelFont;

    /**Label field for the current layer (or -1 if none)*/
    int mCurrentLabelField;
};

#endif // QGSLAYERPROPERTYDIALOG_H
