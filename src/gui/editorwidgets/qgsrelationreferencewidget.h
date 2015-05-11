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
#include "qgsattributetablemodel.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsfeaturelistmodel.h"

#include <QComboBox>
#include <QToolButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QStandardItemModel>

class QgsAttributeForm;
class QgsVectorLayerTools;

class GUI_EXPORT QgsRelationReferenceWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( bool openFormButtonVisible READ openFormButtonVisible WRITE setOpenFormButtonVisible )

  public:
    typedef QPair < QVariant, QgsFeatureId > ValueRelationItem;
    typedef QVector < ValueRelationItem > ValueRelationCache;

    enum CanvasExtent
    {
      Fixed,
      Pan,
      Scale
    };

    explicit QgsRelationReferenceWidget( QWidget* parent );

    ~QgsRelationReferenceWidget();

    void setRelation( QgsRelation relation, bool allowNullValue );

    void setRelationEditable( bool editable );

    //! this sets the related feature using from the foreign key
    void setForeignKey( const QVariant &value );

    //! returns the related feature foreign key
    QVariant foreignKey();

    void setEditorContext( const QgsAttributeEditorContext& context, QgsMapCanvas* canvas, QgsMessageBar* messageBar );

    //! determines if the form of the related feature will be shown
    bool embedForm() {return mEmbedForm;}
    void setEmbedForm( bool display );

    //! determines if the foreign key is shown in a combox box or a read-only line edit
    bool readOnlySelector() {return mReadOnlySelector;}
    void setReadOnlySelector( bool readOnly );

    //! determines if the widge offers the possibility to select the related feature on the map (using a dedicated map tool)
    bool allowMapIdentification() {return mAllowMapIdentification;}
    void setAllowMapIdentification( bool allowMapIdentification );

    //! If the widget will order the combobox entries by value
    bool orderByValue() { return mOrderByValue; }
    //! Set if the widget will order the combobox entries by value
    void setOrderByValue( bool orderByValue );
    //! Set the fields for which filter comboboxes will be created
    void setFilterFields( QStringList filterFields );

    //! determines the open form button is visible in the widget
    bool openFormButtonVisible() {return mOpenFormButtonVisible;}
    void setOpenFormButtonVisible( bool openFormButtonVisible );

    /**
     * Determines if the filters are chained
     *
     * @return True if filters are chained
     */
    bool chainFilters() { return mChainFilters; }


    /**
     * Set if filters are chained.
     * Chained filters restrict the option of subsequent filters based on the selection of a previous filter.
     *
     * @param chainFilters If chaining should be enabled
     */
    void setChainFilters( bool chainFilters );

    //! return the related feature (from the referenced layer)
    //! if no feature is related, it returns an invalid feature
    QgsFeature referencedFeature();

  public slots:
    //! open the form of the related feature in a new dialog
    void openForm();

    //! activate the map tool to select a new related feature on the map
    void mapIdentification();

    //! unset the currently related feature
    void deleteForeignKey();

  protected:
    virtual void showEvent( QShowEvent* e ) override;

    void init();

  signals:
    void foreignKeyChanged( QVariant );

  private slots:
    void highlightActionTriggered( QAction* action );
    void deleteHighlight();
    void comboReferenceChanged( int index );
    void featureIdentified( const QgsFeature& feature );
    void unsetMapTool();
    void mapToolDeactivated();
    void filterChanged();

  private:
    void highlightFeature( QgsFeature f = QgsFeature(), CanvasExtent canvasExtent = Fixed );
    void updateAttributeEditorFrame( const QgsFeature feature );

    // initialized
    QgsAttributeEditorContext mEditorContext;
    QgsMapCanvas* mCanvas;
    QgsMessageBar* mMessageBar;
    QVariant mForeignKey;
    QgsFeature mFeature;
    int mFkeyFieldIdx;
    bool mAllowNull;
    QgsHighlight* mHighlight;
    QgsMapToolIdentifyFeature* mMapTool;
    QgsMessageBarItem* mMessageBarItem;
    QString mRelationName;
    QgsAttributeForm* mReferencedAttributeForm;
    QgsVectorLayer* mReferencedLayer;
    QgsVectorLayer* mReferencingLayer;
    QgsAttributeTableModel* mMasterModel;
    QgsAttributeTableFilterModel* mFilterModel;
    QgsFeatureListModel* mFeatureListModel;
    QList<QComboBox*> mFilterComboBoxes;
    QWidget* mWindowWidget;
    bool mShown;
    QgsRelation mRelation;
    bool mIsEditable;
    QStringList mFilterFields;
    QMap<QString, QMap<QString, QSet<QString> > > mFilterCache;

    // Q_PROPERTY
    bool mEmbedForm;
    bool mReadOnlySelector;
    bool mAllowMapIdentification;
    bool mOrderByValue;
    bool mOpenFormButtonVisible;
    bool mChainFilters;

    // UI
    QVBoxLayout* mTopLayout;
    QToolButton* mMapIdentificationButton;
    QToolButton* mRemoveFKButton;
    QToolButton* mOpenFormButton;
    QToolButton* mHighlightFeatureButton;
    QAction* mHighlightFeatureAction;
    QAction* mScaleHighlightFeatureAction;
    QAction* mPanHighlightFeatureAction;
    QComboBox* mComboBox;
    QGroupBox* mChooserGroupBox;
    QWidget* mFilterContainer;
    QHBoxLayout* mFilterLayout;
    QgsCollapsibleGroupBox* mAttributeEditorFrame;
    QVBoxLayout* mAttributeEditorLayout;
    QLineEdit* mLineEdit;
    QLabel* mInvalidLabel;
};

#endif // QGSRELATIONREFERENCEWIDGET_H
