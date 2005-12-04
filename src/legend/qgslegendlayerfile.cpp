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
#include <QPainter>

QgsLegendLayerFile::QgsLegendLayerFile(QTreeWidgetItem * theLegendItem, QString theString, QgsMapLayer* theLayer)
    : QgsLegendItem(theLegendItem, theString), mLayer(theLayer)
{
  mType = LEGEND_LAYER_FILE;
  QPixmap originalPixmap = getOriginalPixmap();
  //ensure the overview glasses is painted if necessary
  if(mLayer->showInOverviewStatus())
  {
#if defined(Q_OS_MACX) || defined(WIN32) 
      QString pkgDataPath(qApp->applicationDirPath()+QString("/share/qgis"));
#else
      QString pkgDataPath(PKGDATAPATH);
#endif  
      QPixmap inOverviewPixmap(pkgDataPath+QString("/images/icons/inoverview.png"));
      QPainter p(&originalPixmap);
      p.drawPixmap(0,0,inOverviewPixmap);
  }
  QIcon originalIcon(originalPixmap);
  setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
  setCheckState (0, Qt::Checked );
  setText(0, theString);
  setIcon(0, originalIcon);
}


QgsLegendLayerFile::~QgsLegendLayerFile()
{
}

QgsLegendItem::DRAG_ACTION QgsLegendLayerFile::accept(LEGEND_ITEM_TYPE type)
{
  return NO_ACTION;
}

QgsLegendItem::DRAG_ACTION QgsLegendLayerFile::accept(const QgsLegendItem* li) const
{
  if(li->type() == QgsLegendItem::LEGEND_LAYER_FILE)
    {
      if(li->parent() == this->parent())
	{
	  return REORDER;
	}
    }
  return NO_ACTION;
}

QPixmap QgsLegendLayerFile::getOriginalPixmap() const
{
#if defined(Q_OS_MACX) || defined(WIN32)
    QString pkgDataPath(qApp->applicationDirPath()+QString("/share/qgis"));
#else
    QString pkgDataPath(PKGDATAPATH);
#endif
    QPixmap myPixmap(pkgDataPath+QString("/images/icons/file.png"));
    return myPixmap;
}

void QgsLegendLayerFile::setLegendPixmap(const QPixmap& pix)
{
  QIcon theIcon(pix);
  setIcon(0, theIcon);
}

void QgsLegendLayerFile::toggleCheckBox(bool state)
{
  //todo
}

QString QgsLegendLayerFile::nameFromLayer(QgsMapLayer* layer)
{
  QString sourcename = layer->source(); //todo: move this duplicated code into a new function
  if(sourcename.startsWith("host", false))
    {
      //this layer is a database layer
      //modify source string such that password is not visible
      sourcename = layer->name();
    }
  else
    {
      //modify source name such that only the file is visible
      sourcename = layer->source().section('/',-1,-1);
    }
  return sourcename;
}

