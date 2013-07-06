/***************************************************************************
                         qgsattributeeditor.cpp  -  description
                             -------------------
    begin                : July 2009
    copyright            : (C) 2009 by Jürgen E. Fischer
    email                : jef@norbit.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributeeditor.h"
#include <qgsvectorlayer.h>
#include <qgsvectordataprovider.h>
#include <qgscategorizedsymbolrendererv2.h>
#include <qgslonglongvalidator.h>
#include <qgsfieldvalidator.h>
#include <qgsmaplayerregistry.h>
#include <qgslogger.h>
#include <qgsexpression.h>
#include <qgsfilterlineedit.h>
#include <qgscolorbutton.h>
#include <qgsnetworkaccessmanager.h>

#include <QScrollArea>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QFileDialog>
#include <QComboBox>
#include <QListWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QCompleter>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QDial>
#include <QCalendarWidget>
#include <QDialogButtonBox>
#include <QSettings>
#include <QDir>
#include <QUuid>
#include <QGroupBox>
#include <QLabel>
#include <QWebView>
#include <QDesktopServices>

void QgsAttributeEditor::selectFileName()
{
  QPushButton *pb = qobject_cast<QPushButton *>( sender() );
  if ( !pb )
    return;

  QWidget *hbox = qobject_cast<QWidget *>( pb->parent() );
  if ( !hbox )
    return;

  QLineEdit *le = hbox->findChild<QLineEdit *>();
  if ( !le )
    return;

  QString fileName = QFileDialog::getOpenFileName( 0 , tr( "Select a file" ), QFileInfo( le->text() ).absolutePath() );
  if ( fileName.isNull() )
    return;

  le->setText( QDir::toNativeSeparators( fileName ) );
}

void QgsAttributeEditor::selectDate()
{
  QPushButton *pb = qobject_cast<QPushButton *>( sender() );
  if ( !pb )
    return;

  QWidget *hbox = qobject_cast<QWidget *>( pb->parent() );
  if ( !hbox )
    return;

  QLineEdit *le = hbox->findChild<QLineEdit *>();
  if ( !le )
    return;

  QDialog *dlg = new QDialog();
  dlg->setWindowTitle( tr( "Select a date" ) );
  QVBoxLayout *vl = new QVBoxLayout( dlg );

  const QgsFieldValidator *v = dynamic_cast<const QgsFieldValidator *>( le->validator() );
  QString dateFormat = v ? v->dateFormat() : "yyyy-MM-dd";

  QCalendarWidget *cw = new QCalendarWidget( dlg );
  QString prevValue = le->text();
  cw->setSelectedDate( QDate::fromString( prevValue, dateFormat ) );
  vl->addWidget( cw );

  QDialogButtonBox *buttonBox = new QDialogButtonBox( dlg );
  buttonBox->addButton( QDialogButtonBox::Ok );
  buttonBox->addButton( QDialogButtonBox::Cancel );
  vl->addWidget( buttonBox );

  connect( buttonBox, SIGNAL( accepted() ), dlg, SLOT( accept() ) );
  connect( buttonBox, SIGNAL( rejected() ), dlg, SLOT( reject() ) );

  if ( dlg->exec() == QDialog::Accepted )
  {
    QString newValue = cw->selectedDate().toString( dateFormat );
    le->setText( newValue );
    le->setModified( newValue != prevValue );
  }
}

void QgsAttributeEditor::loadUrl( const QString &url )
{
  QLineEdit *le = qobject_cast<QLineEdit *>( sender() );
  if ( !le )
    return;

  QWidget *hbox = qobject_cast<QWidget *>( le->parent() );
  if ( !hbox )
    return;

  QWebView *ww = hbox->findChild<QWebView *>();
  if ( !ww )
    return;

  ww->load( url );
}

void QgsAttributeEditor::loadPixmap( const QString &name )
{
  QLineEdit *le = qobject_cast<QLineEdit *>( sender() );
  if ( !le )
    return;

  QWidget *hbox = qobject_cast<QWidget *>( le->parent() );
  if ( !hbox )
    return;

  QLabel *lw = hbox->findChild<QLabel *>();
  if ( !lw )
    return;

  QPixmap pm( name );
  if ( pm.isNull() )
    return;

  QSize size( mLayer->widgetSize( mIdx ) );
  if ( size.width() == 0 && size.height() > 0 )
  {
    size.setWidth( size.height() * pm.size().width() / pm.size().height() );
  }
  else if ( size.width() > 0 && size.height() == 0 )
  {
    size.setHeight( size.width() * pm.size().height() / pm.size().width() );
  }

  pm = pm.scaled( size, Qt::KeepAspectRatio, Qt::SmoothTransformation );

  lw->setPixmap( pm );
  lw->setMinimumSize( size );
}

void QgsAttributeEditor::updateUrl()
{
  QPushButton *pb = qobject_cast<QPushButton *>( sender() );
  if ( !pb )
    return;

  QWidget *hbox = qobject_cast<QWidget *>( pb->parent() );
  if ( !hbox )
    return;

  QWebView *ww = hbox->findChild<QWebView *>();
  if ( !ww )
    return;

  QLineEdit *le = hbox->findChild<QLineEdit *>();
  if ( !le )
    return;

  le->blockSignals( true );
  le->setText( ww->url().toString() );
  le->blockSignals( false );
}

void QgsAttributeEditor::openUrl()
{
  QPushButton *pb = qobject_cast<QPushButton *>( sender() );
  if ( !pb )
    return;

  QWidget *hbox = qobject_cast<QWidget *>( pb->parent() );
  if ( !hbox )
    return;

  QWebView *ww = hbox->findChild<QWebView *>();
  if ( !ww )
    return;

  QDesktopServices::openUrl( ww->url().toString() );
}

void QgsAttributeEditor::updateColor()
{
  QString color;
  QgsColorButton *scb = qobject_cast<QgsColorButton *>( sender() );
  QLineEdit *sle = qobject_cast<QLineEdit *>( sender() );

  if ( !scb && !sle )
    return;

  QWidget *hbox = qobject_cast<QWidget *>( sender()->parent() );
  if ( !hbox )
    return;

  QgsColorButton *cb = hbox->findChild<QgsColorButton *>();
  if ( !cb )
    return;

  QLineEdit *le = hbox->findChild<QLineEdit *>();
  if ( !le )
    return;

  if ( scb )
  {
    le->blockSignals( true );
    le->setText( scb->color().name() );
    le->blockSignals( false );
  }

  if ( sle )
  {
    cb->blockSignals( true );
    cb->setColor( QColor( sle->text() ) );
    cb->blockSignals( false );
  }
}

QComboBox *QgsAttributeEditor::comboBox( QWidget *editor, QWidget *parent )
{
  QComboBox *cb = 0;
  if ( editor )
    cb = qobject_cast<QComboBox *>( editor );
  else
    cb = new QComboBox( parent );

  return cb;
}

QListWidget *QgsAttributeEditor::listWidget( QWidget *editor, QWidget *parent )
{
  QListWidget *lw = 0;
  if ( editor )
    lw = qobject_cast<QListWidget *>( editor );
  else
    lw = new QListWidget( parent );

  return lw;
}

QWidget *QgsAttributeEditor::createAttributeEditor( QWidget *parent, QWidget *editor, QgsVectorLayer *vl, int idx, const QVariant &value )
{
  QMap<int, QWidget*> dummyProxyWidgets;
  return createAttributeEditor( parent, editor, vl, idx, value, dummyProxyWidgets );
}

QWidget *QgsAttributeEditor::createAttributeEditor( QWidget *parent, QWidget *editor, QgsVectorLayer *vl, int idx, const QVariant &value, QMap<int, QWidget*> &proxyWidgets )
{
  if ( !vl )
    return 0;

  QWidget *myWidget = 0;
  QgsVectorLayer::EditType editType = vl->editType( idx );
  const QgsField &field = vl->pendingFields()[idx];
  QVariant::Type myFieldType = field.type();

  bool synchronized = false;

  switch ( editType )
  {
    case QgsVectorLayer::UniqueValues:
    {
      QList<QVariant> values;
      vl->dataProvider()->uniqueValues( idx, values );

      QComboBox *cb = comboBox( editor, parent );
      if ( cb )
      {
        cb->setEditable( false );

        for ( QList<QVariant>::iterator it = values.begin(); it != values.end(); it++ )
          cb->addItem( it->toString(), it->toString() );

        myWidget = cb;
      }

    }
    break;

    case QgsVectorLayer::Enumeration:
    {
      QStringList enumValues;
      vl->dataProvider()->enumValues( idx, enumValues );

      QComboBox *cb = comboBox( editor, parent );
      if ( cb )
      {
        QStringList::const_iterator s_it = enumValues.constBegin();
        for ( ; s_it != enumValues.constEnd(); ++s_it )
        {
          cb->addItem( *s_it, *s_it );
        }

        myWidget = cb;
      }
    }
    break;

    case QgsVectorLayer::ValueMap:
    {
      const QMap<QString, QVariant> &map = vl->valueMap( idx );

      QComboBox *cb = comboBox( editor, parent );
      if ( cb )
      {
        for ( QMap<QString, QVariant>::const_iterator it = map.begin(); it != map.end(); it++ )
        {
          cb->addItem( it.key(), it.value() );
        }

        myWidget = cb;
      }
    }
    break;

    case QgsVectorLayer::ValueRelation:
    {
      const QgsVectorLayer::ValueRelationData &data = vl->valueRelation( idx );

      QgsVectorLayer *layer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( data.mLayer ) );
      QMap< QString, QString > map;

      if ( layer )
      {
        int ki = layer->fieldNameIndex( data.mOrderByValue ? data.mValue : data.mKey );
        int vi = layer->fieldNameIndex( data.mOrderByValue ? data.mKey : data.mValue );

        QgsExpression *e = 0;
        if ( !data.mFilterExpression.isEmpty() )
        {
          e = new QgsExpression( data.mFilterExpression );
          if ( e->hasParserError() || !e->prepare( layer->pendingFields() ) )
            ki = -1;
        }

        if ( ki >= 0 && vi >= 0 )
        {
          QSet<int> attributes;
          attributes << ki << vi;

          QgsFeatureRequest::Flag flags = QgsFeatureRequest::NoGeometry;

          if ( e )
          {
            if ( e->needsGeometry() )
              flags = QgsFeatureRequest::NoFlags;

            foreach ( const QString &field, e->referencedColumns() )
            {
              int idx = layer->fieldNameIndex( field );
              if ( idx < 0 )
                continue;
              attributes << idx;
            }
          }

          QgsFeatureIterator fit = layer->getFeatures( QgsFeatureRequest().setFlags( flags ).setSubsetOfAttributes( attributes.toList() ) );
          QgsFeature f;
          while ( fit.nextFeature( f ) )
          {
            if ( e && !e->evaluate( &f ).toBool() )
              continue;

            map.insert( f.attribute( ki ).toString(), f.attribute( vi ).toString() );
          }
        }
      }

      if ( !data.mAllowMulti )
      {
        QComboBox *cb = comboBox( editor, parent );
        if ( cb )
        {
          if ( data.mAllowNull )
          {
            QSettings settings;
            cb->addItem( tr( "(no selection)" ), settings.value( "qgis/nullValue", "NULL" ).toString() );
          }

          for ( QMap< QString, QString >::const_iterator it = map.begin(); it != map.end(); it++ )
          {
            if ( data.mOrderByValue )
              cb->addItem( it.key(), it.value() );
            else
              cb->addItem( it.value(), it.key() );
          }

          myWidget = cb;
        }
      }
      else
      {
        QListWidget *lw = listWidget( editor, parent );
        if ( lw )
        {
          QStringList checkList = value.toString().remove( QChar( '{' ) ).remove( QChar( '}' ) ).split( "," );

          for ( QMap< QString, QString >::const_iterator it = map.begin(); it != map.end(); it++ )
          {
            QListWidgetItem *item;
            if ( data.mOrderByValue )
            {
              item = new QListWidgetItem( it.key() );
              item->setData( Qt::UserRole, it.value() );
              item->setCheckState( checkList.contains( it.value() ) ? Qt::Checked : Qt::Unchecked );
            }
            else
            {
              item = new QListWidgetItem( it.value() );
              item->setData( Qt::UserRole, it.key() );
              item->setCheckState( checkList.contains( it.key() ) ? Qt::Checked : Qt::Unchecked );
            }
            lw->addItem( item );
          }

          myWidget = lw;
        }
      }
    }
    break;

    case QgsVectorLayer::Classification:
    {
      QMap<QString, QString> classes;

      const QgsCategorizedSymbolRendererV2 *csr = dynamic_cast<const QgsCategorizedSymbolRendererV2 *>( vl->rendererV2() );
      if ( csr )
      {
        const QgsCategoryList &categories = (( QgsCategorizedSymbolRendererV2 * )csr )->categories(); // FIXME: QgsCategorizedSymbolRendererV2::categories() should be const
        for ( int i = 0; i < categories.size(); i++ )
        {
          QString label = categories[i].label();
          QString value = categories[i].value().toString();
          if ( label.isEmpty() )
            label = value;
          classes.insert( value, label );
        }
      }

      QComboBox *cb = comboBox( editor, parent );
      if ( cb )
      {
        for ( QMap<QString, QString>::const_iterator it = classes.begin(); it != classes.end(); it++ )
        {
          cb->addItem( it.value(), it.key() );
        }

        myWidget = cb;
      }
    }
    break;

    case QgsVectorLayer::DialRange:
    case QgsVectorLayer::SliderRange:
    case QgsVectorLayer::EditRange:
    {
      if ( myFieldType == QVariant::Int )
      {
        int min = vl->range( idx ).mMin.toInt();
        int max = vl->range( idx ).mMax.toInt();
        int step = vl->range( idx ).mStep.toInt();

        if ( editType == QgsVectorLayer::EditRange )
        {
          QSpinBox *sb = 0;

          if ( editor )
            sb = qobject_cast<QSpinBox *>( editor );
          else
            sb = new QSpinBox( parent );

          if ( sb )
          {
            sb->setRange( min, max );
            sb->setSingleStep( step );

            myWidget = sb;
          }
        }
        else
        {
          QAbstractSlider *sl = 0;

          if ( editor )
          {
            sl = qobject_cast<QAbstractSlider*>( editor );
          }
          else if ( editType == QgsVectorLayer::DialRange )
          {
            sl = new QDial( parent );
          }
          else
          {
            sl = new QSlider( Qt::Horizontal, parent );
          }

          if ( sl )
          {
            sl->setRange( min, max );
            sl->setSingleStep( step );

            myWidget = sl;
          }
        }
        break;
      }
      else if ( myFieldType == QVariant::Double )
      {
        QDoubleSpinBox *dsb = 0;
        if ( editor )
          dsb = qobject_cast<QDoubleSpinBox*>( editor );
        else
          dsb = new QDoubleSpinBox( parent );

        if ( dsb )
        {
          double min = vl->range( idx ).mMin.toDouble();
          double max = vl->range( idx ).mMax.toDouble();
          double step = vl->range( idx ).mStep.toDouble();

          dsb->setRange( min, max );
          dsb->setSingleStep( step );

          myWidget = dsb;
        }
        break;
      }
    }

    case QgsVectorLayer::CheckBox:
    {
      QCheckBox *cb = 0;
      QGroupBox *gb = 0;
      if ( editor )
      {
        gb = qobject_cast<QGroupBox *>( editor );
        cb = qobject_cast<QCheckBox*>( editor );
      }
      else
        cb = new QCheckBox( parent );

      if ( cb )
      {
        myWidget = cb;
        break;
      }
      else if ( gb )
      {
        myWidget = gb;
        break;
      }
    }

    // fall-through

    case QgsVectorLayer::LineEdit:
    case QgsVectorLayer::TextEdit:
    case QgsVectorLayer::UuidGenerator:
    case QgsVectorLayer::UniqueValuesEditable:
    case QgsVectorLayer::Immutable:
    {
      QLineEdit *le = 0;
      QTextEdit *te = 0;
      QPlainTextEdit *pte = 0;
      QComboBox * cb = 0;

      if ( editor )
      {
        le = qobject_cast<QLineEdit *>( editor );
        te = qobject_cast<QTextEdit *>( editor );
        pte = qobject_cast<QPlainTextEdit *>( editor );
        cb = qobject_cast<QComboBox *>( editor );
      }
      else if ( editType == QgsVectorLayer::TextEdit )
      {
        pte = new QPlainTextEdit( parent );
      }
      else
      {
        le = new QgsFilterLineEdit( parent );
      }

      if ( le )
      {
        if ( editType == QgsVectorLayer::UniqueValuesEditable )
        {
          QList<QVariant> values;
          vl->dataProvider()->uniqueValues( idx, values );

          QStringList svalues;
          for ( QList<QVariant>::const_iterator it = values.begin(); it != values.end(); it++ )
            svalues << it->toString();

          QCompleter *c = new QCompleter( svalues );
          c->setCompletionMode( QCompleter::PopupCompletion );
          le->setCompleter( c );
        }

        if ( editType == QgsVectorLayer::UuidGenerator )
        {
          le->setReadOnly( true );
        }

        le->setValidator( new QgsFieldValidator( le, field, vl->dateFormat( idx ) ) );

        myWidget = le;
      }

      if ( te )
      {
        te->setAcceptRichText( true );
        myWidget = te;
      }

      if ( pte )
      {
        myWidget = pte;
      }

      if ( cb )
      {
        if ( cb->isEditable() )
          cb->setValidator( new QgsFieldValidator( cb, field, vl->dateFormat( idx ) ) );
        myWidget = cb;
      }

      if ( myWidget )
      {
        if ( editType == QgsVectorLayer::Immutable )
        {
          myWidget->setDisabled( true );
        }

        QgsStringRelay* relay = NULL;

        QMap<int, QWidget*>::const_iterator it = proxyWidgets.find( idx );
        if ( it != proxyWidgets.end() )
        {
          QObject* obj = qvariant_cast<QObject*>(( *it )->property( "QgisAttrEditProxy" ) );
          relay = qobject_cast<QgsStringRelay*>( obj );
        }
        else
        {
          relay = new QgsStringRelay( myWidget );
        }

        const char* rSlot = SLOT( changeText( QString ) );
        const char* rSig = SIGNAL( textChanged( QString ) );
        const char* wSlot = SLOT( setText( QString ) );
        const char* wSig = SIGNAL( textChanged( QString ) );
        if ( te || pte )
        {
          rSlot = SLOT( changeText() );
          wSig = SIGNAL( textChanged() );
        }
        if ( pte )
        {
          wSlot = SLOT( setPlainText( QString ) );
        }
        if ( cb && cb->isEditable() )
        {
          wSlot = SLOT( setEditText( QString ) );
          wSig = SIGNAL( editTextChanged( QString ) );
        }

        synchronized =  connect( relay, rSig, myWidget, wSlot );
        synchronized &= connect( myWidget, wSig, relay, rSlot );

        // store list of proxies in relay
        relay->appendProxy( myWidget );

        if ( !cb || cb->isEditable() )
        {
          myWidget->setProperty( "QgisAttrEditSlot", QVariant( QByteArray( wSlot ) ) );
          myWidget->setProperty( "QgisAttrEditProxy", QVariant( QMetaType::QObjectStar, &relay ) );
        }
      }
    }
    break;

    case QgsVectorLayer::Hidden:
      myWidget = 0;
      break;

    case QgsVectorLayer::FileName:
    case QgsVectorLayer::Calendar:
    case QgsVectorLayer::Photo:
    case QgsVectorLayer::WebView:
    case QgsVectorLayer::Color:
    {
      QCalendarWidget *cw = qobject_cast<QCalendarWidget *>( editor );
      if ( cw )
      {
        myWidget = cw;
        break;
      }

      QWebView *ww = qobject_cast<QWebView *>( editor );
      if ( ww )
      {
        ww->page()->setNetworkAccessManager( QgsNetworkAccessManager::instance() );
        // ww->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );
        ww->settings()->setAttribute( QWebSettings::LocalContentCanAccessRemoteUrls, true );
        ww->settings()->setAttribute( QWebSettings::JavascriptCanOpenWindows, true );
#ifdef QGISDEBUG
        ww->settings()->setAttribute( QWebSettings::DeveloperExtrasEnabled, true );
#endif

        QSize size( vl->widgetSize( idx ) );
        if ( size.width() > 0 || size.height() > 0 )
        {
          if ( size.width() == 0 )
            size.setWidth( 1 );
          if ( size.height() == 0 )
            size.setHeight( 1 );
          ww->setMinimumSize( size );
        }

        myWidget = ww;
        break;
      }

      QLabel *lw = qobject_cast<QLabel *>( editor );
      if ( lw )
      {
        myWidget = lw;
        break;
      }

      QgsColorButton *cb = qobject_cast<QgsColorButton *>( editor );
      if ( cb )
      {
        myWidget = cb;
        break;
      }

      QPushButton *pb0 = 0;
      QPushButton *pb1 = 0;
      QLineEdit *le = qobject_cast<QLineEdit *>( editor );
      if ( le )
      {
        if ( le )
          myWidget = le;

        if ( editor->parent() )
        {
          pb0 = editor->parent()->findChild<QPushButton *>();
        }
      }
      else
      {
        myWidget = new QWidget( parent );
        myWidget->setBackgroundRole( QPalette::Window );
        myWidget->setAutoFillBackground( true );

        le = new QgsFilterLineEdit( myWidget );
        switch ( editType )
        {
          case QgsVectorLayer::FileName:
          case QgsVectorLayer::Photo:
          case QgsVectorLayer::Calendar:
            pb0 = new QPushButton( tr( "..." ), myWidget );
            break;

          case QgsVectorLayer::WebView:
            pb0 = new QPushButton( tr( "<" ), myWidget );
            pb0->setObjectName( "saveUrl" );
            pb1 = new QPushButton( tr( "..." ), myWidget );
            pb1->setObjectName( "openUrl" );
            break;

          case QgsVectorLayer::Color:
            pb0 = new QgsColorButton( myWidget );
            break;

          default:
            break;
        }


        int row = 0;
        QGridLayout *layout = new QGridLayout( myWidget );
        if ( editType == QgsVectorLayer::Photo )
        {
          lw = new QLabel( myWidget );
          layout->addWidget( lw, 0, 0, 1, 2 );
          row++;
        }
        else if ( editType == QgsVectorLayer::WebView )
        {
          ww = new QWebView( myWidget );
          ww->setObjectName( "webview" );
          ww->page()->setNetworkAccessManager( QgsNetworkAccessManager::instance() );
          // ww->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );
          ww->settings()->setAttribute( QWebSettings::LocalContentCanAccessRemoteUrls, true );
          ww->settings()->setAttribute( QWebSettings::JavascriptCanOpenWindows, true );
#ifdef QGISDEBUG
          ww->settings()->setAttribute( QWebSettings::DeveloperExtrasEnabled, true );
#endif

          QSize size( vl->widgetSize( idx ) );
          if ( size.width() > 0 || size.height() > 0 )
          {
            if ( size.width() == 0 )
              size.setWidth( 1 );
            if ( size.height() == 0 )
              size.setHeight( 1 );
            ww->setMinimumSize( size );
          }

          layout->addWidget( ww, 0, 0, 1, 2 );
          row++;
        }

        layout->addWidget( le, row, 0 );
        layout->addWidget( pb0, row, 1 );
        if ( pb1 )
          layout->addWidget( pb1, row, 2 );

        myWidget->setLayout( layout );
      }

      if ( le )
      {
        le->setValidator( new QgsFieldValidator( le, field, vl->dateFormat( idx ) ) );

        if ( ww )
          connect( le, SIGNAL( textChanged( const QString & ) ), new QgsAttributeEditor( le, vl, idx ), SLOT( loadUrl( const QString & ) ) );
        if ( lw )
          connect( le, SIGNAL( textChanged( const QString & ) ), new QgsAttributeEditor( le, vl, idx ), SLOT( loadPixmap( const QString & ) ) );
        if ( editType == QgsVectorLayer::Color )
          connect( le, SIGNAL( textChanged( const QString & ) ), new QgsAttributeEditor( le ), SLOT( updateColor() ) );
      }

      if ( pb0 )
      {
        if ( editType == QgsVectorLayer::FileName || editType == QgsVectorLayer::Photo )
        {
          connect( pb0, SIGNAL( clicked() ), new QgsAttributeEditor( pb0 ), SLOT( selectFileName() ) );
          pb0->setToolTip( tr( "Select filename..." ) );
        }
        if ( editType == QgsVectorLayer::WebView )
        {
          connect( pb0, SIGNAL( clicked() ), new QgsAttributeEditor( pb0 ), SLOT( updateUrl() ) );
          pb0->setToolTip( tr( "Save current page url in attribute" ) );
        }
        if ( editType == QgsVectorLayer::Calendar )
        {
          connect( pb0, SIGNAL( clicked() ), new QgsAttributeEditor( pb0 ), SLOT( selectDate() ) );
          pb0->setToolTip( tr( "Select date in calendar" ) );
        }
        if ( editType == QgsVectorLayer::Color )
        {
          connect( pb0, SIGNAL( colorChanged( const QColor & ) ), new QgsAttributeEditor( pb0 ), SLOT( updateColor() ) );
          pb0->setToolTip( tr( "Select color in browser" ) );
        }
      }

      if ( pb1 )
      {
        if ( editType == QgsVectorLayer::WebView )
        {
          connect( pb1, SIGNAL( clicked() ), new QgsAttributeEditor( pb1 ), SLOT( openUrl() ) );
          pb1->setToolTip( tr( "Open current page in default browser" ) );
        }
      }
    }
    break;
  }

  QMap<int, QWidget*>::const_iterator it = proxyWidgets.find( idx );
  if ( it != proxyWidgets.end() )
  {
    if ( !synchronized )
    {
      myWidget->setEnabled( false );
    }
  }
  else
  {
    proxyWidgets.insert( idx, myWidget );
  }

  setValue( myWidget, vl, idx, value );

  return myWidget;
}

bool QgsAttributeEditor::retrieveValue( QWidget *widget, QgsVectorLayer *vl, int idx, QVariant &value )
{
  if ( !widget )
    return false;

  const QgsField &theField = vl->pendingFields()[idx];
  QgsVectorLayer::EditType editType = vl->editType( idx );
  bool modified = false;
  QString text;

  QSettings settings;
  QString nullValue = settings.value( "qgis/nullValue", "NULL" ).toString();

  QLineEdit *le = qobject_cast<QLineEdit *>( widget );
  if ( le )
  {
    text = le->text();
    modified = le->isModified();
    if ( text == nullValue )
    {
      text = QString::null;
    }
  }

  QTextEdit *te = qobject_cast<QTextEdit *>( widget );
  if ( te )
  {
    text = te->toHtml();
    modified = te->document()->isModified();
    if ( text == nullValue )
    {
      text = QString::null;
    }
  }

  QPlainTextEdit *pte = qobject_cast<QPlainTextEdit *>( widget );
  if ( pte )
  {
    text = pte->toPlainText();
    modified = pte->document()->isModified();
    if ( text == nullValue )
    {
      text = QString::null;
    }
  }

  QComboBox *cb = qobject_cast<QComboBox *>( widget );
  if ( cb )
  {
    if ( editType == QgsVectorLayer::UniqueValues ||
         editType == QgsVectorLayer::ValueMap ||
         editType == QgsVectorLayer::Classification ||
         editType == QgsVectorLayer::ValueRelation )
    {
      text = cb->itemData( cb->currentIndex() ).toString();
      if ( text == nullValue )
      {
        text = QString::null;
      }
    }
    else
    {
      text = cb->currentText();
    }
    modified = true;
  }

  QListWidget *lw = qobject_cast<QListWidget *>( widget );
  if ( lw )
  {
    if ( editType == QgsVectorLayer::ValueRelation )
    {
      text = '{';
      for ( int i = 0, n = 0; i < lw->count(); i++ )
      {
        if ( lw->item( i )->checkState() == Qt::Checked )
        {
          if ( n > 0 )
          {
            text.append( ',' );
          }
          text.append( lw->item( i )->data( Qt::UserRole ).toString() );
          n++;
        }
      }
      text.append( '}' );
    }
    else
    {
      text = QString::null;
    }
    modified = true;
  }

  QSpinBox *sb = qobject_cast<QSpinBox *>( widget );
  if ( sb )
  {
    text = QString::number( sb->value() );
  }

  QAbstractSlider *slider = qobject_cast<QAbstractSlider *>( widget );
  if ( slider )
  {
    text = QString::number( slider->value() );
  }

  QDoubleSpinBox *dsb = qobject_cast<QDoubleSpinBox *>( widget );
  if ( dsb )
  {
    text = QString::number( dsb->value() );
  }

  QCheckBox *ckb = qobject_cast<QCheckBox *>( widget );
  if ( ckb )
  {
    QPair<QString, QString> states = vl->checkedState( idx );
    text = ckb->isChecked() ? states.first : states.second;
  }

  QGroupBox *gb = qobject_cast<QGroupBox *>( widget );
  if ( gb )
  {
    QPair<QString, QString> states = vl->checkedState( idx );
    text = gb->isChecked() ? states.first : states.second;
  }

  QCalendarWidget *cw = qobject_cast<QCalendarWidget *>( widget );
  if ( cw )
  {
    text = cw->selectedDate().toString( vl->dateFormat( idx ) );
  }

  le = widget->findChild<QLineEdit *>();
  // QCalendarWidget and QGroupBox have an internal QLineEdit which returns the year
  // part of the date so we need to skip this if we have a QCalendarWidget
  if ( !cw && !gb && le )
  {
    text = le->text();
    modified = le->isModified();
    if ( text == nullValue )
    {
      text = QString::null;
    }
  }

  switch ( theField.type() )
  {
    case QVariant::Int:
    {
      bool ok;
      int myIntValue = text.toInt( &ok );
      if ( ok && !text.isEmpty() )
      {
        value = QVariant( myIntValue );
        modified = true;
      }
      else if ( modified )
      {
        value = QVariant();
      }
    }
    break;
    case QVariant::LongLong:
    {
      bool ok;
      qlonglong myLongValue = text.toLong( &ok );
      if ( ok && !text.isEmpty() )
      {
        value = QVariant( myLongValue );
        modified = true;
      }
      else if ( modified )
      {
        value = QVariant();
      }
    }
    case QVariant::Double:
    {
      bool ok;
      double myDblValue = text.toDouble( &ok );
      if ( ok && !text.isEmpty() )
      {
        value = QVariant( myDblValue );
        modified = true;
      }
      else if ( modified )
      {
        value = QVariant();
      }
    }
    break;
    case QVariant::Date:
    {
      QDate myDateValue = QDate::fromString( text, vl->dateFormat( idx ) );
      if ( myDateValue.isValid() && !text.isEmpty() )
      {
        value = myDateValue;
        modified = true;
      }
      else if ( modified )
      {
        value = QVariant();
      }
    }
    break;
    default: //string
      modified = true;
      if ( text.isNull() )
        value = QVariant( theField.type() );
      else
        value = QVariant( text );
      break;
  }

  return modified;
}

bool QgsAttributeEditor::setValue( QWidget *editor, QgsVectorLayer *vl, int idx, const QVariant &value )
{
  if ( !editor )
    return false;

  QgsVectorLayer::EditType editType = vl->editType( idx );
  const QgsField &field = vl->pendingFields()[idx];
  QVariant::Type myFieldType = field.type();

  QSettings settings;
  QString nullValue = settings.value( "qgis/nullValue", "NULL" ).toString();

  switch ( editType )
  {
    case QgsVectorLayer::Classification:
    case QgsVectorLayer::UniqueValues:
    case QgsVectorLayer::Enumeration:
    case QgsVectorLayer::ValueMap:
    case QgsVectorLayer::ValueRelation:
    {
      QVariant v = value;
      QComboBox *cb = qobject_cast<QComboBox *>( editor );
      if ( !cb )
        return false;

      if ( v.isNull() )
      {
        v = nullValue;
      }

      int idx = cb->findData( v );
      if ( idx < 0 )
        return false;

      cb->setCurrentIndex( idx );
    }
    break;

    case QgsVectorLayer::DialRange:
    case QgsVectorLayer::SliderRange:
    case QgsVectorLayer::EditRange:
    {
      if ( myFieldType == QVariant::Int )
      {
        if ( editType == QgsVectorLayer::EditRange )
        {
          QSpinBox *sb = qobject_cast<QSpinBox *>( editor );
          if ( !sb )
            return false;
          sb->setValue( value.toInt() );
        }
        else
        {
          QAbstractSlider *sl = qobject_cast<QAbstractSlider *>( editor );
          if ( !sl )
            return false;
          sl->setValue( value.toInt() );
        }
        break;
      }
      else if ( myFieldType == QVariant::Double )
      {
        QDoubleSpinBox *dsb = qobject_cast<QDoubleSpinBox *>( editor );
        if ( !dsb )
          return false;
        dsb->setValue( value.toDouble() );
      }
    }

    case QgsVectorLayer::CheckBox:
    {
      QGroupBox *gb = qobject_cast<QGroupBox *>( editor );
      if ( gb )
      {
        QPair<QString, QString> states = vl->checkedState( idx );
        gb->setChecked( value == states.first );
        break;
      }

      QCheckBox *cb = qobject_cast<QCheckBox *>( editor );
      if ( cb )
      {
        QPair<QString, QString> states = vl->checkedState( idx );
        cb->setChecked( value == states.first );
        break;
      }
    }

    // fall-through

    case QgsVectorLayer::LineEdit:
    case QgsVectorLayer::UniqueValuesEditable:
    case QgsVectorLayer::Immutable:
    case QgsVectorLayer::UuidGenerator:
    case QgsVectorLayer::TextEdit:
    {
      QgsFilterLineEdit *fle = qobject_cast<QgsFilterLineEdit *>( editor );
      QLineEdit *le = qobject_cast<QLineEdit *>( editor );
      QComboBox *cb = qobject_cast<QComboBox *>( editor );
      QTextEdit *te = qobject_cast<QTextEdit *>( editor );
      QPlainTextEdit *pte = qobject_cast<QPlainTextEdit *>( editor );
      if ( !le && ! cb && !te && !pte )
        return false;

      if ( fle && !( myFieldType == QVariant::Int || myFieldType == QVariant::Double || myFieldType == QVariant::LongLong || myFieldType == QVariant::Date ) )
      {
        fle->setNullValue( nullValue );
      }

      QString text;
      if ( value.isNull() )
      {
        if ( myFieldType == QVariant::Int || myFieldType == QVariant::Double || myFieldType == QVariant::LongLong || myFieldType == QVariant::Date )
          text = "";
        else if ( editType == QgsVectorLayer::UuidGenerator )
          text = QUuid::createUuid().toString();
        else
          text = nullValue;
      }
      else
      {
        text = field.displayString( value );
      }

      if ( le )
        le->setText( text );
      if ( cb && cb->isEditable() )
        cb->setEditText( text );
      if ( te )
        te->setHtml( text );
      if ( pte )
        pte->setPlainText( text );
    }
    break;

    case QgsVectorLayer::FileName:
    case QgsVectorLayer::Calendar:
    case QgsVectorLayer::Photo:
    case QgsVectorLayer::WebView:
    case QgsVectorLayer::Color:
    {
      QCalendarWidget *cw = qobject_cast<QCalendarWidget *>( editor );
      if ( cw )
      {
        cw->setSelectedDate( value.toDate() );
        break;
      }

      QWebView *ww = qobject_cast<QWebView *>( editor );
      if ( ww )
      {
        ww->load( value.toString() );
        break;
      }

      QLabel *lw = qobject_cast<QLabel *>( editor );
      if ( lw )
        break;

      QgsColorButton *cb = qobject_cast<QgsColorButton *>( editor );
      if ( cb )
      {
        cb->setColor( QColor( value.toString() ) );
        break;
      }

      QgsFilterLineEdit *fle = qobject_cast<QgsFilterLineEdit*>( editor );
      QLineEdit *le = qobject_cast<QLineEdit*>( editor );
      if ( !le )
      {
        le = editor->findChild<QLineEdit *>();
        fle = qobject_cast<QgsFilterLineEdit *>( le );
      }
      if ( !le )
        return false;

      if ( fle && !( myFieldType == QVariant::Int || myFieldType == QVariant::Double || myFieldType == QVariant::LongLong || myFieldType == QVariant::Date ) )
      {
        fle->setNullValue( nullValue );
      }

      QString text;
      if ( value.isNull() )
      {
        if ( myFieldType == QVariant::Int || myFieldType == QVariant::Double || myFieldType == QVariant::LongLong || myFieldType == QVariant::Date )
          text = "";
        else
          text = nullValue;
      }
      else if ( editType == QgsVectorLayer::Calendar && value.canConvert( QVariant::Date ) )
      {
        text = value.toDate().toString( vl->dateFormat( idx ) );
      }
      else
      {
        text = value.toString();
      }

      le->setText( text );
    }
    break;

    case QgsVectorLayer::Hidden:
      break;
  }

  return true;
}

QWidget* QgsAttributeEditor::createWidgetFromDef( const QgsAttributeEditorElement* widgetDef, QWidget* parent, QgsVectorLayer* vl, QgsAttributes &attrs, QMap<int, QWidget*> &proxyWidgets, bool createGroupBox )
{
  QWidget *newWidget = 0;

  switch ( widgetDef->type() )
  {
    case QgsAttributeEditorElement::AeTypeField:
    {
      const QgsAttributeEditorField* fieldDef = dynamic_cast<const QgsAttributeEditorField*>( widgetDef );
      int fldIdx = fieldDef->idx();
      newWidget = createAttributeEditor( parent, 0, vl, fldIdx, attrs.value( fldIdx, QVariant() ), proxyWidgets );

      if ( vl->editType( fldIdx ) != QgsVectorLayer::Immutable )
      {
        if ( newWidget->isEnabled() && vl->isEditable() && vl->fieldEditable( fldIdx ) )
        {
          newWidget->setEnabled( true );
        }
        else if ( vl->editType( fldIdx ) == QgsVectorLayer::Photo )
        {
          foreach ( QWidget *w, newWidget->findChildren<QWidget *>() )
          {
            w->setEnabled( qobject_cast<QLabel *>( w ) ? true : false );
          }
        }
        else if ( vl->editType( fldIdx ) == QgsVectorLayer::WebView )
        {
          foreach ( QWidget *w, newWidget->findChildren<QWidget *>() )
          {
            if ( qobject_cast<QWebView *>( w ) )
              w->setEnabled( true );
            else if ( qobject_cast<QPushButton *>( w ) && w->objectName() == "openUrl" )
              w->setEnabled( true );
            else
              w->setEnabled( false );
          }
        }
        else
        {
          newWidget->setEnabled( false );
        }
      }

      break;
    }

    case QgsAttributeEditorElement::AeTypeContainer:
    {
      const QgsAttributeEditorContainer* container = dynamic_cast<const QgsAttributeEditorContainer*>( widgetDef );
      QWidget* myContainer;

      if ( createGroupBox )
      {
        QGroupBox* groupBox = new QGroupBox( parent );
        groupBox->setTitle( container->name() );
        myContainer = groupBox;
        newWidget = myContainer;
      }
      else
      {
        QScrollArea *scrollArea = new QScrollArea( parent );

        myContainer = new QWidget( scrollArea );

        scrollArea->setWidget( myContainer );
        scrollArea->setWidgetResizable( true );
        scrollArea->setFrameShape( QFrame::NoFrame );

        newWidget = scrollArea;
      }

      QGridLayout* gbLayout = new QGridLayout( myContainer );
      myContainer->setLayout( gbLayout );

      int index = 0;

      QList<QgsAttributeEditorElement*> children = container->children();

      for ( QList<QgsAttributeEditorElement*>::const_iterator it = children.begin(); it != children.end(); ++it )
      {
        QgsAttributeEditorElement* childDef = *it;
        QWidget* editor = createWidgetFromDef( childDef, myContainer, vl, attrs, proxyWidgets, true );

        if ( childDef->type() == QgsAttributeEditorElement::AeTypeContainer )
        {
          gbLayout->addWidget( editor, index, 0, 1, 2 );
        }
        else
        {
          const QgsAttributeEditorField* fieldDef = dynamic_cast<const QgsAttributeEditorField*>( childDef );

          //show attribute alias if available
          QString myFieldName = vl->attributeDisplayName( fieldDef->idx() );
          QLabel *mypLabel = new QLabel( myFieldName, myContainer );

          if ( vl->labelOnTop( fieldDef->idx() ) )
          {
            gbLayout->addWidget( mypLabel, index++, 0, 1, 2 );
            gbLayout->addWidget( editor, index, 0, 1 , 2 );
          }
          else
          {
            gbLayout->addWidget( mypLabel, index, 0 );
            gbLayout->addWidget( editor, index, 1 );
          }
        }

        ++index;
      }
      gbLayout->addItem( new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding ), index , 0 );

      break;
    }

    default:
      QgsDebugMsg( "Unknown attribute editor widget type encountered..." );
      break;
  }

  return newWidget;
}

void QgsStringRelay::changeText()
{
  QObject* sObj = QObject::sender();
  QTextEdit *te = qobject_cast<QTextEdit *>( sObj );
  QPlainTextEdit *pte = qobject_cast<QPlainTextEdit *>( sObj );

  if ( te )
    changeText( te->toPlainText() );
  if ( pte )
    changeText( pte->toPlainText() );
}

void QgsStringRelay::changeText( QString str )
{
  QObject* sObj = QObject::sender();
  const char* sSlot = sObj->property( "QgisAttrEditSlot" ).toByteArray().constData();

  // disconnect widget being edited from relay's signal
  disconnect( this, SIGNAL( textChanged( QString ) ), sObj, sSlot );

  // block all proxies' signals
  QList<bool> oldBlockSigs;
  for ( int i = 0; i < mProxyList.size(); ++i )
  {
    oldBlockSigs << ( mProxyList[i] )->blockSignals( true );
  }

  // update all proxies not being edited without creating cyclical signals/slots
  emit textChanged( str );

  // reconnect widget being edited and reset blockSignals state
  connect( this, SIGNAL( textChanged( QString ) ), sObj, sSlot );
  for ( int i = 0; i < mProxyList.size(); ++i )
  {
    mProxyList[i]->blockSignals( oldBlockSigs[i] );
  }
}
