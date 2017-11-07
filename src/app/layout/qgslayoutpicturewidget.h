/***************************************************************************
                         qgslayoutpicturewidget.h
                         --------------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTPICTUREWIDGET_H
#define QGSLAYOUTPICTUREWIDGET_H

#include "ui_qgslayoutpicturewidgetbase.h"
#include "qgslayoutitemwidget.h"

class QgsLayoutItemPicture;

/**
 * \ingroup app
 * A widget for configuring layout picture items.
 */
class QgsLayoutPictureWidget: public QgsLayoutItemBaseWidget, private Ui::QgsLayoutPictureWidgetBase
{
    Q_OBJECT

  public:
    explicit QgsLayoutPictureWidget( QgsLayoutItemPicture *picture );

    //! Add the icons of the standard directories to the preview
    void addStandardDirectoriesToPreview();

  public slots:
    void mPictureBrowseButton_clicked();
    void mPictureLineEdit_editingFinished();
    void mPictureRotationSpinBox_valueChanged( double d );
    void mPreviewListWidget_currentItemChanged( QListWidgetItem *current, QListWidgetItem *previous );
    void mAddDirectoryButton_clicked();
    void mRemoveDirectoryButton_clicked();
    void mRotationFromComposerMapCheckBox_stateChanged( int state );
    void mapChanged( QgsLayoutItem *item );
    void mResizeModeComboBox_currentIndexChanged( int index );
    void mAnchorPointComboBox_currentIndexChanged( int index );

  protected:

    bool setNewItem( QgsLayoutItem *item ) override;

    void resizeEvent( QResizeEvent *event ) override;

  protected slots:
    //! Initializes data defined buttons to current atlas coverage layer
    void populateDataDefinedButtons();

  private slots:
    //! Sets the GUI elements to the values of mPicture
    void setGuiElementValues();

    //! Sets the picture rotation GUI control value
    void setPicRotationSpinValue( double r );

    /**
     * Load SVG and pixel-based image previews
     * \param collapsed Whether the parent group box is collapsed */
    void loadPicturePreviews( bool collapsed );

    void mFillColorButton_colorChanged( const QColor &color );
    void mStrokeColorButton_colorChanged( const QColor &color );
    void mStrokeWidthSpinBox_valueChanged( double d );
    void mPictureRotationOffsetSpinBox_valueChanged( double d );
    void mNorthTypeComboBox_currentIndexChanged( int index );

  private:
    QgsLayoutItemPicture *mPicture = nullptr;
    QgsLayoutItemPropertiesWidget *mItemPropertiesWidget = nullptr;


    //! Whether the picture selection previews have been loaded
    bool mPreviewsLoaded = false;

    //! Add the icons of a directory to the preview. Returns 0 in case of success
    int addDirectoryToPreview( const QString &path );

    //! Tests if a file is valid svg
    bool testSvgFile( const QString &filename ) const;
    //! Tests if a file is a valid pixel format
    bool testImageFile( const QString &filename ) const;

    //! Renders an svg file to a QIcon, correctly handling any SVG parameters present in the file
    QIcon svgToIcon( const QString &filePath ) const;

    void updateSvgParamGui( bool resetValues = true );
};

#endif // QGSLAYOUTPICTUREWIDGET_H
