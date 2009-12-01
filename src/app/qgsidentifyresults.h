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
/* $Id$ */
#ifndef QGSIDENTIFYRESULTS_H
#define QGSIDENTIFYRESULTS_H

#include "ui_qgsidentifyresultsbase.h"
#include "qgsattributeaction.h"
#include "qgscontexthelp.h"

#include <QWidget>
#include <QList>
#include <QSharedPointer>

class QCloseEvent;
class QTreeWidgetItem;
class QAction;
class QMenu;

class QgsMapLayer;
class QgsVectorLayer;
class QgsRubberBand;
class QgsMapCanvas;
class QDockWidget;

/**
 *@author Gary E.Sherman
 */

class QgsIdentifyResults: public QDialog, private Ui::QgsIdentifyResultsBase
{
    Q_OBJECT

  public:

    //! Constructor - takes it own copy of the QgsAttributeAction so
    // that it is independent of whoever created it.
    QgsIdentifyResults( QgsMapCanvas *canvas, QWidget *parent = 0, Qt::WFlags f = 0 );

    ~QgsIdentifyResults();

    /** Add add feature */
    void addFeature( QgsMapLayer *layer, int fid,
                     QString displayField, QString displayValue,
                     const QMap< QString, QString > &attributes,
                     const QMap< QString, QString > &derivedAttributes );

    /** Remove results */
    void clear();

    /** map tool was deactivated */
    void deactivate();

    /** map tool was activated */
    void activate();

    void closeEvent( QCloseEvent *e );

  signals:
    void selectedFeatureChanged( QgsVectorLayer *, int featureId );

  public slots:

    void show();

    void close();
    void contextMenuEvent( QContextMenuEvent* );

    void layerDestroyed();
    void editingToggled();
    void featureDeleted( int fid );

    void featureForm();
    void zoomToFeature();
    void copyAttributeValue();
    void copyFeatureAttributes();
    void highlightAll();
    void highlightLayer();
    void expandAll();
    void collapseAll();

    /* Called when an item is expanded so that we can ensure that the
       column width if expanded to show it */
    void itemExpanded( QTreeWidgetItem * );

    //! sends signal if current feature id has changed
    void handleCurrentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous );

    /* Item in tree was clicked */
    void itemClicked( QTreeWidgetItem *lvi, int column );

    QTreeWidgetItem *retrieveAttributes( QTreeWidgetItem *item, QList< QPair<QString, QString> > &attributes, int &currentIdx );

    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

  private:
    QMenu *mActionPopup;
    QMap<QTreeWidgetItem *, QSharedPointer<QgsRubberBand> > mRubberBands;
    QgsMapCanvas *mCanvas;

    QgsVectorLayer *vectorLayer( QTreeWidgetItem *item );
    QTreeWidgetItem *featureItem( QTreeWidgetItem *item );
    QTreeWidgetItem *layerItem( QTreeWidgetItem *item );
    QTreeWidgetItem *layerItem( QObject *layer );

    void highlightLayer( QTreeWidgetItem *object );
    void disconnectLayer( QObject *object );

    void setColumnText( int column, const QString & label );
    void expandColumnsToFit();
    void saveWindowLocation();
    void restorePosition();

    void highlightFeature( QTreeWidgetItem *item );

    void doAction( QTreeWidgetItem *item, int action );

    QDockWidget *mDock;
};

class QgsFeatureAction : public QAction
{
    Q_OBJECT

  public:
    QgsFeatureAction( const QString &name, QgsIdentifyResults *results, QgsVectorLayer *vl, int action, QTreeWidgetItem *featItem ) :
        QAction( name, results ), mResults( results ), mLayer( vl ), mAction( action ), mFeatItem( featItem )
    {}

  public slots:
    void execute();

  private:
    QgsIdentifyResults *mResults;
    QgsVectorLayer *mLayer;
    int mAction;
    QTreeWidgetItem *mFeatItem;
};

#endif
