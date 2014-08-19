/***************************************************************************
    qgsrelationreferencewidget.h
     --------------------------------------
    Date                 : 20.4.2013
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

#ifndef QGSRELATIONREFERENCEWIDGET_H
#define QGSRELATIONREFERENCEWIDGET_H

#include "qgsattributeeditorcontext.h"
#include "qgscollapsiblegroupbox.h"
#include "qgsfeature.h"
#include "qgshighlight.h"
#include "qgsmaptoolidentifyfeature.h"

#include <QComboBox>
#include <QToolButton>
#include <QLineEdit>
#include <QVBoxLayout>

class QgsAttributeDialog;
class QgsVectorLayerTools;

class GUI_EXPORT QgsRelationReferenceWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( bool embedForm READ embedForm WRITE setEmbedForm )
    Q_PROPERTY( bool readOnlySelector READ readOnlySelector WRITE setReadOnlySelector )
    Q_PROPERTY( bool allowMapIdentification READ allowMapIdentification WRITE setAllowMapIdentification )

  public:
    enum CanvasExtent
    {
      Fixed,
      Pan,
      Scale
    };

    explicit QgsRelationReferenceWidget( QWidget* parent );

    ~QgsRelationReferenceWidget();

    void setRelation( QgsRelation relation , bool allowNullValue );

    void setRelationEditable( bool editable );

    void setRelatedFeature( const QVariant &value );

    QVariant relatedFeature();

    void setEditorContext( QgsAttributeEditorContext context, QgsMapCanvas* canvas, QgsMessageBar* messageBar );

    bool embedForm() {return mEmbedForm;}
    void setEmbedForm( bool display );

    bool readOnlySelector() {return mReadOnlySelector;}
    void setReadOnlySelector( bool readOnly );

    bool allowMapIdentification() {return mAllowMapIdentification;}
    void setAllowMapIdentification( bool allowMapIdentification );

  signals:
    void relatedFeatureChanged( QVariant );

  private slots:
    void highlightActionTriggered( QAction* action );
    void deleteHighlight();
    void openForm();
    void mapIdentification();
    void referenceChanged( int index );
    void setRelatedFeature( const QgsFeatureId& fid );
    void featureIdentified( const QgsFeatureId& fid );
    void mapToolDeactivated();


  private:
    void highlightFeature( CanvasExtent canvasExtent = Fixed );

    // initialized
    QgsAttributeEditorContext mEditorContext;
    QgsMapCanvas* mCanvas;
    QgsMessageBar* mMessageBar;
    QgsHighlight* mHighlight;
    bool mInitialValueAssigned;
    QgsMapToolIdentifyFeature* mMapTool;
    QgsMessageBarItem* mMessageBarItem;
    QString mRelationName;
    QgsAttributeDialog* mReferencedAttributeDialog;
    QgsVectorLayer* mReferencedLayer;
    QgsVectorLayer* mReferencingLayer;
    QWidget* mWindowWidget;

    // Q_PROPERTY
    bool mEmbedForm;
    bool mReadOnlySelector;
    bool mAllowMapIdentification;

    // UI
    QVBoxLayout* mTopLayout;
    QHash<QgsFeatureId, QVariant> mFidFkMap; // Mapping from feature id => foreign key
    QToolButton* mMapIdentificationButton;
    QToolButton* mOpenFormButton;
    QToolButton* mHighlightFeatureButton;
    QAction* mHighlightFeatureAction;
    QAction* mScaleHighlightFeatureAction;
    QAction* mPanHighlightFeatureAction;
    QAction* mOpenFormAction;
    QAction* mMapIdentificationAction;
    QComboBox* mComboBox;
    QgsCollapsibleGroupBox* mAttributeEditorFrame;
    QVBoxLayout* mAttributeEditorLayout;
    QLineEdit* mLineEdit;
};

#endif // QGSRELATIONREFERENCEWIDGET_H
