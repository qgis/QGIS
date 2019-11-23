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

#include "ui_qgslayoutatlaswidgetbase.h"

class QgsPrintLayout;
class QgsLayoutAtlas;
class QgsMessageBar;

/**
 * \ingroup app
  * A widget for layout atlas settings.
  */
class QgsLayoutAtlasWidget: public QWidget, private Ui::QgsLayoutAtlasWidgetBase
{
    Q_OBJECT
  public:
    QgsLayoutAtlasWidget( QWidget *parent, QgsPrintLayout *layout );
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
    QPointer< QgsPrintLayout > mLayout;
    QgsLayoutAtlas *mAtlas = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
    bool mBlockUpdates = false;

    void blockAllSignals( bool b );
    void checkLayerType( QgsVectorLayer *layer );
};
