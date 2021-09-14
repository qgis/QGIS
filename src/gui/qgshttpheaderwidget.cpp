#include "qgshttpheaderwidget.h"
#include "ui_qgshttpheaderwidget.h"

QgsHttpHeaderWidget::QgsHttpHeaderWidget( QWidget *parent ) :
  QWidget( parent )
{
  setupUi( this );
  setupConnections();
}

QgsHttpHeaderWidget::~QgsHttpHeaderWidget()
{
  // nope
}

void QgsHttpHeaderWidget::setupConnections()
{
  // Action and interaction connections
  connect( btnAddQueryPair, &QToolButton::clicked, this, &QgsHttpHeaderWidget::addQueryPair );
  connect( btnRemoveQueryPair, &QToolButton::clicked, this, &QgsHttpHeaderWidget::removeQueryPair );
}

void QgsHttpHeaderWidget::addQueryPairRow( const QString &key, const QString &val )
{
  const int rowCnt = tblwdgQueryPairs->rowCount();
  tblwdgQueryPairs->insertRow( rowCnt );

  const Qt::ItemFlags itmFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable
                                 | Qt::ItemIsEditable | Qt::ItemIsDropEnabled;

  QTableWidgetItem *keyItm = new QTableWidgetItem( key );
  keyItm->setFlags( itmFlags );
  tblwdgQueryPairs->setItem( rowCnt, 0, keyItm );

  QTableWidgetItem *valItm = new QTableWidgetItem( val );
  keyItm->setFlags( itmFlags );
  tblwdgQueryPairs->setItem( rowCnt, 1, valItm );
}

QgsHttpHeaders QgsHttpHeaderWidget::httpHeaders() const
{
  QgsHttpHeaders querypairs;
  for ( int i = 0; i < tblwdgQueryPairs->rowCount(); ++i )
  {
    if ( tblwdgQueryPairs->item( i, 0 )->text().isEmpty() )
    {
      continue;
    }
    querypairs [ tblwdgQueryPairs->item( i, 0 )->text() ] = QVariant( tblwdgQueryPairs->item( i, 1 )->text() ) ;
  }

  if ( !mRefererLineEdit->text().isEmpty() )
  {
    querypairs [ "referer" ] = QVariant( mRefererLineEdit->text() ) ;
  }
  return querypairs;
}


void QgsHttpHeaderWidget::addQueryPair()
{
  addQueryPairRow( QString(), QString() );
  tblwdgQueryPairs->setFocus();
  tblwdgQueryPairs->setCurrentCell( tblwdgQueryPairs->rowCount() - 1, 0 );
  tblwdgQueryPairs->edit( tblwdgQueryPairs->currentIndex() );
}


void QgsHttpHeaderWidget::removeQueryPair()
{
  tblwdgQueryPairs->removeRow( tblwdgQueryPairs->currentRow() );
}


void QgsHttpHeaderWidget::setFromSettings( const QgsSettings &settings, const QString &key )
{
  // load headers from settings
  QgsHttpHeaders headers;
  headers.setFromSettings( settings, key );
  printf("In QgsHttpHeaderWidget::setFromSettings: headers[referer]:'%s'\n",
         headers["referer"].toString().toStdString().c_str());

  // push headers to table
  tblwdgQueryPairs->clearContents();
  /*QTableWidgetItem * item;
  int row = 0;*/
  QList<QString> lst = headers.keys();
  for ( auto ite = lst.constBegin(); ite != lst.constEnd(); ++ite )
  {
    if ( ite->compare( "referer" ) != 0 )
    {
      addQueryPairRow( *ite, headers[ *ite ].toString() );
    }
    else
    {
      mRefererLineEdit->setText( headers[ *ite ].toString() );
    }
  }
  /* mRefererLineEdit->setText( settings.value( key + "/referer" ).toString() ); */
}

/* settings.setValue( key + "/referer", mRefererLineEdit->text() ); */
void QgsHttpHeaderWidget::updateSettings( QgsSettings &settings, const QString &key ) const
{
  QgsHttpHeaders h = httpHeaders();
  h.updateSettings( settings, key );
  printf("In QgsHttpHeaderWidget::updateSettings: h[referer]:'%s', settings:'%s'\n",
         h["referer"].toString().toStdString().c_str(),
          settings.value( key + "referer" ).toString().toStdString().c_str());
}
