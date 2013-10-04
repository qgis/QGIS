/***************************************************************************
    qgsrelationeditor.h
     --------------------------------------
    Date                 : 17.5.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRELATIONEDITOR_H
#define QGSRELATIONEDITOR_H

#include <QWidget>

#include "ui_qgsrelationeditorwidgetbase.h"
#include "qgsattributeeditorcontext.h"
#include "qgsrelation.h"

class QgsDualView;
class QgsFeature;
class QgsGenericFeatureSelectionManager;
class QgsVectorLayer;
class QgsVectorLayerTools;

class QgsRelationEditorWidget : public QgsCollapsibleGroupBox, private Ui::QgsRelationEditorWidgetBase
{
    Q_OBJECT

  public:
    QgsRelationEditorWidget( const QgsRelation& relation, const QgsFeature& feature, QgsAttributeEditorContext context, QWidget* parent = NULL );

    static QgsRelationEditorWidget* createRelationEditor( const QgsRelation& relation, const QgsFeature& feature, QgsAttributeEditorContext context, QWidget* parent = NULL );

  private slots:
    void onCollapsedStateChanged( bool state );
    void referencingLayerEditingToggled();
    void viewModeChanged( int mode );

    void on_mAddFeatureButton_clicked();
    void on_mLinkFeatureButton_clicked();
    void on_mDeleteFeatureButton_clicked();
    void on_mUnlinkFeatureButton_clicked();
    void on_mToggleEditingButton_toggled( bool state );

  private:
    QgsDualView* mDualView;
    QgsGenericFeatureSelectionManager* mFeatureSelectionMgr;
    QgsAttributeEditorContext mEditorContext;
    QgsRelation mRelation;
    QgsFeature mFeature;

};

#endif // QGSRELATIONEDITOR_H
