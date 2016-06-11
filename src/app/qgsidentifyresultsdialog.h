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

#include "ui_qgsidentifyresultsbase.h"
#include "qgsactionmanager.h"
#include "qgscontexthelp.h"
#include "qgsfeature.h"
#include "qgsfeaturestore.h"
#include "qgsfield.h"
#include "qgsmaptoolidentify.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsmaplayeractionregistry.h"
#include "qgswebview.h"

#include <QWidget>
#include <QList>

class QCloseEvent;
class QTreeWidgetItem;
class QAction;
class QMenu;

class QgsVectorLayer;
class QgsRasterLayer;
class QgsHighlight;
class QgsMapCanvas;
class QgsDockWidget;

class QwtPlotCurve;

/**
 *@author Gary E.Sherman
 */

class APP_EXPORT QgsIdentifyResultsWebView : public QgsWebView
{
    Q_OBJECT
  public:
    QgsIdentifyResultsWebView( QWidget *parent = nullptr );
    QSize sizeHint() const override;
  public slots:
    void print();
  protected:
    void contextMenuEvent( QContextMenuEvent* ) override;
    QgsWebView *createWindow( QWebPage::WebWindowType type ) override;
};

class APP_EXPORT QgsIdentifyResultsFeatureItem: public QTreeWidgetItem
{
  public:
    QgsIdentifyResultsFeatureItem( const QgsFields &fields, const QgsFeature &feature, const QgsCoordinateReferenceSystem &crs, const QStringList & strings = QStringList() );
    const QgsFields &fields() const { return mFields; }
    const QgsFeature &feature() const { return mFeature; }
    const QgsCoordinateReferenceSystem &crs() { return mCrs; }

  private:
    QgsFields mFields;
    QgsFeature mFeature;
    QgsCoordinateReferenceSystem mCrs;
};

class APP_EXPORT QgsIdentifyResultsWebViewItem: public QObject, public QTreeWidgetItem
{
    Q_OBJECT

  public:
    QgsIdentifyResultsWebViewItem( QTreeWidget *treeWidget = nullptr );
    QgsIdentifyResultsWebView *webView() { return mWebView; }
    void setHtml( const QString &html );
    /** @note added in 2.1 */
    void setContent( const QByteArray & data, const QString & mimeType = QString(), const QUrl & baseUrl = QUrl() );

  public slots:
    void loadFinished( bool ok );

  private:
    QgsIdentifyResultsWebView *mWebView;
};

class APP_EXPORT QgsIdentifyPlotCurve
{
  public:

    QgsIdentifyPlotCurve() { mPlotCurve = nullptr; }
    QgsIdentifyPlotCurve( const QMap<QString, QString> &attributes,
                          QwtPlot* plot, const QString &title = QString(), QColor color = QColor() );
    ~QgsIdentifyPlotCurve();

  private:
    QwtPlotCurve* mPlotCurve;

    QgsIdentifyPlotCurve( const QgsIdentifyPlotCurve& rh );
    QgsIdentifyPlotCurve& operator=( const QgsIdentifyPlotCurve& rh );
};

class APP_EXPORT QgsIdentifyResultsDialog: public QDialog, private Ui::QgsIdentifyResultsBase
{
    Q_OBJECT

  public:

    //! Constructor - takes it own copy of the QgsAttributeAction so
    // that it is independent of whoever created it.
    QgsIdentifyResultsDialog( QgsMapCanvas *canvas, QWidget *parent = nullptr, Qt::WindowFlags f = nullptr );

    ~QgsIdentifyResultsDialog();

    /** Add add feature from vector layer */
    void addFeature( QgsVectorLayer * layer,
                     const QgsFeature &f,
                     const QMap< QString, QString > &derivedAttributes );

    /** Add add feature from other layer */
    void addFeature( QgsRasterLayer * layer,
                     const QString& label,
                     const QMap< QString, QString > &attributes,
                     const QMap< QString, QString > &derivedAttributes,
                     const QgsFields &fields = QgsFields(),
                     const QgsFeature &feature = QgsFeature(),
                     const QMap<QString, QVariant> &params = ( QMap<QString, QVariant>() ) );

    /** Add feature from identify results */
    void addFeature( const QgsMapToolIdentify::IdentifyResult& result );

    /** Map tool was deactivated */
    void deactivate();

    /** Map tool was activated */
    void activate();

  signals:
    void selectedFeatureChanged( QgsVectorLayer *, QgsFeatureId featureId );

    // Emitted when raster identify format of a layer changed
    void formatChanged( QgsRasterLayer *layer );

    void copyToClipboard( QgsFeatureStore& featureStore );

    void activateLayer( QgsMapLayer * );

  public slots:
    /** Remove results */
    void clear();

    void updateViewModes();

    void show();

    void contextMenuEvent( QContextMenuEvent* ) override;

    void layerDestroyed();
    void editingToggled();
    void featureDeleted( QgsFeatureId fid );
    void attributeValueChanged( QgsFeatureId fid, int idx, const QVariant & );

    void featureForm();
    void zoomToFeature();
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

    /* Called when an item is expanded so that we can ensure that the
       column width if expanded to show it */
    void itemExpanded( QTreeWidgetItem * );

    //! sends signal if current feature id has changed
    void handleCurrentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous );
    /* Item in tree was clicked */
    void itemClicked( QTreeWidgetItem *lvi, int column );

    QTreeWidgetItem *retrieveAttributes( QTreeWidgetItem *item, QgsAttributeMap &attributes, int &currentIdx );

    void helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

    void on_cmbIdentifyMode_currentIndexChanged( int index );

    void on_cmbViewMode_currentIndexChanged( int index );

    void on_mExpandNewAction_triggered( bool checked );

    void on_cbxAutoFeatureForm_toggled( bool checked );

    void on_mExpandAction_triggered( bool checked ) { Q_UNUSED( checked ); expandAll(); }
    void on_mCollapseAction_triggered( bool checked ) { Q_UNUSED( checked ); collapseAll(); }

    void on_mActionCopy_triggered( bool checked );

    void formatChanged( int index );

    void printCurrentItem();

    void mapLayerActionDestroyed();

  private:
    QString representValue( QgsVectorLayer* vlayer, const QString& fieldName, const QVariant& value );

    enum ItemDataRole
    {
      GetFeatureInfoUrlRole = Qt::UserRole + 10
    };

    QMenu *mActionPopup;
    QMap<QTreeWidgetItem *, QgsHighlight * > mHighlights;
    QgsMapCanvas *mCanvas;
    QList<QgsFeature> mFeatures;
    QMap< QString, QMap< QString, QVariant > > mWidgetCaches;

    QgsMapLayer *layer( QTreeWidgetItem *item );
    QgsVectorLayer *vectorLayer( QTreeWidgetItem *item );
    QgsRasterLayer *rasterLayer( QTreeWidgetItem *item );
    QTreeWidgetItem *featureItem( QTreeWidgetItem *item );
    QTreeWidgetItem *layerItem( QTreeWidgetItem *item );
    QTreeWidgetItem *layerItem( QObject *layer );

    void highlightLayer( QTreeWidgetItem *object );
    void layerProperties( QTreeWidgetItem *object );
    void disconnectLayer( QObject *object );

    void saveWindowLocation();

    void setColumnText( int column, const QString & label );
    void expandColumnsToFit();

    void highlightFeature( QTreeWidgetItem *item );

    void doAction( QTreeWidgetItem *item, int action );

    void doMapLayerAction( QTreeWidgetItem *item, QgsMapLayerAction* action );

    QgsDockWidget *mDock;

    QVector<QgsIdentifyPlotCurve *> mPlotCurves;
};

class QgsIdentifyResultsDialogMapLayerAction : public QAction
{
    Q_OBJECT

  public:
    QgsIdentifyResultsDialogMapLayerAction( const QString &name, QObject *parent, QgsMapLayerAction* action, QgsMapLayer* layer, QgsFeature * f )
        : QAction( name, parent )
        , mAction( action )
        , mFeature( f )
        , mLayer( layer )
    {}

  public slots:
    void execute();

  private:
    QgsMapLayerAction* mAction;
    QgsFeature* mFeature;
    QgsMapLayer* mLayer;
};

#endif
