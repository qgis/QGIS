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

class QgsAttributeForm;
class QgsVectorLayerTools;

class GUI_EXPORT QgsRelationReferenceWidget : public QWidget
{
    Q_OBJECT

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

    //! this sets the related feature using from the foreign key
    void setRelatedFeature( const QVariant &value );

    //! returns the related feature foreign key
    QVariant foreignKey();

    void setEditorContext( QgsAttributeEditorContext context, QgsMapCanvas* canvas, QgsMessageBar* messageBar );

    bool embedForm() {return mEmbedForm;}
    void setEmbedForm( bool display );

    bool readOnlySelector() {return mReadOnlySelector;}
    void setReadOnlySelector( bool readOnly );

    bool allowMapIdentification() {return mAllowMapIdentification;}
    void setAllowMapIdentification( bool allowMapIdentification );

  protected:
    virtual void showEvent( QShowEvent* e );

    void init();

  signals:
    void relatedFeatureChanged( QVariant );

  private slots:
    void highlightActionTriggered( QAction* action );
    void deleteHighlight();
    void openForm();
    void mapIdentificationTriggered( QAction* action );
    void comboReferenceChanged( int index );
    void removeRelatedFeature();
    void featureIdentified( const QgsFeature& feature );
    void mapToolDeactivated();


  private:
    QgsFeature relatedFeature();
    void highlightFeature( QgsFeature f = QgsFeature(), CanvasExtent canvasExtent = Fixed );
    void updateAttributeEditorFrame( const QgsFeature feature );

    // initialized
    QgsAttributeEditorContext mEditorContext;
    QgsMapCanvas* mCanvas;
    QgsMessageBar* mMessageBar;
    QVariant mForeignKey;
    QgsFeatureId mFeatureId;
    int mFkeyFieldIdx;
    bool mAllowNull;
    QgsHighlight* mHighlight;
    QgsMapToolIdentifyFeature* mMapTool;
    QgsMessageBarItem* mMessageBarItem;
    QString mRelationName;
    QgsAttributeForm* mReferencedAttributeForm;
    QgsVectorLayer* mReferencedLayer;
    QgsVectorLayer* mReferencingLayer;
    QWidget* mWindowWidget;
    bool mShown;
    QgsRelation mRelation;

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
    QAction* mRemoveFeatureAction;
    QComboBox* mComboBox;
    QgsCollapsibleGroupBox* mAttributeEditorFrame;
    QVBoxLayout* mAttributeEditorLayout;
    QLineEdit* mLineEdit;
};

#endif // QGSRELATIONREFERENCEWIDGET_H
