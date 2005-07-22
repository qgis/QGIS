/***************************************************************************
 *   Copyright (C) 2005 by Tim Sutton   *
 *   aps02ts@macbuntu   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "qgslegend.h"
#include "qgslegendlayerfile.h"
#include "qgsmaplayer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerproperties.h"
#include <qapplication.h>
#include <qcheckbox.h>
#include <qpixmap.h>
#include <qobject.h>
#include <qpopupmenu.h>

QgsLegendLayerFile::QgsLegendLayerFile(QListViewItem * theLegendItem, QString theString, QgsMapLayer* theLayer)
    : QgsLegendItem(theLegendItem, theString), mLayer(theLayer)
{
  mType = LEGEND_LAYER_FILE;
  
  //visibility check box
  mVisibilityCheckBox = new QCheckBox(listView());
  ((QgsLegend*)(listView()))->registerCheckBox(this, mVisibilityCheckBox);
  mVisibilityCheckBox->setChecked(true);
  mVisibilityCheckBox->hide();
  QObject::connect(mVisibilityCheckBox, SIGNAL(toggled(bool)), mLayer, SLOT(setVisible(bool)));

  setPixmap(0, getOriginalPixmap());
}


QgsLegendLayerFile::~QgsLegendLayerFile()
{
    //unregistering of the checkbox is done by QgsLegend
    delete mVisibilityCheckBox;
}

bool QgsLegendLayerFile::accept(LEGEND_ITEM_TYPE type)
{
  return false;
}

void QgsLegendLayerFile::handleDoubleClickEvent()
{
#ifdef QGISDEBUG
    qWarning("In QgsLegendLayerFile::handleDoubleClickEvent");
#endif
    if (mLayer->type() == QgsMapLayer::RASTER)
    {
        QgsRasterLayerProperties *rlp = new QgsRasterLayerProperties(mLayer);
        // The signals to change the raster layer properties will only be emitted
        // when the user clicks ok or apply
        if (rlp->exec())
        {
            //this code will be called it the user selects ok
            //mMapCanvas->setDirty(true);
            //mMapCanvas->refresh();
            //mMapCanvas->render();
            // mMapLegend->update(); XXX WHY CALL UPDATE HERE?
            delete rlp;
            qApp->processEvents();
        }
    }
    else //vector
    {
        mLayer->showLayerProperties();
    }
}

void QgsLegendLayerFile::handleRightClickEvent(const QPoint& position)
{
#ifdef QGISDEBUG
    qWarning("In QgsLegendLayerFile::handleRightClickEvent");
#endif
    //if (!mMapCanvas->isDrawing()&&lvi) //todo: test if QgsMapCanvas::isDrawing
    QPopupMenu *mPopupMenu = mLayer->contextMenu();
    if (mPopupMenu)
    {
	mPopupMenu->exec(position);
    }
}

QPixmap QgsLegendLayerFile::getOriginalPixmap() const
{
    QPixmap myPixmap(QString(PKGDATAPATH)+QString("/images/icons/file.png"));
    return myPixmap;
}

void QgsLegendLayerFile::setLegendPixmap(const QPixmap& pix)
{
    setPixmap(0, pix);
}

void QgsLegendLayerFile::toggleCheckBox(bool state)
{
    mVisibilityCheckBox->setChecked(state);
}

