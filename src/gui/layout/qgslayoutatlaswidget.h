/***************************************************************************
                              qgslayoutatlaswidget.h
                              ---------------------------
    begin                : October 2012
    copyright            : (C) 2012 Hugo Mercier
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTATLASWIDGET_H
#define QGSLAYOUTATLASWIDGET_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "ui_qgslayoutatlaswidgetbase.h"

class QgsPrintLayout;
class QgsLayoutAtlas;
class QgsMessageBar;

/**
 * \ingroup gui
 * \brief Widget for configuring the properties of a layout atlas.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutAtlasWidget : public QWidget, private Ui::QgsLayoutAtlasWidgetBase
{
    Q_OBJECT
  public:
    //! Constructor
    QgsLayoutAtlasWidget( QWidget *parent, QgsPrintLayout *layout );
    //! Sets the message bar to which to emit messages
    void setMessageBar( QgsMessageBar *bar );

  private slots:
    void mUseAtlasCheckBox_stateChanged( int state );
    void changeCoverageLayer( QgsMapLayer *layer );
    void mAtlasFilenamePatternEdit_editingFinished();
    void mAtlasFilenameExpressionButton_clicked();
    void mAtlasHideCoverageCheckBox_stateChanged( int state );
    void mAtlasSingleFileCheckBox_stateChanged( int state );
    void mAtlasSortFeatureCheckBox_stateChanged( int state );
    void changesSortFeatureExpression( const QString &expression, bool valid );
    void mAtlasSortFeatureDirectionButton_clicked();
    void mAtlasFeatureFilterEdit_editingFinished();
    void mAtlasFeatureFilterButton_clicked();
    void mAtlasFeatureFilterCheckBox_stateChanged( int state );
    void pageNameExpressionChanged( const QString &expression, bool valid );
    void changeFileFormat();
    void updateGuiElements();
    void updateAtlasFeatures();

  private:
    QPointer<QgsPrintLayout> mLayout;
    QgsLayoutAtlas *mAtlas = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
    bool mBlockUpdates = false;

    void blockAllSignals( bool b );
    void checkLayerType( QgsVectorLayer *layer );
};

#endif // QGSLAYOUTATLASWIDGET_H
