/***************************************************************************
                      qgsidentifyresults.h  -  description
                               ------------------
        begin                : Fri Oct 25 2002
        copyright            : (C) 2002 by Gary E.Sherman
        email                : sherman at mrcc dot com
        Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSIDENTIFYRESULTSDIALOG_H
#define QGSIDENTIFYRESULTSDIALOG_H

#include "qgis_app.h"
#include "ui_qgsidentifyresultsbase.h"
#include "qgshelp.h"
#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsmaptoolidentify.h"
#include "qgswebview.h"
#include "qgsexpressioncontext.h"
#include "qgsmaptoolselectionhandler.h"
#include "qgsrelation.h"

#include <QWidget>
#include <QList>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

class QCloseEvent;
class QToolButton;
class QTreeWidgetItem;
class QAction;
class QMenu;

class QgsFeatureStore;
class QgsVectorLayer;
class QgsRasterLayer;
class QgsHighlight;
class QgsMapCanvas;
class QgsMeshLayer;
class QgsDockWidget;
class QgsMapLayerAction;
class QgsEditorWidgetSetup;
class QgsSettingsEntryBool;
class QgsTiledSceneLayer;

class QwtPlotCurve;

class APP_EXPORT QgsIdentifyResultsWebView : public QgsWebView
{
    Q_OBJECT
  public:
    QgsIdentifyResultsWebView( QWidget *parent = nullptr );
    QSize sizeHint() const override;
  public slots:
    void print();
    void downloadRequested( const QNetworkRequest &request );
    void unsupportedContent( QNetworkReply *reply );
  protected:
    void contextMenuEvent( QContextMenuEvent * ) override;
    QgsWebView *createWindow( QWebPage::WebWindowType type ) override;
  private:
    void handleDownload( QUrl url );
};

class APP_EXPORT QgsIdentifyResultsFeatureItem: public QTreeWidgetItem
{
  public:
    QgsIdentifyResultsFeatureItem( const QgsFields &fields, const QgsFeature &feature, const QgsCoordinateReferenceSystem &crs, const QStringList &strings = QStringList() );
    const QgsFields &fields() const { return mFields; }
    const QgsFeature &feature() const { return mFeature; }
    QgsCoordinateReferenceSystem crs() const { return mCrs; }

  private:
    QgsFields mFields;
    QgsFeature mFeature;
    QgsCoordinateReferenceSystem mCrs;
};

//! Tree widget item being the parent item of a referenced or referencing relation
class APP_EXPORT QgsIdentifyResultsRelationItem: public QTreeWidgetItem
{
  public:
    //! Constructor
    QgsIdentifyResultsRelationItem( const QStringList &strings, const QgsRelation &relation, bool isReferencedRole, const QgsFeature &topFeature );

    //! Return the relation
    const QgsRelation &relation() const { return mRelation; }

    /**
     * Return true if getRelatedFeatures(mTopFeature) should be called on mRelation,
      * or false if getReferencedFeature(mTopFeature) should be called.
      */
    bool isReferencedRole() const { return mIsReferencedRole; }

    //! Return the feature that is the parent of this item.
    const QgsFeature &topFeature() const { return mTopFeature; }

  private:
    QgsRelation mRelation;
    bool mIsReferencedRole;
    QgsFeature mTopFeature;
};

class APP_EXPORT QgsIdentifyResultsWebViewItem: public QObject, public QTreeWidgetItem
{
    Q_OBJECT

  public:
    QgsIdentifyResultsWebViewItem( QTreeWidget *treeWidget = nullptr );
    QgsIdentifyResultsWebView *webView() { return mWebView; }
    void setHtml( const QString &html );
    void setContent( const QByteArray &data, const QString &mimeType = QString(), const QUrl &baseUrl = QUrl() );

  public slots:
    void loadFinished( bool ok );

  private:
    QgsIdentifyResultsWebView *mWebView = nullptr;
};

class APP_EXPORT QgsIdentifyPlotCurve
{
  public:

    QgsIdentifyPlotCurve() { mPlotCurve = nullptr; }
    QgsIdentifyPlotCurve( const QMap<QString, QString> &attributes,
                          QwtPlot *plot, const QString &title = QString(), QColor color = QColor() );
    ~QgsIdentifyPlotCurve();

    QgsIdentifyPlotCurve( const QgsIdentifyPlotCurve &rh ) = delete;
    QgsIdentifyPlotCurve &operator=( const QgsIdentifyPlotCurve &rh ) = delete;

  private:
    QwtPlotCurve *mPlotCurve = nullptr;

};

class APP_EXPORT QgsIdentifyResultsDialog: public QDialog, private Ui::QgsIdentifyResultsBase
{
    Q_OBJECT

  public:

    /**
     * Constructor
     */
    QgsIdentifyResultsDialog( QgsMapCanvas *canvas, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    ~QgsIdentifyResultsDialog() override;

    static const QgsSettingsEntryBool *settingHideNullValues;
    static const QgsSettingsEntryBool *settingShowRelations;

    //! Adds feature from vector layer
    void addFeature( QgsVectorLayer *layer,
                     const QgsFeature &f,
                     const QMap< QString, QString > &derivedAttributes );

    //! Adds feature from raster layer
    void addFeature( QgsRasterLayer *layer,
                     const QString &label,
                     const QMap< QString, QString > &attributes,
                     const QMap< QString, QString > &derivedAttributes,
                     const QgsFields &fields = QgsFields(),
                     const QgsFeature &feature = QgsFeature(),
                     const QMap<QString, QVariant> &params = ( QMap<QString, QVariant>() ) );

    /**
     * Adds results from mesh layer
     * \since QGIS 3.6
     */
    void addFeature( QgsMeshLayer *layer,
                     const QString &label,
                     const QMap< QString, QString > &attributes,
                     const QMap< QString, QString > &derivedAttributes );

    /**
     * Adds results from vector tile layer
     * \since QGIS 3.14
     */
    void addFeature( QgsVectorTileLayer *layer,
                     const QString &label,
                     const QgsFields &fields,
                     const QgsFeature &feature,
                     const QMap< QString, QString > &derivedAttributes );

    /**
     * Adds results from point cloud layer
     * \since QGIS 3.18
     */
    void addFeature( QgsPointCloudLayer *layer,
                     const QString &label,
                     const QMap< QString, QString > &attributes );

    /**
     * Adds results from tiled scene layer
     * \since QGIS 3.34
     */
    void addFeature( QgsTiledSceneLayer *layer,
                     const QString &label,
                     const QMap< QString, QString > &attributes,
                     const QMap< QString, QString > &derivedAttributes );

    //! Adds feature from identify results
    void addFeature( const QgsMapToolIdentify::IdentifyResult &result );

    //! Map tool was deactivated
    void deactivate();

    //! Map tool was activated
    void activate();

    /**
     * Sets an expression context scope to consider for resolving underlying
     * actions.
     *
     */
    void setExpressionContextScope( const QgsExpressionContextScope &scope );

    /**
     * Returns an expression context scope used for resolving underlying
     * actions.
     *
     */
    QgsExpressionContextScope expressionContextScope() const;

    QgsMapToolSelectionHandler::SelectionMode selectionMode() const;

  signals:
    void selectedFeatureChanged( QgsVectorLayer *, QgsFeatureId featureId );

    // Emitted when raster identify format of a layer changed
    void formatChanged( QgsRasterLayer *layer );

    void copyToClipboard( QgsFeatureStore &featureStore );

    void activateLayer( QgsMapLayer * );

    void selectionModeChanged();

  public slots:
    //! Remove results
    void clear();

    void updateViewModes();

    void show();

    void contextMenuEvent( QContextMenuEvent * ) override;

    void layerDestroyed();
    void editingToggled();
    void featureDeleted( QgsFeatureId fid );
    void attributeValueChanged( QgsFeatureId fid, int idx, const QVariant & );

    void featureForm();
    void zoomToFeature();
    void identifyFeature();
    void copyAttributeValue();
    void copyFeature();
    void toggleFeatureSelection();
    void copyFeatureAttributes();
    void copyGetFeatureInfoUrl();
    void highlightAll();
    void highlightLayer();
    void activateLayer();
    void layerProperties();
    void clearHighlights();
    void expandAll();
    void collapseAll();
    void selectFeatureByAttribute();

    //! sends signal if current feature id has changed
    void handleCurrentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous );
    /* Item in tree was clicked */
    void itemClicked( QTreeWidgetItem *lvi, int column );

    void itemExpanded( QTreeWidgetItem *item );

    QgsAttributeMap retrieveAttributes( QTreeWidgetItem *item );
    QVariant retrieveAttribute( QTreeWidgetItem *item );

    void cmbIdentifyMode_currentIndexChanged( int index );

    void cmbViewMode_currentIndexChanged( int index );

    void mExpandNewAction_triggered( bool checked );

    void mActionAutoFeatureForm_toggled( bool checked );

    void mActionHideDerivedAttributes_toggled( bool checked );

    void mActionHideNullValues_toggled( bool checked );

    void mActionShowRelations_toggled( bool checked );

    void mExpandAction_triggered( bool checked ) { Q_UNUSED( checked ) expandAll(); }
    void mCollapseAction_triggered( bool checked ) { Q_UNUSED( checked ) collapseAll(); }

    void mActionCopy_triggered( bool checked );

    void formatChanged( int index );

    void printCurrentItem();

    void mapLayerActionDestroyed();

    void showHelp();

  private:
    QString representValue( QgsVectorLayer *vlayer, const QgsEditorWidgetSetup &setup, const QString &fieldName, const QVariant &value );

    enum ItemDataRole
    {
      GetFeatureInfoUrlRole = Qt::UserRole + 10,
      FeatureRole,
    };

    QMenu *mActionPopup = nullptr;
    QHash<QTreeWidgetItem *, QgsHighlight * > mHighlights;
    QgsMapCanvas *mCanvas = nullptr;
    QList<QgsFeature> mFeatures;
    QMap< QString, QMap< QString, QVariant > > mWidgetCaches;
    QgsExpressionContextScope mExpressionContextScope;
    QToolButton *mSelectModeButton = nullptr;

    QgsMapLayer *layer( QTreeWidgetItem *item );
    static QgsVectorLayer *vectorLayer( QTreeWidgetItem *item );
    QgsRasterLayer *rasterLayer( QTreeWidgetItem *item );
    QgsMeshLayer *meshLayer( QTreeWidgetItem *item );
    QgsVectorTileLayer *vectorTileLayer( QTreeWidgetItem *item );
    QgsPointCloudLayer *pointCloudLayer( QTreeWidgetItem *item );
    QgsTiledSceneLayer *tiledSceneLayer( QTreeWidgetItem *item );
    QTreeWidgetItem *featureItem( QTreeWidgetItem *item );
    QTreeWidgetItem *layerItem( QTreeWidgetItem *item );
    QTreeWidgetItem *layerItem( QObject *layer );


    void highlightLayer( QTreeWidgetItem *object );
    void layerProperties( QTreeWidgetItem *object );
    void disconnectLayer( QObject *object );

    void saveWindowLocation();

    void setColumnText( int column, const QString &label );
    void expandColumnsToFit();

    void highlightFeature( QTreeWidgetItem *item );

    void doAction( QTreeWidgetItem *item, const QUuid &action );

    void doMapLayerAction( QTreeWidgetItem *item, QgsMapLayerAction *action );

    QgsDockWidget *mDock = nullptr;

    QVector<QgsIdentifyPlotCurve *> mPlotCurves;

    QgsMapToolSelectionHandler::SelectionMode mSelectionMode = QgsMapToolSelectionHandler::SelectSimple;

    void setSelectionMode();

    void initSelectionModes();
    QgsIdentifyResultsFeatureItem *createFeatureItem( QgsVectorLayer *vlayer, const QgsFeature &f, const QMap<QString, QString> &derivedAttributes, QTreeWidgetItem *parentItem );

    static bool isFeatureInAncestors( QTreeWidgetItem *item, const QgsVectorLayer *vlayer, const QgsFeature &f );

    friend class TestQgsIdentify;
};

class QgsIdentifyResultsDialogMapLayerAction : public QAction
{
    Q_OBJECT

  public:
    QgsIdentifyResultsDialogMapLayerAction( const QString &name, QObject *parent, QgsMapLayerAction *action, QgsMapLayer *layer, QgsFeature *f )
      : QAction( name, parent )
      , mAction( action )
      , mFeature( f )
      , mLayer( layer )
    {}

  public slots:
    void execute();

  private:
    QgsMapLayerAction *mAction = nullptr;
    QgsFeature *mFeature = nullptr;
    QgsMapLayer *mLayer = nullptr;
};

#endif
