/***************************************************************************
                     qgsidentifyresults.cpp  -  description
                              -------------------
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

#include "qgsidentifyresultsdialog.h"
#include "qgsapplication.h"
#include "qgisapp.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgshighlight.h"
#include "qgsgeometry.h"
#include "qgsattributedialog.h"
#include "qgsmapcanvas.h"
#include "qgsattributeaction.h"
#include "qgsfeatureaction.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"

#include <QCloseEvent>
#include <QLabel>
#include <QAction>
#include <QTreeWidgetItem>
#include <QPixmap>
#include <QSettings>
#include <QMenu>
#include <QClipboard>
#include <QDockWidget>
#include <QMenuBar>
#include <QPushButton>
#include <QWebView>
#include <QPrinter>
#include <QPrintDialog>
#include <QDesktopServices>
#include <QMessageBox>
#include <QComboBox>
#include <QWebFrame>

QgsIdentifyResultsWebView::QgsIdentifyResultsWebView( QWidget *parent ) : QWebView( parent )
{
  setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
  page()->setNetworkAccessManager( QgsNetworkAccessManager::instance() );
  // page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );
  page()->setLinkDelegationPolicy( QWebPage::DontDelegateLinks );
  settings()->setAttribute( QWebSettings::LocalContentCanAccessRemoteUrls, true );
  settings()->setAttribute( QWebSettings::JavascriptCanOpenWindows, true );
#ifdef QGISDEBUG
  settings()->setAttribute( QWebSettings::DeveloperExtrasEnabled, true );
#endif
}

void QgsIdentifyResultsWebView::print( void )
{
  QPrinter printer;
  QPrintDialog *dialog = new QPrintDialog( &printer );
  if ( dialog->exec() == QDialog::Accepted )
  {
    QWebView::print( &printer );
  }
}

void QgsIdentifyResultsWebView::contextMenuEvent( QContextMenuEvent *e )
{
  QMenu *menu = page()->createStandardContextMenu();
  if ( !menu )
    return;

  QAction *action = new QAction( tr( "Print" ), this );
  connect( action, SIGNAL( triggered() ), this, SLOT( print() ) );
  menu->addAction( action );
  menu->exec( e->globalPos() );
  delete menu;
}

QWebView *QgsIdentifyResultsWebView::createWindow( QWebPage::WebWindowType type )
{
  QDialog *d = new QDialog( this );
  QLayout *l = new QVBoxLayout( d );

  QWebView *wv = new QWebView( d );
  l->addWidget( wv );

  wv->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
  wv->page()->setNetworkAccessManager( QgsNetworkAccessManager::instance() );
  wv->settings()->setAttribute( QWebSettings::LocalContentCanAccessRemoteUrls, true );
  wv->settings()->setAttribute( QWebSettings::JavascriptCanOpenWindows, true );
#ifdef QGISDEBUG
  wv->settings()->setAttribute( QWebSettings::DeveloperExtrasEnabled, true );
#endif

  d->setModal( type != QWebPage::WebBrowserWindow );
  d->show();

  return wv;
}

// QgsIdentifyResultsWebView size:
// ---------------
//
// 1) QTreeWidget is not able to scroll continuously through the content of large widgets
//    inserted into items via setItemWidget, it always jumps to the top of next
//    item and it is not able to scroll to the bottom of an inserted large
//    widget (until the QTreeWidget itself is large enough to show the whole
//    inserted widget).  => We have to keep the height of QgsIdentifyResultsWebView smaller
//    than the height of QTreeWidget so that a user can see it entire, even if
//    this height is smaller than QgsIdentifyResultsWebView content (i.e. QgsIdentifyResultsWebView scroolbar
//    is added). We make it even a bit smaller so that a user can see a bit of
//    context (items above/below) when scrolling which is more pleasant.
//
// 2) contentsSize() is 0,0 until a page is loaded. If there are no external
//    resources (like images) used, contentsSize() is available immediately
//    after setHtml(), otherwise the contentSize() is 0,0 until the page is
//    loaded and contentsSizeChanged () is emitted.
//
// 3) If QgsIdentifyResultsWebView is resized (on page load) after it was inserted into
//    QTreeWidget, the row does not reflect that change automaticaly and
//    consecutive resize of QTreeWidget will cause to shrink QgsIdentifyResultsWebView to the
//    original row height.  That is expected, Qt: "setItemWidget() should only
//    be used to display static content...  => we must not change QgsIdentifyResultsWebView
//    size after it was inserted to QTreeWidget

// TODO(?): Sometimes it may happen that if multiple QgsIdentifyResultsWebView are inserted to
// QTreeWidget for the first time, and both share the same external source
// (image) the layout gets somehow confused - wrong positions, overlapped (Qt
// bug?) until next QTreeWidget resize.

// TODO(?): if the results dialog is resized to smaller height, existing
// QgsIdentifyResultsWebView are not (and must not be!) resized and scrolling becomes a bit
// unpleasant until next identify. AFAIK it could only be solved using
// QItemDelegate.

// size hint according to content
QSize QgsIdentifyResultsWebView::sizeHint() const
{
  QSize s = page()->mainFrame()->contentsSize();
  QgsDebugMsg( QString( "content size: %1 x %2" ).arg( s.width() ).arg( s.height() ) );
  int height = s.height();

  // if page is not yet loaded set some minimum height
  if ( height == 0 )
  {
    height = 100;
  }
  else
  {
    // parent is qt_scrollarea_viewport
    // parent is not available the first time - before results dialog was shown
    QWidget *widget = qobject_cast<QWidget *>( parent() );
    if ( widget )
    {
      int max = widget->size().height() * 0.9;
      QgsDebugMsg( QString( "parent widget height = %1 max height = %2" ).arg( widget->size().height() ).arg( max ) );
      if ( height > max )
        height = max;
    }
    else
    {
      QgsDebugMsg( "parent not available" ) ;
    }
  }

  s = QSize( size().width(), height );
  QgsDebugMsg( QString( "size: %1 x %2" ).arg( s.width() ).arg( s.height() ) );
  return s;
}

class QgsIdentifyResultsDock : public QDockWidget
{
  public:
    QgsIdentifyResultsDock( const QString & title, QWidget * parent = 0, Qt::WindowFlags flags = 0 )
        : QDockWidget( title, parent, flags )
    {
      setObjectName( "IdentifyResultsTableDock" ); // set object name so the position can be saved
    }

    virtual void closeEvent( QCloseEvent *e )
    {
      Q_UNUSED( e );
      deleteLater();
    }
};

QgsIdentifyResultsFeatureItem::QgsIdentifyResultsFeatureItem( const QgsFields &fields, const QgsFeature &feature, const QgsCoordinateReferenceSystem &crs, const QStringList & strings )
    : QTreeWidgetItem( strings )
    , mFields( fields )
    , mFeature( feature )
    , mCrs( crs )
{
}

void QgsIdentifyResultsWebViewItem::setHtml( const QString &html )
{
  mWebView->setHtml( html );
}

QgsIdentifyResultsWebViewItem::QgsIdentifyResultsWebViewItem( QTreeWidget *treeWidget )
{
  mWebView = new QgsIdentifyResultsWebView( treeWidget );
  mWebView->hide();
  setText( 0, tr( "Loading..." ) );
  connect( mWebView->page(), SIGNAL( loadFinished( bool ) ), this, SLOT( loadFinished( bool ) ) );
}

void QgsIdentifyResultsWebViewItem::loadFinished( bool ok )
{
  QgsDebugMsg( "Entered" );
  Q_UNUSED( ok );

  mWebView->show();
  treeWidget()->setItemWidget( this, 0, mWebView );

  // Span columns to save some space, must be after setItemWidget() to take efect.
  setFirstColumnSpanned( true );

  disconnect( mWebView->page(), SIGNAL( loadFinished( bool ) ), this, SLOT( loadFinished( bool ) ) );

}

// Tree hierarchy
//
// layer [userrole: QgsMapLayer]
//   feature: displayfield|displayvalue [userrole: fid, index in feature list]
//     derived attributes (if any) [userrole: "derived"]
//       name value
//     actions (if any) [userrole: "actions"]
//       edit [userrole: "edit"]
//       action [userrole: "action", idx]
//     displayname [userroles: fieldIdx, original name] displayvalue [userrole: original value]
//     displayname [userroles: fieldIdx, original name] displayvalue [userrole: original value]
//     displayname [userroles: fieldIdx, original name] displayvalue [userrole: original value]
//   feature
//     derived attributes (if any)
//       name value
//     actions (if any)
//       action
//     name value

QgsIdentifyResultsDialog::QgsIdentifyResultsDialog( QgsMapCanvas *canvas, QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f )
    , mActionPopup( 0 )
    , mCanvas( canvas )
    , mDock( NULL )
{
  setupUi( this );

  mExpandToolButton->setIcon( QgsApplication::getThemeIcon( "/mActionExpandTree.png" ) );
  mCollapseToolButton->setIcon( QgsApplication::getThemeIcon( "/mActionCollapseTree.png" ) );
  mExpandNewToolButton->setIcon( QgsApplication::getThemeIcon( "/mActionExpandNewTree.png" ) );
  mCopyToolButton->setIcon( QgsApplication::getThemeIcon( "/mActionEditCopy.png" ) );
  mPrintToolButton->setIcon( QgsApplication::getThemeIcon( "/mActionFilePrint.png" ) );

  QSettings mySettings;
  restoreGeometry( mySettings.value( "/Windows/Identify/geometry" ).toByteArray() );
  bool myDockFlag = mySettings.value( "/qgis/dockIdentifyResults", false ).toBool();
  if ( myDockFlag )
  {
    mDock = new QgsIdentifyResultsDock( tr( "Identify Results" ) , QgisApp::instance() );
    mDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    mDock->setWidget( this );
    QgisApp::instance()->addDockWidget( Qt::LeftDockWidgetArea, mDock );
  }
  mExpandNewToolButton->setChecked( mySettings.value( "/Map/identifyExpand", false ).toBool() );
  mCopyToolButton->setEnabled( false );
  lstResults->setColumnCount( 2 );
  setColumnText( 0, tr( "Feature" ) );
  setColumnText( 1, tr( "Value" ) );

  int width = mySettings.value( "/Windows/Identify/columnWidth", "0" ).toInt();
  if ( width > 0 )
  {
    lstResults->setColumnWidth( 0, width );
  }

  connect( buttonBox, SIGNAL( rejected() ), this, SLOT( close() ) );

  connect( lstResults, SIGNAL( itemExpanded( QTreeWidgetItem* ) ),
           this, SLOT( itemExpanded( QTreeWidgetItem* ) ) );

  connect( lstResults, SIGNAL( currentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ),
           this, SLOT( handleCurrentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ) );

  connect( lstResults, SIGNAL( itemClicked( QTreeWidgetItem*, int ) ),
           this, SLOT( itemClicked( QTreeWidgetItem*, int ) ) );

  connect( mPrintToolButton, SIGNAL( clicked() ),
           this, SLOT( printCurrentItem() ) );
}

QgsIdentifyResultsDialog::~QgsIdentifyResultsDialog()
{
  clearHighlights();
  if ( mActionPopup )
    delete mActionPopup;
}

QTreeWidgetItem *QgsIdentifyResultsDialog::layerItem( QObject *object )
{
  for ( int i = 0; i < lstResults->topLevelItemCount(); i++ )
  {
    QTreeWidgetItem *item = lstResults->topLevelItem( i );

    if ( item->data( 0, Qt::UserRole ).value<QObject *>() == object )
      return item;
  }

  return 0;
}

void QgsIdentifyResultsDialog::addFeature( QgsMapToolIdentify::IdentifyResult result )
{
  if ( result.mLayer->type() == QgsMapLayer::VectorLayer )
  {
    addFeature( qobject_cast<QgsVectorLayer *>( result.mLayer ), result.mFeature, result.mDerivedAttributes );
  }
  else if ( result.mLayer->type() == QgsMapLayer::RasterLayer )
  {
    addFeature( qobject_cast<QgsRasterLayer *>( result.mLayer ), result.mLabel, result.mAttributes, result.mDerivedAttributes, result.mFields,  result.mFeature, result.mParams );
  }
}

void QgsIdentifyResultsDialog::addFeature( QgsVectorLayer *vlayer, const QgsFeature &f, const QMap<QString, QString> &derivedAttributes )
{
  QTreeWidgetItem *layItem = layerItem( vlayer );

  if ( layItem == 0 )
  {
    layItem = new QTreeWidgetItem( QStringList() << QString::number( lstResults->topLevelItemCount() ) << vlayer->name() );
    layItem->setData( 0, Qt::UserRole, QVariant::fromValue( qobject_cast<QObject *>( vlayer ) ) );
    lstResults->addTopLevelItem( layItem );

    connect( vlayer, SIGNAL( layerDeleted() ), this, SLOT( layerDestroyed() ) );
    connect( vlayer, SIGNAL( layerCrsChanged() ), this, SLOT( layerDestroyed() ) );
    connect( vlayer, SIGNAL( featureDeleted( QgsFeatureId ) ), this, SLOT( featureDeleted( QgsFeatureId ) ) );
    connect( vlayer, SIGNAL( attributeValueChanged( QgsFeatureId, int, const QVariant & ) ),
             this,   SLOT( attributeValueChanged( QgsFeatureId, int, const QVariant & ) ) );
    connect( vlayer, SIGNAL( editingStarted() ), this, SLOT( editingToggled() ) );
    connect( vlayer, SIGNAL( editingStopped() ), this, SLOT( editingToggled() ) );
  }

  //QgsIdentifyResultsFeatureItem *featItem = new QgsIdentifyResultsFeatureItem( fields, f, crs );
  QgsIdentifyResultsFeatureItem *featItem = new QgsIdentifyResultsFeatureItem( vlayer->pendingFields(), f, vlayer->crs() );
  featItem->setData( 0, Qt::UserRole, FID_TO_STRING( f.id() ) );
  featItem->setData( 0, Qt::UserRole + 1, mFeatures.size() );
  mFeatures << f;
  layItem->addChild( featItem );

  const QgsFields &fields = vlayer->pendingFields();
  const QgsAttributes& attrs = f.attributes();
  for ( int i = 0; i < attrs.count(); ++i )
  {
    if ( i >= fields.count() )
      continue;

    QString value = fields[i].displayString( attrs[i] );
    QTreeWidgetItem *attrItem = new QTreeWidgetItem( QStringList() << QString::number( i ) << value );

    attrItem->setData( 0, Qt::DisplayRole, vlayer->attributeDisplayName( i ) );
    attrItem->setData( 0, Qt::UserRole, fields[i].name() );
    attrItem->setData( 0, Qt::UserRole + 1, i );

    attrItem->setData( 1, Qt::UserRole, value );

    switch ( vlayer->editType( i ) )
    {
      case QgsVectorLayer::Hidden:
        // skip the item
        delete attrItem;
        continue;

      case QgsVectorLayer::ValueMap:
        value = vlayer->valueMap( i ).key( value, QString( "(%1)" ).arg( value ) );
        break;

      case QgsVectorLayer::Calendar:
        if ( attrs[i].canConvert( QVariant::Date ) )
          value = attrs[i].toDate().toString( vlayer->dateFormat( i ) );
        break;

      default:
        break;
    }

    attrItem->setData( 1, Qt::DisplayRole, value );

    if ( fields[i].name() == vlayer->displayField() )
    {
      featItem->setText( 0, attrItem->text( 0 ) );
      featItem->setText( 1, attrItem->text( 1 ) );
    }

    featItem->addChild( attrItem );
  }

  if ( derivedAttributes.size() >= 0 )
  {
    QTreeWidgetItem *derivedItem = new QTreeWidgetItem( QStringList() << tr( "(Derived)" ) );
    derivedItem->setData( 0, Qt::UserRole, "derived" );
    featItem->addChild( derivedItem );

    for ( QMap< QString, QString>::const_iterator it = derivedAttributes.begin(); it != derivedAttributes.end(); it++ )
    {
      derivedItem->addChild( new QTreeWidgetItem( QStringList() << it.key() << it.value() ) );
    }
  }

  if ( vlayer->pendingFields().size() > 0 || vlayer->actions()->size() )
  {
    QTreeWidgetItem *actionItem = new QTreeWidgetItem( QStringList() << tr( "(Actions)" ) );
    actionItem->setData( 0, Qt::UserRole, "actions" );
    featItem->addChild( actionItem );

    if ( vlayer->pendingFields().size() > 0 )
    {
      QTreeWidgetItem *editItem = new QTreeWidgetItem( QStringList() << "" << ( vlayer->isEditable() ? tr( "Edit feature form" ) : tr( "View feature form" ) ) );
      editItem->setIcon( 0, QgsApplication::getThemeIcon( vlayer->isEditable() ? "/mIconEditable.png" : "/mIconEditable.png" ) );
      editItem->setData( 0, Qt::UserRole, "edit" );
      actionItem->addChild( editItem );
    }

    for ( int i = 0; i < vlayer->actions()->size(); i++ )
    {
      const QgsAction &action = vlayer->actions()->at( i );

      if ( !action.runable() )
        continue;

      QTreeWidgetItem *twi = new QTreeWidgetItem( QStringList() << "" << action.name() );
      twi->setIcon( 0, QgsApplication::getThemeIcon( "/mAction.svg" ) );
      twi->setData( 0, Qt::UserRole, "action" );
      twi->setData( 0, Qt::UserRole + 1, QVariant::fromValue( i ) );
      actionItem->addChild( twi );
    }
  }

  highlightFeature( featItem );
}

void QgsIdentifyResultsDialog::addFeature( QgsRasterLayer *layer,
    QString label,
    const QMap<QString, QString> &attributes,
    const QMap<QString, QString> &derivedAttributes,
    const QgsFields &fields,
    const QgsFeature &feature,
    const QMap<QString, QVariant> &params )
{
  QgsDebugMsg( QString( "feature.isValid() = %1" ).arg( feature.isValid() ) );
  QTreeWidgetItem *layItem = layerItem( layer );

  QgsRaster::IdentifyFormat currentFormat = QgsRasterDataProvider::identifyFormatFromName( layer->customProperty( "identify/format" ).toString() );

  if ( layItem == 0 )
  {
    layItem = new QTreeWidgetItem( QStringList() << QString::number( lstResults->topLevelItemCount() ) << layer->name() );
    layItem->setData( 0, Qt::UserRole, QVariant::fromValue( qobject_cast<QObject *>( layer ) ) );

    lstResults->addTopLevelItem( layItem );

    QComboBox *formatCombo = new QComboBox();

    // Add all supported formats, best first. HTML is considered the best because
    // it usually holds most informations.
    int capabilities = layer->dataProvider()->capabilities();
    QList<QgsRaster::IdentifyFormat> formats;
    formats << QgsRaster::IdentifyFormatHtml
    << QgsRaster::IdentifyFormatFeature
    << QgsRaster::IdentifyFormatText
    << QgsRaster::IdentifyFormatValue;
    foreach ( QgsRaster::IdentifyFormat f, formats )
    {
      if ( !( QgsRasterDataProvider::identifyFormatToCapability( f ) & capabilities ) )
        continue;
      formatCombo->addItem( QgsRasterDataProvider::identifyFormatLabel( f ), f );
      formatCombo->setItemData( formatCombo->count() - 1, qVariantFromValue( qobject_cast<QObject *>( layer ) ), Qt::UserRole + 1 );
      if ( currentFormat == f )
        formatCombo->setCurrentIndex( formatCombo->count() - 1 );
    }

    if ( formatCombo->count() > 1 )
    {
      // Add format combo box item
      // Space added before format to keep it first in ordered list: TODO better (user data)
      QTreeWidgetItem *formatItem = new QTreeWidgetItem( QStringList() << " " + tr( "Format" ) );
      layItem->addChild( formatItem );
      lstResults->setItemWidget( formatItem, 1, formatCombo );
      connect( formatCombo, SIGNAL( currentIndexChanged( int ) ), this, SLOT( formatChanged( int ) ) );
    }
    else
    {
      delete formatCombo;
    }

    connect( layer, SIGNAL( destroyed() ), this, SLOT( layerDestroyed() ) );
    connect( layer, SIGNAL( layerCrsChanged() ), this, SLOT( layerDestroyed() ) );
  }
  // Set/reset getFeatureInfoUrl (currently only available for Feature, so it may change if format changes)
  layItem->setData( 0, GetFeatureInfoUrlRole, params.value( "getFeatureInfoUrl" ) );

  QgsIdentifyResultsFeatureItem *featItem = new QgsIdentifyResultsFeatureItem( fields, feature, layer->crs(), QStringList() << label << "" );
  layItem->addChild( featItem );

  // add feature attributes
  if ( feature.isValid() )
  {
    QgsDebugMsg( QString( "fields size = %1 attributes size = %2" ).arg( fields.size() ).arg( feature.attributes().size() ) );
    const QgsAttributes& attrs = feature.attributes();
    for ( int i = 0; i < attrs.count(); ++i )
    {
      if ( i >= fields.count() )
        continue;

      QTreeWidgetItem *attrItem = new QTreeWidgetItem( QStringList() << QString::number( i ) << attrs[i].toString() );

      attrItem->setData( 0, Qt::DisplayRole, fields[i].name() );

      QVariant value = attrs[i];
      attrItem->setData( 1, Qt::DisplayRole, value );
      featItem->addChild( attrItem );
    }
  }

  if ( currentFormat == QgsRaster::IdentifyFormatHtml )
  {
    QgsIdentifyResultsWebViewItem *attrItem = new QgsIdentifyResultsWebViewItem( lstResults );
    featItem->addChild( attrItem ); // before setHtml()!
    attrItem->setHtml( attributes.begin().value() );
  }
  else
  {
    for ( QMap<QString, QString>::const_iterator it = attributes.begin(); it != attributes.end(); it++ )
    {
      featItem->addChild( new QTreeWidgetItem( QStringList() << it.key() << it.value() ) );
    }
  }

  if ( derivedAttributes.size() >= 0 )
  {
    QTreeWidgetItem *derivedItem = new QTreeWidgetItem( QStringList() << tr( "(Derived)" ) );
    derivedItem->setData( 0, Qt::UserRole, "derived" );
    featItem->addChild( derivedItem );

    for ( QMap< QString, QString>::const_iterator it = derivedAttributes.begin(); it != derivedAttributes.end(); it++ )
    {
      derivedItem->addChild( new QTreeWidgetItem( QStringList() << it.key() << it.value() ) );
    }
  }
}

void QgsIdentifyResultsDialog::editingToggled()
{
  QTreeWidgetItem *layItem = layerItem( sender() );
  QgsVectorLayer *vlayer = vectorLayer( layItem );
  if ( !layItem || !vlayer )
    return;

  // iterate features
  int i;
  for ( i = 0; i < layItem->childCount(); i++ )
  {
    QTreeWidgetItem *featItem = layItem->child( i );

    int j;
    for ( j = 0; j < featItem->childCount() && featItem->child( j )->data( 0, Qt::UserRole ).toString() != "actions"; j++ )
      QgsDebugMsg( QString( "%1: skipped %2" ).arg( featItem->child( j )->data( 0, Qt::UserRole ).toString() ) );

    if ( j == featItem->childCount() || featItem->child( j )->childCount() < 1 )
      continue;

    QTreeWidgetItem *actions = featItem->child( j );

    for ( j = 0; i < actions->childCount() && actions->child( j )->data( 0, Qt::UserRole ).toString() != "edit"; j++ )
      ;

    if ( j == actions->childCount() )
      continue;

    QTreeWidgetItem *editItem = actions->child( j );
    editItem->setIcon( 0, QgsApplication::getThemeIcon( vlayer->isEditable() ? "/mIconEditable.png" : "/mIconEditable.png" ) );
    editItem->setText( 1, vlayer->isEditable() ? tr( "Edit feature form" ) : tr( "View feature form" ) );
  }
}

// Call to show the dialog box.
void QgsIdentifyResultsDialog::show()
{
  // Enforce a few things before showing the dialog box
  lstResults->sortItems( 0, Qt::AscendingOrder );
  // column width is now stored in settings
  //expandColumnsToFit();

  if ( lstResults->topLevelItemCount() > 0 )
  {
    QTreeWidgetItem *layItem = lstResults->topLevelItem( 0 );
    QTreeWidgetItem *featItem = layItem->child( 0 );

    if ( lstResults->topLevelItemCount() == 1 &&
         layItem->childCount() == 1 &&
         QSettings().value( "/Map/identifyAutoFeatureForm", false ).toBool() )
    {
      QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( layItem->data( 0, Qt::UserRole ).value<QObject *>() );
      if ( layer )
      {
        // if this is the only feature and it's on a vector layer
        // don't show the form dialog instead of the results window
        lstResults->setCurrentItem( featItem );
        featureForm();
        clear();
        return;
      }
    }

    // expand first layer and feature
    featItem->setExpanded( true );
    layItem->setExpanded( true );
  }

  // expand all if enabled
  if ( mExpandNewToolButton->isChecked() )
  {
    lstResults->expandAll();
  }

  QDialog::show();
  raise();
}

// Slot called when user clicks the Close button
// (saves the current window size/position)
void QgsIdentifyResultsDialog::close()
{
  clear();

  delete mActionPopup;
  mActionPopup = 0;

  saveWindowLocation();
  done( 0 );
  if ( mDock )
    mDock->close();
}

// Save the current window size/position before closing
// from window menu or X in titlebar
void QgsIdentifyResultsDialog::closeEvent( QCloseEvent *e )
{
  // We'll close in our own good time thanks...
  e->ignore();
  close();
}

void QgsIdentifyResultsDialog::itemClicked( QTreeWidgetItem *item, int column )
{
  Q_UNUSED( column );
  if ( item->data( 0, Qt::UserRole ).toString() == "edit" )
  {
    lstResults->setCurrentItem( item );
    featureForm();
  }
  else if ( item->data( 0, Qt::UserRole ).toString() == "action" )
  {
    doAction( item, item->data( 0, Qt::UserRole + 1 ).toInt() );
  }
}

// Popup (create if necessary) a context menu that contains a list of
// actions that can be applied to the data in the identify results
// dialog box.

void QgsIdentifyResultsDialog::contextMenuEvent( QContextMenuEvent* event )
{
  QgsDebugMsg( "Entered" );
  QTreeWidgetItem *item = lstResults->itemAt( lstResults->viewport()->mapFrom( this, event->pos() ) );
  // if the user clicked below the end of the attribute list, just return
  if ( !item )
    return;

  QgsVectorLayer *vlayer = vectorLayer( item );
  QgsRasterLayer *rlayer = rasterLayer( item );
  if ( vlayer == 0 && rlayer == 0 )
  {
    QgsDebugMsg( "Item does not belong to a layer." );
    return;
  }

  if ( mActionPopup )
    delete mActionPopup;

  mActionPopup = new QMenu();

  int idx = -1;
  // QTreeWidgetItem *featItem = featureItem( item );
  QgsIdentifyResultsFeatureItem *featItem = dynamic_cast<QgsIdentifyResultsFeatureItem *>( featureItem( item ) );
  if ( featItem )
  {
    if ( vlayer )
    {
      mActionPopup->addAction(
        QgsApplication::getThemeIcon( vlayer->isEditable() ? "/mIconEditable.png" : "/mIconEditable.png" ),
        vlayer->isEditable() ? tr( "Edit feature form" ) : tr( "View feature form" ),
        this, SLOT( featureForm() ) );
    }

    if ( featItem->feature().isValid() )
    {
      mActionPopup->addAction( tr( "Zoom to feature" ), this, SLOT( zoomToFeature() ) );
      mActionPopup->addAction( tr( "Copy feature" ), this, SLOT( copyFeature() ) );
    }

    mActionPopup->addAction( tr( "Copy attribute value" ), this, SLOT( copyAttributeValue() ) );
    mActionPopup->addAction( tr( "Copy feature attributes" ), this, SLOT( copyFeatureAttributes() ) );

    if ( item->parent() == featItem && item->childCount() == 0 )
    {
      idx = item->data( 0, Qt::UserRole + 1 ).toInt();
    }
  }

  if ( rlayer )
  {
    QTreeWidgetItem *layItem = layerItem( item );
    if ( layItem && !layItem->data( 0, GetFeatureInfoUrlRole ).toString().isEmpty() )
    {
      mActionPopup->addAction( tr( "Copy GetFeatureInfo request URL" ), this, SLOT( copyGetFeatureInfoUrl() ) );
    }
  }
  if ( mActionPopup->children().size() > 0 )
  {
    mActionPopup->addSeparator();
  }

  mActionPopup->addAction( tr( "Clear results" ), this, SLOT( clear() ) );
  mActionPopup->addAction( tr( "Clear highlights" ), this, SLOT( clearHighlights() ) );
  mActionPopup->addAction( tr( "Highlight all" ), this, SLOT( highlightAll() ) );
  mActionPopup->addAction( tr( "Highlight layer" ), this, SLOT( highlightLayer() ) );
  mActionPopup->addAction( tr( "Layer properties..." ), this, SLOT( layerProperties() ) );
  mActionPopup->addSeparator();
  mActionPopup->addAction( tr( "Expand all" ), this, SLOT( expandAll() ) );
  mActionPopup->addAction( tr( "Collapse all" ), this, SLOT( collapseAll() ) );
  mActionPopup->addSeparator();

  if ( featItem && vlayer && vlayer->actions()->size() > 0 )
  {
    mActionPopup->addSeparator();

    int featIdx = featItem->data( 0, Qt::UserRole + 1 ).toInt();

    // The assumption is made that an instance of QgsIdentifyResults is
    // created for each new Identify Results dialog box, and that the
    // contents of the popup menu doesn't change during the time that
    // such a dialog box is around.
    for ( int i = 0; i < vlayer->actions()->size(); i++ )
    {
      const QgsAction &action = vlayer->actions()->at( i );

      if ( !action.runable() )
        continue;

      QgsFeatureAction *a = new QgsFeatureAction( action.name(), mFeatures[ featIdx ], vlayer, i, idx, this );
      mActionPopup->addAction( QgsApplication::getThemeIcon( "/mAction.svg" ), action.name(), a, SLOT( execute() ) );
    }
  }

  mActionPopup->popup( event->globalPos() );
}

// Save the current window location (store in ~/.qt/qgisrc)
void QgsIdentifyResultsDialog::saveWindowLocation()
{
  QSettings settings;
  settings.setValue( "/Windows/Identify/geometry", saveGeometry() );
  // first column width
  settings.setValue( "/Windows/Identify/columnWidth", lstResults->columnWidth( 0 ) );
}

void QgsIdentifyResultsDialog::setColumnText( int column, const QString & label )
{
  QTreeWidgetItem* header = lstResults->headerItem();
  header->setText( column, label );
}

void QgsIdentifyResultsDialog::expandColumnsToFit()
{
  lstResults->resizeColumnToContents( 0 );
  lstResults->resizeColumnToContents( 1 );
}

void QgsIdentifyResultsDialog::clearHighlights()
{
  foreach ( QgsHighlight *h, mHighlights )
  {
    delete h;
  }

  mHighlights.clear();
}

void QgsIdentifyResultsDialog::clear()
{
  for ( int i = 0; i < lstResults->topLevelItemCount(); i++ )
  {
    disconnectLayer( lstResults->topLevelItem( i )->data( 0, Qt::UserRole ).value<QObject *>() );
  }

  lstResults->clear();
  clearHighlights();

  // keep it visible but disabled, it can switch from disabled/enabled
  // after raster format change
  mPrintToolButton->setDisabled( true );
}

void QgsIdentifyResultsDialog::activate()
{
#if 0
  foreach ( QgsRubberBand *rb, mRubberBands )
  {
    rb->show();
  }
#endif

  if ( lstResults->topLevelItemCount() > 0 )
  {
    show();
    raise();
  }
}

void QgsIdentifyResultsDialog::deactivate()
{
#if 0
  foreach ( QgsRubberBand *rb, mRubberBands )
  {
    rb->hide();
  }
#endif
}

void QgsIdentifyResultsDialog::doAction( QTreeWidgetItem *item, int action )
{
  QTreeWidgetItem *featItem = featureItem( item );
  if ( !featItem )
    return;

  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( featItem->parent()->data( 0, Qt::UserRole ).value<QObject *>() );
  if ( !layer )
    return;

  int idx = -1;
  if ( item->parent() == featItem )
  {
    QString fieldName = item->data( 0, Qt::DisplayRole ).toString();

    const QgsFields& fields = layer->pendingFields();
    for ( int fldIdx = 0; fldIdx < fields.count(); ++fldIdx )
    {
      if ( fields[fldIdx].name() == fieldName )
      {
        idx = fldIdx;
        break;
      }
    }
  }

  int featIdx = featItem->data( 0, Qt::UserRole + 1 ).toInt();
  layer->actions()->doAction( action, mFeatures[ featIdx ], idx );
}

QTreeWidgetItem *QgsIdentifyResultsDialog::featureItem( QTreeWidgetItem *item )
{
  if ( !item )
    return 0;

  QTreeWidgetItem *featItem = 0;
  if ( item->parent() )
  {
    if ( item->parent()->parent() )
    {
      if ( item->parent()->parent()->parent() )
      {
        // derived or action attribute item
        featItem = item->parent()->parent();
      }
      else
      {
        // attribute item
        featItem = item->parent();
      }
    }
    else
    {
      // feature item
      featItem = item;
    }
  }
  else
  {
    // top level layer item, return feature item if only one
#if 0
    if ( item->childCount() > 1 )
      return 0;
    featItem = item->child( 0 );
#endif

    int count = 0;

    for ( int i = 0; i < item->childCount(); i++ )
    {
      QgsIdentifyResultsFeatureItem *fi = dynamic_cast<QgsIdentifyResultsFeatureItem *>( item->child( i ) );
      if ( fi )
      {
        count++;
        if ( !featItem )
          featItem = fi;
      }
    }

    if ( count != 1 )
      return 0;
  }

  return featItem;
}

QTreeWidgetItem *QgsIdentifyResultsDialog::layerItem( QTreeWidgetItem *item )
{
  if ( item && item->parent() )
  {
    item = featureItem( item )->parent();
  }

  return item;
}


QgsVectorLayer *QgsIdentifyResultsDialog::vectorLayer( QTreeWidgetItem *item )
{
  item = layerItem( item );
  if ( !item )
    return NULL;
  return qobject_cast<QgsVectorLayer *>( item->data( 0, Qt::UserRole ).value<QObject *>() );
}

QgsRasterLayer *QgsIdentifyResultsDialog::rasterLayer( QTreeWidgetItem *item )
{
  item = layerItem( item );
  if ( !item )
    return NULL;
  return qobject_cast<QgsRasterLayer *>( item->data( 0, Qt::UserRole ).value<QObject *>() );
}

QTreeWidgetItem *QgsIdentifyResultsDialog::retrieveAttributes( QTreeWidgetItem *item, QgsAttributeMap &attributes, int &idx )
{
  QTreeWidgetItem *featItem = featureItem( item );
  if ( !featItem )
    return 0;

  idx = -1;

  attributes.clear();
  for ( int i = 0; i < featItem->childCount(); i++ )
  {
    QTreeWidgetItem *item = featItem->child( i );
    if ( item->childCount() > 0 )
      continue;
    if ( item == lstResults->currentItem() )
      idx = item->data( 0, Qt::UserRole + 1 ).toInt();
    attributes.insert( item->data( 0, Qt::UserRole + 1 ).toInt(), item->data( 1, Qt::DisplayRole ) );
  }

  return featItem;
}

void QgsIdentifyResultsDialog::itemExpanded( QTreeWidgetItem *item )
{
  Q_UNUSED( item );
  // column width is now stored in settings
  //expandColumnsToFit();
}

void QgsIdentifyResultsDialog::handleCurrentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous )
{
  Q_UNUSED( previous );

  mPrintToolButton->setEnabled( false );

  QgsIdentifyResultsFeatureItem *featItem = dynamic_cast<QgsIdentifyResultsFeatureItem *>( featureItem( current ) );
  mCopyToolButton->setEnabled( featItem && featItem->feature().isValid() );

  if ( !current )
  {
    emit selectedFeatureChanged( 0, 0 );
    return;
  }

  // An item may be printed if a child is QgsIdentifyResultsWebViewItem
  for ( int i = 0; i < current->childCount(); i++ )
  {
    QgsIdentifyResultsWebViewItem *wv = dynamic_cast<QgsIdentifyResultsWebViewItem*>( current->child( i ) );
    if ( wv != 0 )
    {
      mPrintToolButton->setEnabled( true );
      break;
    }
  }

  QTreeWidgetItem *layItem = layerItem( current );
  if ( current == layItem )
  {
    highlightLayer( layItem );
  }
  else
  {
    clearHighlights();
    highlightFeature( current );
  }
}

void QgsIdentifyResultsDialog::layerDestroyed()
{
  QObject *theSender = sender();

  for ( int i = 0; i < lstResults->topLevelItemCount(); i++ )
  {
    QTreeWidgetItem *layItem = lstResults->topLevelItem( i );

    if ( layItem->data( 0, Qt::UserRole ).value<QObject *>() == sender() )
    {
      for ( int j = 0; j < layItem->childCount(); j++ )
      {
        delete mHighlights.take( layItem->child( i ) );
      }
    }
  }

  disconnectLayer( theSender );
  delete layerItem( theSender );

  if ( lstResults->topLevelItemCount() == 0 )
  {
    close();
  }
}

void QgsIdentifyResultsDialog::disconnectLayer( QObject *layer )
{
  if ( !layer )
    return;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( vlayer )
  {
    disconnect( vlayer, SIGNAL( layerDeleted() ), this, SLOT( layerDestroyed() ) );
    disconnect( vlayer, SIGNAL( featureDeleted( QgsFeatureId ) ), this, SLOT( featureDeleted( QgsFeatureId ) ) );
    disconnect( vlayer, SIGNAL( attributeValueChanged( QgsFeatureId, int, const QVariant & ) ),
                this,   SLOT( attributeValueChanged( QgsFeatureId, int, const QVariant & ) ) );
    disconnect( vlayer, SIGNAL( editingStarted() ), this, SLOT( editingToggled() ) );
    disconnect( vlayer, SIGNAL( editingStopped() ), this, SLOT( editingToggled() ) );
  }
  else
  {
    disconnect( layer, SIGNAL( destroyed() ), this, SLOT( layerDestroyed() ) );
  }
}

void QgsIdentifyResultsDialog::featureDeleted( QgsFeatureId fid )
{
  QTreeWidgetItem *layItem = layerItem( sender() );

  if ( !layItem )
    return;

  for ( int i = 0; i < layItem->childCount(); i++ )
  {
    QTreeWidgetItem *featItem = layItem->child( i );

    if ( featItem && STRING_TO_FID( featItem->data( 0, Qt::UserRole ) ) == fid )
    {
      delete mHighlights.take( featItem );
      delete featItem;
      break;
    }
  }

  if ( layItem->childCount() == 0 )
  {
    delete layItem;
  }

  if ( lstResults->topLevelItemCount() == 0 )
  {
    close();
  }
}

void QgsIdentifyResultsDialog::attributeValueChanged( QgsFeatureId fid, int idx, const QVariant &val )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( sender() );
  QTreeWidgetItem *layItem = layerItem( sender() );

  if ( !layItem )
    return;

  for ( int i = 0; i < layItem->childCount(); i++ )
  {
    QTreeWidgetItem *featItem = layItem->child( i );

    if ( featItem && STRING_TO_FID( featItem->data( 0, Qt::UserRole ) ) == fid )
    {
      if ( featItem->data( 0, Qt::DisplayRole ).toString() == vlayer->displayField() )
        featItem->setData( 1, Qt::DisplayRole, val );

      for ( int j = 0; j < featItem->childCount(); j++ )
      {
        QTreeWidgetItem *item = featItem->child( j );
        if ( item->childCount() > 0 )
          continue;

        if ( item->data( 0, Qt::UserRole + 1 ).toInt() == idx )
        {
          item->setData( 1, Qt::DisplayRole, val );
          return;
        }
      }
    }
  }
}

void QgsIdentifyResultsDialog::highlightFeature( QTreeWidgetItem *item )
{
  QgsVectorLayer *layer = vectorLayer( item );
  QgsRasterLayer *rlayer = rasterLayer( item );
  if ( !layer && !rlayer )
    return;

  QgsIdentifyResultsFeatureItem *featItem = dynamic_cast<QgsIdentifyResultsFeatureItem *>( featureItem( item ) );
  if ( !featItem )
    return;


  if ( mHighlights.contains( featItem ) )
    return;

  if ( !featItem->feature().geometry() || featItem->feature().geometry()->wkbType() == QGis::WKBUnknown )
    return;

  QgsHighlight *h = new QgsHighlight( mCanvas, featItem->feature().geometry(), layer );
  if ( h )
  {
    h->setWidth( 2 );
    h->setColor( Qt::red );
    h->show();
    mHighlights.insert( featItem, h );
  }
}

void QgsIdentifyResultsDialog::zoomToFeature()
{
  QTreeWidgetItem *item = lstResults->currentItem();

  QgsMapLayer *layer;
  QgsVectorLayer *vlayer = vectorLayer( item );
  QgsRasterLayer *rlayer = rasterLayer( item );
  if ( !vlayer && !rlayer )
    return;

  layer = vlayer ? ( QgsMapLayer * )vlayer : ( QgsMapLayer * )rlayer;

  QgsIdentifyResultsFeatureItem *featItem = dynamic_cast<QgsIdentifyResultsFeatureItem *>( featureItem( item ) );
  if ( !featItem )
  {
    return;
  }

  QgsFeature feat = featItem->feature();
  if ( !feat.geometry() )
  {
    return;
  }

  // TODO: verify CRS for raster WMS features
  QgsRectangle rect = mCanvas->mapRenderer()->layerExtentToOutputExtent( layer, feat.geometry()->boundingBox() );

  if ( rect.isEmpty() )
  {
    QgsPoint c = rect.center();
    rect = mCanvas->extent();
    rect.scale( 0.5, &c );
  }

  mCanvas->setExtent( rect );
  mCanvas->refresh();
}

void QgsIdentifyResultsDialog::featureForm()
{
  QTreeWidgetItem *item = lstResults->currentItem();

  QgsVectorLayer *vlayer = vectorLayer( item );
  if ( !vlayer )
    return;

  QTreeWidgetItem *featItem = featureItem( item );
  if ( !featItem )
    return;

  int fid = STRING_TO_FID( featItem->data( 0, Qt::UserRole ) );
  int idx = featItem->data( 0, Qt::UserRole + 1 ).toInt();

  QgsFeature f;
  if ( !vlayer->getFeatures( QgsFeatureRequest().setFilterFid( fid ) ).nextFeature( f ) )
    return;

  QgsFeatureAction action( tr( "Attribute changes" ), f, vlayer, idx, -1, this );
  if ( vlayer->isEditable() )
  {
    if ( action.editFeature() )
    {
      mCanvas->refresh();
    }
  }
  else
  {
    action.viewFeatureForm( mHighlights.take( featItem ) );
  }
}

void QgsIdentifyResultsDialog::highlightAll()
{
  for ( int i = 0; i < lstResults->topLevelItemCount(); i++ )
  {
    QTreeWidgetItem *layItem = lstResults->topLevelItem( i );

    for ( int j = 0; j < layItem->childCount(); j++ )
    {
      highlightFeature( layItem->child( j ) );
    }
  }
}

void QgsIdentifyResultsDialog::highlightLayer()
{
  highlightLayer( lstResults->currentItem() );
}

void QgsIdentifyResultsDialog::highlightLayer( QTreeWidgetItem *item )
{
  QTreeWidgetItem *layItem = layerItem( item );
  if ( !layItem )
    return;

  clearHighlights();

  for ( int i = 0; i < layItem->childCount(); i++ )
  {
    highlightFeature( layItem->child( i ) );
  }
}

void QgsIdentifyResultsDialog::layerProperties()
{
  layerProperties( lstResults->currentItem() );
}

void QgsIdentifyResultsDialog::layerProperties( QTreeWidgetItem *item )
{
  QgsVectorLayer *vlayer = vectorLayer( item );
  if ( !vlayer )
    return;

  QgisApp::instance()->showLayerProperties( vlayer );
}

void QgsIdentifyResultsDialog::expandAll()
{
  lstResults->expandAll();
}

void QgsIdentifyResultsDialog::collapseAll()
{
  lstResults->collapseAll();
}

void QgsIdentifyResultsDialog::copyAttributeValue()
{
  QClipboard *clipboard = QApplication::clipboard();
  QString text = lstResults->currentItem()->data( 1, Qt::DisplayRole ).toString();
  QgsDebugMsg( QString( "set clipboard: %1" ).arg( text ) );
  clipboard->setText( text );
}

void QgsIdentifyResultsDialog::copyFeatureAttributes()
{
  QgsDebugMsg( "Entered" );
  QClipboard *clipboard = QApplication::clipboard();
  QString text;

  QgsVectorLayer *vlayer = vectorLayer( lstResults->currentItem() );
  QgsRasterLayer *rlayer = rasterLayer( lstResults->currentItem() );
  if ( !vlayer & !rlayer )
  {
    return;
  }
  if ( vlayer )
  {
    int idx;
    QgsAttributeMap attributes;
    retrieveAttributes( lstResults->currentItem(), attributes, idx );

    const QgsFields &fields = vlayer->pendingFields();

    for ( QgsAttributeMap::const_iterator it = attributes.begin(); it != attributes.end(); it++ )
    {
      int attrIdx = it.key();
      if ( attrIdx < 0 || attrIdx >= fields.count() )
        continue;

      text += QString( "%1: %2\n" ).arg( fields[attrIdx].name() ).arg( it.value().toString() );
    }
  }
  else if ( rlayer )
  {
    QTreeWidgetItem *featItem = featureItem( lstResults->currentItem() );
    if ( !featItem )
      return;

    for ( int i = 0; i < featItem->childCount(); i++ )
    {
      QTreeWidgetItem *item = featItem->child( i );
      if ( item->childCount() > 0 )
        continue;
      text += QString( "%1: %2\n" ).arg( item->data( 0, Qt::DisplayRole ).toString() ).arg( item->data( 1, Qt::DisplayRole ).toString() );
    }
  }

  QgsDebugMsg( QString( "set clipboard: %1" ).arg( text ) );
  clipboard->setText( text );
}

void QgsIdentifyResultsDialog::copyGetFeatureInfoUrl()
{
  QClipboard *clipboard = QApplication::clipboard();
  QTreeWidgetItem *item = lstResults->currentItem();
  QTreeWidgetItem *layItem = layerItem( item );
  if ( !layItem )
    return;
  clipboard->setText( layItem->data( 0, GetFeatureInfoUrlRole ).toString() );
}

void QgsIdentifyResultsDialog::printCurrentItem()
{
  QTreeWidgetItem *item = lstResults->currentItem();
  if ( !item )
    return;

  // There should only be one HTML item / result
  QgsIdentifyResultsWebViewItem *wv = 0;
  for ( int i = 0; i < item->childCount() && !wv; i++ )
  {
    wv = dynamic_cast<QgsIdentifyResultsWebViewItem*>( item->child( i ) );
  }

  if ( !wv )
  {
    QMessageBox::warning( this, tr( "Cannot not print" ), tr( "Cannot print this item" ) );
    return;
  }

  wv->webView()->print();
}

void QgsIdentifyResultsDialog::on_mExpandNewToolButton_toggled( bool checked )
{
  QSettings settings;
  settings.setValue( "/Map/identifyExpand", checked );
}

void QgsIdentifyResultsDialog::on_mCopyToolButton_clicked( bool checked )
{
  Q_UNUSED( checked );
  copyFeature();
}

void QgsIdentifyResultsDialog::copyFeature()
{
  QgsDebugMsg( "Entered" );

  QgsIdentifyResultsFeatureItem *item = dynamic_cast<QgsIdentifyResultsFeatureItem *>( featureItem( lstResults->selectedItems().value( 0 ) ) );

  if ( !item ) // should not happen
  {
    QgsDebugMsg( "Selected item is not feature" );
    return;
  }

  QgsFeatureStore featureStore( item->fields(), item->crs() );
  featureStore.features().append( item->feature() );
  emit copyToClipboard( featureStore );
}

void QgsIdentifyResultsDialog::formatChanged( int index )
{
  QgsDebugMsg( "Entered" );
  QComboBox *combo = qobject_cast<QComboBox*>( sender() );
  if ( !combo )
  {
    QgsDebugMsg( "sender is not QComboBox" );
    return;
  }

  QgsRaster::IdentifyFormat format = ( QgsRaster::IdentifyFormat ) combo->itemData( index, Qt::UserRole ).toInt();
  QgsDebugMsg( QString( "format = %1" ).arg( format ) );
  QgsRasterLayer *layer = qobject_cast<QgsRasterLayer *>( combo->itemData( index, Qt::UserRole + 1 ).value<QObject *>() );
  if ( !layer )
  {
    QgsDebugMsg( "cannot get raster layer" );
    return;
  }

  // Store selected identify format in layer
  layer->setCustomProperty( "identify/format", QgsRasterDataProvider::identifyFormatName( format ) );

  // remove all childs of that layer from results, except the first (format)
  QTreeWidgetItem *layItem = layerItem( layer );
  if ( !layItem )
  {
    QgsDebugMsg( "cannot get layer item" );
    return;
  }

  for ( int i = layItem->childCount() - 1; i > 0; i-- )
  {
    QTreeWidgetItem *child = layItem->child( i );
    QgsDebugMsg( QString( "remove %1:0x%2" ).arg( i ).arg(( qint64 ) child, 0, 16 ) );
    layItem->removeChild( child );
    QgsDebugMsg( QString( "removed %1:0x%2" ).arg( i ).arg(( qint64 ) child, 0, 16 ) );
  }

  // let know QgsMapToolIdentify that format changed
  emit formatChanged( layer );

  // Expand
  layItem->setExpanded( true );
  for ( int i = 1; i < layItem->childCount(); i++ )
  {
    QTreeWidgetItem *subItem = layItem->child( i );
    subItem->setExpanded( true );
    for ( int j = 0; j < subItem->childCount(); j++ )
    {
      subItem->child( j )->setExpanded( true );
    }
  }
}
