/***************************************************************************
     mapcoordsdialog.cpp
     --------------------------------------
    Date                 : 2005
    Copyright            : (C) 2005 by Lars Luthman
    Email                : larsl at users dot sourceforge dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#include <QValidator>
#include <QPushButton>

#include "qgsmapcanvas.h"

#include "qgsgeorefvalidators.h"
#include "qgsmapcoordsdialog.h"

QgsMapCoordsDialog::QgsMapCoordsDialog( QgsMapCanvas* qgisCanvas, QgsPoint pixelCoords, QWidget* parent )
  : QDialog( parent, Qt::Dialog ), mQgisCanvas(qgisCanvas), mPixelCoords(pixelCoords)
{
  setupUi( this );

  setAttribute(Qt::WA_DeleteOnClose);

  mPointFromCanvasPushButton = new QPushButton(QIcon(":/icons/mPushButtonPencil.png"), tr("From map canvas"));
  mPointFromCanvasPushButton->setCheckable(true);
  buttonBox->addButton(mPointFromCanvasPushButton, QDialogButtonBox::ActionRole);
  adjustSize();

  // User can input either DD or DMS coords (from QGis mapcanav we take DD coords)
  QgsDMSAndDDValidator *validator = new QgsDMSAndDDValidator(this);
  leXCoord->setValidator( validator );
  leYCoord->setValidator( validator );

  mToolEmitPoint = new QgsGeorefMapToolEmitPoint( qgisCanvas );
  mToolEmitPoint->setButton( mPointFromCanvasPushButton );

  connect(mPointFromCanvasPushButton, SIGNAL(clicked(bool)), this, SLOT(setToolEmitPoint(bool)));

  connect( mToolEmitPoint, SIGNAL( canvasClicked( const QgsPoint&, Qt::MouseButton ) ),
           this, SLOT( maybeSetXY( const QgsPoint&, Qt::MouseButton ) ) );
  connect( mToolEmitPoint, SIGNAL(mouseReleased()), this, SLOT(setPrevTool()));

  connect( leXCoord, SIGNAL( textChanged( const QString& ) ), this, SLOT( updateOK() ) );
  connect( leYCoord, SIGNAL( textChanged( const QString& ) ), this, SLOT( updateOK() ) );
  updateOK();
}

QgsMapCoordsDialog::~QgsMapCoordsDialog()
{
  delete mToolEmitPoint;
}

void QgsMapCoordsDialog::updateOK()
{
  bool enable = ( leXCoord->text().size() != 0 && leYCoord->text().size() != 0 );
  QPushButton *okPushButton = buttonBox->button(QDialogButtonBox::Ok);
  okPushButton->setEnabled( enable );
}

void QgsMapCoordsDialog::setPrevTool()
{
  mQgisCanvas->setMapTool( mPrevMapTool );
}

void QgsMapCoordsDialog::on_buttonBox_accepted()
{
  bool ok;
  double x = leXCoord->text().toDouble(&ok);
  if (!ok)
    x = dmsToDD(leXCoord->text());

  double y = leYCoord->text().toDouble(&ok);
  if (!ok)
    y = dmsToDD(leYCoord->text());

  emit pointAdded(mPixelCoords, QgsPoint(x, y));

  close();
}

void QgsMapCoordsDialog::maybeSetXY( const QgsPoint & xy, Qt::MouseButton button )
{
  // Only LeftButton should set point
  if ( Qt::LeftButton == button )
  {
    leXCoord->clear();
    leYCoord->clear();
    leXCoord->setText( QString::number( xy.x(), 'f', 7 ) );
    leYCoord->setText( QString::number( xy.y(), 'f', 7 ) );
  }

  parentWidget()->showNormal();
  parentWidget()->activateWindow();
  parentWidget()->raise();

  mPointFromCanvasPushButton->setChecked(false);
  buttonBox->button(QDialogButtonBox::Ok)->setFocus();
}

void QgsMapCoordsDialog::setToolEmitPoint(bool isEnable)
{
  if (isEnable)
  {
    parentWidget()->showMinimized();
    parentWidget()->parentWidget()->activateWindow();
    parentWidget()->parentWidget()->raise();

    mPrevMapTool = mQgisCanvas->mapTool();
    mQgisCanvas->setMapTool(mToolEmitPoint);
  }
  else
  {
    mQgisCanvas->setMapTool(mPrevMapTool);
  }
}

double QgsMapCoordsDialog::dmsToDD(QString dms)
{
  QStringList list = dms.split(' ');
  QString tmpStr = list.at(0);
  double res = qAbs(tmpStr.toDouble());

  tmpStr = list.value(1);
  if (!tmpStr.isEmpty())
    res += tmpStr.toDouble() / 60;

  tmpStr = list.value(2);
  if (!tmpStr.isEmpty())
    res += tmpStr.toDouble() / 3600;

  if (dms.startsWith('-'))
    return -res;
  else
    return res;
}
