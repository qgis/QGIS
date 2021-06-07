/***************************************************************************
                          qgsbusyindicatordialog.cpp
                          --------------------------
    begin                : Mar 27, 2013
    copyright            : (C) 2013 by Larry Shaffer
    email                : larrys at dakcarto dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsbusyindicatordialog.h"

#include <QDialog>
#include <QLayout>
#include <QLabel>
#include <QProgressBar>

QgsBusyIndicatorDialog::QgsBusyIndicatorDialog( const QString &message, QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mMessage( QString( message ) )

{
  setWindowTitle( tr( "QGIS" ) );
  setLayout( new QVBoxLayout() );
  setWindowModality( Qt::WindowModal );
  setMinimumWidth( 250 );
  mMsgLabel = new QLabel( mMessage );
  layout()->addWidget( mMsgLabel );

  QProgressBar *pb = new QProgressBar();
  pb->setMaximum( 0 ); // show as busy indicator
  layout()->addWidget( pb );

  if ( mMessage.isEmpty() )
  {
    mMsgLabel->hide();
  }
}

void QgsBusyIndicatorDialog::setMessage( const QString &message )
{
  if ( !message.isEmpty() )
  {
    mMessage = QString( message );
    mMsgLabel->setText( mMessage );
    mMsgLabel->show();
  }
}
