/***************************************************************************
                              qgsatlascompositionwidget.h
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

#include "ui_qgsatlascompositionwidgetbase.h"

class QgsComposition;
class QgsMapLayer;
class QgsComposerMap;
class QgsComposerItem;

/**
 * \ingroup app
  * Input widget for QgsAtlasComposition
  */
class QgsAtlasCompositionWidget:
  public QWidget,
  private Ui::QgsAtlasCompositionWidgetBase
{
    Q_OBJECT
  public:
    QgsAtlasCompositionWidget( QWidget *parent, QgsComposition *c );

  public slots:
    void mUseAtlasCheckBox_stateChanged( int state );
    void changeCoverageLayer( QgsMapLayer *layer );
    void mAtlasFilenamePatternEdit_editingFinished();
    void mAtlasFilenameExpressionButton_clicked();
    void mAtlasHideCoverageCheckBox_stateChanged( int state );
    void mAtlasSingleFileCheckBox_stateChanged( int state );

    void mAtlasSortFeatureCheckBox_stateChanged( int state );
    void changesSortFeatureField( const QString &fieldName );
    void mAtlasSortFeatureDirectionButton_clicked();
    void mAtlasFeatureFilterEdit_editingFinished();
    void mAtlasFeatureFilterButton_clicked();
    void mAtlasFeatureFilterCheckBox_stateChanged( int state );
    void pageNameExpressionChanged( const QString &expression, bool valid );

    void changeFileFormat();

  private slots:
    void updateGuiElements();

    void updateAtlasFeatures();

  private:
    QgsComposition *mComposition = nullptr;

    void blockAllSignals( bool b );
    void checkLayerType( QgsVectorLayer *layer );
};
