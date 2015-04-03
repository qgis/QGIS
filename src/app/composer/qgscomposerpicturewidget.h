/***************************************************************************
                         qgscomposerpicturewidget.h
                         --------------------------
    begin                : August 13, 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERPICTUREWIDGET_H
#define QGSCOMPOSERPICTUREWIDGET_H

#include "ui_qgscomposerpicturewidgetbase.h"
#include "qgscomposeritemwidget.h"

class QgsComposerPicture;

/** \ingroup MapComposer
 * A widget for adding an image to a map composition.
 */
class QgsComposerPictureWidget: public QgsComposerItemBaseWidget, private Ui::QgsComposerPictureWidgetBase
{
    Q_OBJECT

  public:

    QgsComposerPictureWidget( QgsComposerPicture* picture );
    ~QgsComposerPictureWidget();

    /**Add the icons of the standard directories to the preview*/
    void addStandardDirectoriesToPreview();

  public slots:
    void on_mPictureBrowseButton_clicked();
    void on_mPictureLineEdit_editingFinished();
    void on_mPictureRotationSpinBox_valueChanged( double d );
    void on_mPreviewListWidget_currentItemChanged( QListWidgetItem* current, QListWidgetItem* previous );
    void on_mAddDirectoryButton_clicked();
    void on_mRemoveDirectoryButton_clicked();
    void on_mRotationFromComposerMapCheckBox_stateChanged( int state );
    void on_mComposerMapComboBox_activated( const QString & text );
    void on_mResizeModeComboBox_currentIndexChanged( int index );
    void on_mAnchorPointComboBox_currentIndexChanged( int index );

  protected:
    void showEvent( QShowEvent * event ) override;
    void resizeEvent( QResizeEvent * event ) override;

    QgsComposerObject::DataDefinedProperty ddPropertyForWidget( QgsDataDefinedButton *widget ) override;

  protected slots:
    /**Initializes data defined buttons to current atlas coverage layer*/
    void populateDataDefinedButtons();

  private slots:
    /**Sets the GUI elements to the values of mPicture*/
    void setGuiElementValues();

    /**Sets the picture rotation GUI control value*/
    void setPicRotationSpinValue( double r );

    /** Load SVG and pixel-based image previews
     * @param collapsed Whether the parent group box is collapsed */
    void loadPicturePreviews( bool collapsed );

  private:
    QgsComposerPicture* mPicture;
    /** Whether the picture selection previews have been loaded */
    bool mPreviewsLoaded;

    /**Add the icons of a directory to the preview. Returns 0 in case of success*/
    int addDirectoryToPreview( const QString& path );

    /**Tests if a file is valid svg*/
    bool testSvgFile( const QString& filename ) const;
    /**Tests if a file is a valid pixel format*/
    bool testImageFile( const QString& filename ) const;
    /**Updates the map combo box with the current composer map ids*/
    void refreshMapComboBox();
};

#endif
