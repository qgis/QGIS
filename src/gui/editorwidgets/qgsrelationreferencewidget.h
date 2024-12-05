/***************************************************************************
    qgsrelationreferencewidget.h
     --------------------------------------
    Date                 : 20.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
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
#include "qgis_sip.h"
#include "qgsfeature.h"
#include "qobjectuniqueptr.h"

#include <QComboBox>
#include <QToolButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include "qgis_gui.h"

class QgsAttributeForm;
class QgsVectorLayerTools;
class QgsMapCanvas;
class QgsMessageBar;
class QgsHighlight;
class QgsMapTool;
class QgsMapToolIdentifyFeature;
class QgsMapToolDigitizeFeature;
class QgsMessageBarItem;
class QgsFeatureListComboBox;
class QgsCollapsibleGroupBox;
class QLabel;

#ifdef SIP_RUN
//%ModuleHeaderCode
// fix to allow compilation with sip that for some reason
// doesn't add this include to the file where the code from
// ConvertToSubClassCode goes.
#include <qgsrelationreferencewidget.h>
//%End
#endif

/**
 * \ingroup gui
 * \class QgsRelationReferenceWidget
 */
class GUI_EXPORT QgsRelationReferenceWidget : public QWidget
{
#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsRelationReferenceWidget *>( sipCpp ) )
      sipType = sipType_QgsRelationReferenceWidget;
    else
      sipType = NULL;
    SIP_END
#endif

    Q_OBJECT
    Q_PROPERTY( bool openFormButtonVisible READ openFormButtonVisible WRITE setOpenFormButtonVisible )

  public:
    enum CanvasExtent
    {
      Fixed,
      Pan,
      Scale
    };

    explicit QgsRelationReferenceWidget( QWidget *parent SIP_TRANSFERTHIS );

    ~QgsRelationReferenceWidget() override;

    void setRelation( const QgsRelation &relation, bool allowNullValue );

    void setRelationEditable( bool editable );

    /**
     * this sets the related feature using from the foreign key
     * \deprecated QGIS 3.10. Use setForeignKeys.
     */
    Q_DECL_DEPRECATED void setForeignKey( const QVariant &value ) SIP_DEPRECATED;

    /**
     * Sets the related feature using the foreign keys
     * \since QGIS 3.10
     */
    void setForeignKeys( const QVariantList &values );

    /**
     * returns the related feature foreign key
     * \deprecated QGIS 3.10
     */
    Q_DECL_DEPRECATED QVariant foreignKey() const SIP_DEPRECATED;

    //! returns the related feature foreign key

    /**
    * Returns the related feature foreign keys
    * \since QGIS 3.10
    */
    QVariantList foreignKeys() const;

    /**
     * Sets the editor \a context
     * \note if context cadDockWidget is null, it won't be possible to digitize
     * the geometry of a referenced feature from this widget
     */
    void setEditorContext( const QgsAttributeEditorContext &context, QgsMapCanvas *canvas, QgsMessageBar *messageBar );

    //! determines if the form of the related feature will be shown
    bool embedForm() { return mEmbedForm; }
    void setEmbedForm( bool display );

    //! determines if the drop-down is enabled
    bool readOnlySelector() { return mReadOnlySelector; }
    void setReadOnlySelector( bool readOnly );

    //! determines if the widget offers the possibility to select the related feature on the map (using a dedicated map tool)
    bool allowMapIdentification() { return mAllowMapIdentification; }
    void setAllowMapIdentification( bool allowMapIdentification );

    //! Sets the fields for which filter comboboxes will be created
    void setFilterFields( const QStringList &filterFields );

    //! determines the open form button is visible in the widget
    bool openFormButtonVisible() { return mOpenFormButtonVisible; }
    void setOpenFormButtonVisible( bool openFormButtonVisible );

    /**
     * Determines if the filters are chained
     *
     * \returns TRUE if filters are chained
     */
    bool chainFilters() const { return mChainFilters; }

    /**
     * Set if filters are chained.
     * Chained filters restrict the option of subsequent filters based on the selection of a previous filter.
     *
     * \param chainFilters If chaining should be enabled
     */
    void setChainFilters( bool chainFilters );

    /**
     * Returns the currently set filter expression.
     */
    QString filterExpression() const { return mFilterExpression; };

    /**
     * If not empty, will be used as filter expression.
     * Only if this evaluates to TRUE, the value will be shown.
     */
    void setFilterExpression( const QString &filterExpression );

    /**
     * Returns the related feature (from the referenced layer)
     * if no feature is related, it returns an invalid feature
     */
    QgsFeature referencedFeature() const;

    /**
     * Sets the widget to display in an indeterminate "mixed value" state.
     */
    void showIndeterminateState();

    /**
     * Determines if a button for adding new features should be shown.
     *
     */
    bool allowAddFeatures() const;

    /**
     * Determines if a button for adding new features should be shown.
     *
     */
    void setAllowAddFeatures( bool allowAddFeatures );

    /**
     * Returns the current relation, which might be invalid
     * \since QGIS 3.10
     */
    QgsRelation relation() const;

    /**
     * Set the current form feature (from the referencing layer)
     *
     * \since QGIS 3.10
     */
    void setFormFeature( const QgsFeature &formFeature );

    /**
     * Returns the public data source of the referenced layer
     * \since QGIS 3.12
     */
    QString referencedLayerDataSource() const;

    /**
     * Set the public data source of the referenced layer to \a referencedLayerDataSource
     * \since QGIS 3.12
     */
    void setReferencedLayerDataSource( const QString &referencedLayerDataSource );

    /**
     * Returns the data provider key of the referenced layer
     * \since QGIS 3.12
     */
    QString referencedLayerProviderKey() const;

    /**
     * Set the data provider key of the referenced layer to \a referencedLayerProviderKey
     * \since QGIS 3.12
     */
    void setReferencedLayerProviderKey( const QString &referencedLayerProviderKey );

    /**
     * Returns the id of the referenced layer
     * \since QGIS 3.12
     */
    QString referencedLayerId() const;

    /**
     * Set the id of the referenced layer to \a referencedLayerId
     * \since QGIS 3.12
     */
    void setReferencedLayerId( const QString &referencedLayerId );

    /**
     * Returns the name of the referenced layer
     * \since QGIS 3.12
     */
    QString referencedLayerName() const;

    /**
     * Set the name of the referenced layer to \a referencedLayerName
     * \since QGIS 3.12
     */
    void setReferencedLayerName( const QString &referencedLayerName );

    /**
     * Returns the limit of fetched features (0 means all features)
     * \since QGIS 3.32
     */
    int fetchLimit() const { return mFetchLimit; }

    /**
     * Set the limit of fetched features (0 means all features)
     * \since QGIS 3.32
     */
    void setFetchLimit( int fetchLimit ) { mFetchLimit = fetchLimit; }


  public slots:
    //! open the form of the related feature in a new dialog
    void openForm();

    //! activate the map tool to select a new related feature on the map
    void mapIdentification();

    //! unset the currently related feature
    void deleteForeignKeys();

  protected:
    void showEvent( QShowEvent *e ) override;

    void init();

  signals:

    /**
     * Emitted when the foreign key changed
     * \deprecated QGIS 3.10
     */
    Q_DECL_DEPRECATED void foreignKeyChanged( const QVariant &key ) SIP_DEPRECATED;

    /**
     * Emitted when the foreign keys changed
     * \since QGIS 3.10
     */
    void foreignKeysChanged( const QVariantList &keys );

  private slots:
    void highlightActionTriggered( QAction *action );
    void deleteHighlight();
    void comboReferenceChanged();
    void comboReferenceFoundChanged( bool found );
    void featureIdentified( const QgsFeature &feature );
    void setMapTool( QgsMapTool *mapTool );
    void unsetMapTool();
    void mapToolDeactivated();
    void filterChanged();
    void addEntry();
    void updateAddEntryButton();
    void entryAdded( const QgsFeature &f );
    void onKeyPressed( QKeyEvent *e );

  private:
    void highlightFeature( QgsFeature f = QgsFeature(), CanvasExtent canvasExtent = Fixed );
    void updateAttributeEditorFrame( const QgsFeature &feature );
    void disableChainedComboBoxes( const QComboBox *cb );
    void emitForeignKeysChanged( const QVariantList &foreignKeys, bool force = false );

    // initialized
    QgsAttributeEditorContext mEditorContext;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
    QVariantList mForeignKeys;
    QgsFeature mFeature;
    QgsFeature mFormFeature;
    // Index of the referenced layer key
    QStringList mReferencedFields;
    bool mAllowNull = true;
    QgsHighlight *mHighlight = nullptr;
    QgsMapTool *mCurrentMapTool = nullptr;
    QObjectUniquePtr<QgsMapToolIdentifyFeature> mMapToolIdentify;
    QObjectUniquePtr<QgsMapToolDigitizeFeature> mMapToolDigitize;
    QgsMessageBarItem *mMessageBarItem = nullptr;
    QgsAttributeForm *mReferencedAttributeForm = nullptr;
    QgsVectorLayer *mReferencedLayer = nullptr;
    QgsVectorLayer *mReferencingLayer = nullptr;
    QgsFeatureListComboBox *mComboBox = nullptr;
    QList<QComboBox *> mFilterComboBoxes;
    QString mFilterExpression;
    QWidget *mWindowWidget = nullptr;
    bool mShown = false;
    QgsRelation mRelation;
    bool mIsEditable = true;
    QStringList mFilterFields;
    QMap<QString, QMap<QString, QSet<QString>>> mFilterCache;
    bool mInitialized = false;
    int mFetchLimit = 0;

    // Q_PROPERTY
    bool mEmbedForm = false;
    bool mReadOnlySelector = false;
    bool mAllowMapIdentification = false;
    bool mOpenFormButtonVisible = true;
    bool mChainFilters = false;
    bool mAllowAddFeatures = false;
    QString mReferencedLayerId;
    QString mReferencedLayerName;
    QString mReferencedLayerDataSource;
    QString mReferencedLayerProviderKey;

    // UI
    QVBoxLayout *mTopLayout = nullptr;
    QToolButton *mMapIdentificationButton = nullptr;
    QToolButton *mRemoveFKButton = nullptr;
    QToolButton *mOpenFormButton = nullptr;
    QToolButton *mHighlightFeatureButton = nullptr;
    QToolButton *mAddEntryButton = nullptr;
    QAction *mHighlightFeatureAction = nullptr;
    QAction *mScaleHighlightFeatureAction = nullptr;
    QAction *mPanHighlightFeatureAction = nullptr;
    QWidget *mChooserContainer = nullptr;
    QWidget *mFilterContainer = nullptr;
    QHBoxLayout *mFilterLayout = nullptr;
    QgsCollapsibleGroupBox *mAttributeEditorFrame = nullptr;
    QVBoxLayout *mAttributeEditorLayout = nullptr;
    QLabel *mInvalidLabel = nullptr;

    friend class TestQgsRelationReferenceWidget;
};

#endif // QGSRELATIONREFERENCEWIDGET_H
