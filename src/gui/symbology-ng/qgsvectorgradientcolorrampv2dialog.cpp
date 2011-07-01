
#include "qgsvectorgradientcolorrampv2dialog.h"

#include "qgsvectorcolorrampv2.h"

#include <QColorDialog>
#include <QInputDialog>
#include <QPainter>

QgsVectorGradientColorRampV2Dialog::QgsVectorGradientColorRampV2Dialog( QgsVectorGradientColorRampV2* ramp, QWidget* parent )
    : QDialog( parent ), mRamp( ramp )
{

  setupUi( this );

  connect( btnColor1, SIGNAL( clicked() ), this, SLOT( setColor1() ) );
  connect( btnColor2, SIGNAL( clicked() ), this, SLOT( setColor2() ) );

  // handle stops
  QgsVectorGradientColorRampV2::StopsMap stops = ramp->stops();
  groupStops->setChecked( !stops.isEmpty() );

  QgsVectorGradientColorRampV2::StopsMap::iterator i;
  QList<QTreeWidgetItem *> items;
  for ( i = stops.begin(); i != stops.end(); ++i )
  {
    QStringList lst;
    lst << "." << QString::number( i.key()*100, 'f', 0 );
    QTreeWidgetItem* item = new QTreeWidgetItem( lst );

    setStopColor( item, i.value() );
    item->setData( 0, StopOffsetRole, i.key() );

    items.append( item );
  }
  treeStops->insertTopLevelItems( 0, items );
  treeStops->resizeColumnToContents( 0 );
  treeStops->sortByColumn( 1, Qt::AscendingOrder );
  treeStops->setSortingEnabled( true );

  connect( groupStops, SIGNAL( toggled( bool ) ), this, SLOT( toggledStops( bool ) ) );
  connect( btnAddStop, SIGNAL( clicked() ), this, SLOT( addStop() ) );
  connect( btnRemoveStop, SIGNAL( clicked() ), this, SLOT( removeStop() ) );
  connect( treeStops, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ), this, SLOT( stopDoubleClicked( QTreeWidgetItem*, int ) ) );

  updatePreview();
}

void QgsVectorGradientColorRampV2Dialog::updatePreview()
{
  // update ramp stops from the tree widget
  QgsVectorGradientColorRampV2::StopsMap map;
  if ( groupStops->isChecked() )
  {
    int count = treeStops->topLevelItemCount();
    for ( int i = 0; i < count; i++ )
    {
      QTreeWidgetItem* item = treeStops->topLevelItem( i );
      double key = item->data( 0, StopOffsetRole ).toDouble();
      QColor c = item->data( 0, StopColorRole ).value<QColor>();
      map.insert( key, c );
    }
  }
  mRamp->setStops( map );

  // generate the preview
  QSize size( 300, 40 );
  lblPreview->setPixmap( QgsSymbolLayerV2Utils::colorRampPreviewPixmap( mRamp, size ) );

  btnColor1->setColor( mRamp->color1() );
  btnColor2->setColor( mRamp->color2() );
}

void QgsVectorGradientColorRampV2Dialog::setColor1()
{
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  // Qt bug: http://bugreports.qt.nokia.com/browse/QTBUG-14889
  // FIXME need to also check max QT_VERSION when Qt bug fixed
  QColor color = QColorDialog::getColor( mRamp->color1(), this, "", QColorDialog::DontUseNativeDialog );
#else
  QColor color = QColorDialog::getColor( mRamp->color1(), this );
#endif
  if ( !color.isValid() )
    return;
  mRamp->setColor1( color );
  updatePreview();
}

void QgsVectorGradientColorRampV2Dialog::setColor2()
{
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  // Qt bug: http://bugreports.qt.nokia.com/browse/QTBUG-14889
  // FIXME need to also check max QT_VERSION when Qt bug fixed
  QColor color = QColorDialog::getColor( mRamp->color2(), this, "", QColorDialog::DontUseNativeDialog );
#else
  QColor color = QColorDialog::getColor( mRamp->color2(), this );
#endif
  if ( !color.isValid() )
    return;
  mRamp->setColor2( color );
  updatePreview();
}

void QgsVectorGradientColorRampV2Dialog::setStopColor( QTreeWidgetItem* item, QColor color )
{
  QSize iconSize( 16, 16 );
  QPixmap pixmap( iconSize );
  pixmap.fill( QColor( 0, 0, 0, 0 ) );
  QRect rect( 1, 1, iconSize.width() - 2, iconSize.height() - 2 );

  // draw a slightly rounded rectangle
  QPainter p;
  p.begin( &pixmap );
  p.setPen( Qt::NoPen );
  p.setRenderHint( QPainter::Antialiasing );
  p.setBrush( color );
  p.drawRoundedRect( rect, 4, 4 );
  p.end();

  item->setIcon( 0, QIcon( pixmap ) );
  item->setData( 0, StopColorRole, color );
  item->setText( 0, color.name() );
}

void QgsVectorGradientColorRampV2Dialog::stopDoubleClicked( QTreeWidgetItem* item, int column )
{
  if ( column == 0 )
  {
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
    // Native Mac dialog works only for Qt Carbon
    // Qt bug: http://bugreports.qt.nokia.com/browse/QTBUG-14889
    // FIXME need to also check max QT_VERSION when Qt bug fixed
    QColor color = QColorDialog::getColor( item->data( 0, StopColorRole ).value<QColor>(), this, "", QColorDialog::DontUseNativeDialog );
#else
    QColor color = QColorDialog::getColor( item->data( 0, StopColorRole ).value<QColor>(), this );
#endif
    if ( !color.isValid() )
      return;
    setStopColor( item, color );

    updatePreview();
  }
  else
  {
    bool ok;
    double key = item->data( 0, StopOffsetRole ).toDouble();
    int val = ( int )( key * 100 );
#if QT_VERSION >= 0x40500
    val = QInputDialog::getInt( this, tr( "Offset of the stop" ),
                                tr( "Please enter offset in percents (%) of the new stop" ),
                                val, 0, 100, 1, &ok );
#else
    QString res = QInputDialog::getText( this, tr( "Offset of the stop" ),
                                         tr( "Please enter offset in percents (%) of the new stop" ),
                                         QLineEdit::Normal, QString::number( val ), &ok );
    if ( ok )
      val = res.toInt( &ok );
    if ( ok )
      ok = val >= 0 && val <= 100;
#endif
    if ( !ok )
      return;

    double newkey = val / 100.0;
    item->setText( 1, QString::number( val ) );

    item->setData( 0, StopOffsetRole, newkey );

    updatePreview();
  }
}

void QgsVectorGradientColorRampV2Dialog::addStop()
{
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  // Qt bug: http://bugreports.qt.nokia.com/browse/QTBUG-14889
  // also Qt 4.7 Mac Cocoa bug: calling QInputDialog::getInt after QColorDialog::getColor will freeze app
  // workaround: call QColorDialog::getColor below instead of here,
  // but not needed at this time because of the other Qt bug
  // FIXME need to also check max QT_VERSION when Qt bug(s) fixed
  QColor color = QColorDialog::getColor( QColor(), this, "", QColorDialog::DontUseNativeDialog );
#else
  QColor color = QColorDialog::getColor( QColor(), this );
#endif
  if ( !color.isValid() )
    return;

  bool ok;
  int val = 50;
#if QT_VERSION >= 0x40500
  val = QInputDialog::getInt( this, tr( "Offset of the stop" ),
                              tr( "Please enter offset in percents (%) of the new stop" ),
                              val, 0, 100, 1, &ok );
#else
  QString res = QInputDialog::getText( this, tr( "Offset of the stop" ),
                                       tr( "Please enter offset in percents (%) of the new stop" ),
                                       QLineEdit::Normal, QString::number( val ), &ok );
  if ( ok )
    val = res.toInt( &ok );
  if ( ok )
    ok = val >= 0 && val <= 100;
#endif
  if ( !ok )
    return;

  double key = val / 100.0;
  QStringList lst;
  lst << "." << QString::number( val, 'f', 0 );
  QTreeWidgetItem* item = new QTreeWidgetItem( lst );

  setStopColor( item, color );
  item->setData( 0, StopOffsetRole, key );

  treeStops->addTopLevelItem( item );

  treeStops->resizeColumnToContents( 0 );

  updatePreview();
}

void QgsVectorGradientColorRampV2Dialog::removeStop()
{
  QTreeWidgetItem* item = treeStops->currentItem();
  if ( !item )
    return;
  int index = treeStops->indexOfTopLevelItem( item );
  treeStops->takeTopLevelItem( index );

  updatePreview();
}

void QgsVectorGradientColorRampV2Dialog::toggledStops( bool on )
{
  Q_UNUSED( on );
  updatePreview();
}
