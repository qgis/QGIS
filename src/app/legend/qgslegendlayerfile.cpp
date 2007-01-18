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

#include "qgsapplication.h"
#include "qgslegend.h"
#include "qgslegendlayer.h"
#include "qgslegendlayerfile.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

// attribute table
#include "qgsattributetable.h"
#include "qgsattributetabledisplay.h"

#include "qgsencodingfiledialog.h"

#include <QApplication>
#include <QMessageBox>
#include <QPainter>
#include <QSettings>


QgsLegendLayerFile::QgsLegendLayerFile(QTreeWidgetItem * theLegendItem, QString theString, QgsMapLayer* theLayer)
  : QgsLegendItem(theLegendItem, theString), mLyr(theLayer), mTableDisplay(NULL)
{
  // Set the initial visibility flag for layers
  // This user option allows the user to turn off inital drawing of
  // layers when they are added to the map. This is useful when adding
  // many layers and the user wants to adjusty symbology, etc prior to
  // actually viewing the layer.
  QSettings settings;
  bool visible = settings.readBoolEntry("/qgis/new_layers_visible", 1);
  mLyr.setVisible(visible);

  // not in overview by default
  mLyr.setInOverview(FALSE);
  
  mType = LEGEND_LAYER_FILE;
  
  setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  setCheckState(0, Qt::Checked);
  setText(0, theString);

  // get notifications of changed selection - used to update attribute table
  connect(mLyr.layer(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));

  // get notifications of modified layer - used to close table as it's out of sync
  connect(mLyr.layer(), SIGNAL(wasModified(bool)), this, SLOT(closeTable(bool)));
  
  connect(mLyr.layer(), SIGNAL(layerNameChanged()), this, SLOT(layerNameChanged()));
}

QgsLegendLayerFile::~QgsLegendLayerFile()
{
  if (mTableDisplay)
  {
    mTableDisplay->close();
    delete mTableDisplay;
  }
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
  QPixmap myPixmap(QgsApplication::themePath()+"mActionFileSmall.png");
  return myPixmap;
}

void QgsLegendLayerFile::updateLegendItem()
{
  QPixmap pix = legend()->pixmaps().mOriginalPixmap;
  
  if(mLyr.inOverview())
  {
    //add overview glasses to the pixmap
    QPainter p(&pix);
    p.drawPixmap(0,0, legend()->pixmaps().mInOverviewPixmap);
  }
  if(mLyr.layer()->isEditable())
  {
    //add editing icon to the pixmap
    QPainter p(&pix);
    p.drawPixmap(30,0, legend()->pixmaps().mEditablePixmap);
  }

  /*
  // TODO:
  if(mLyr.layer()->hasProjectionError())
  {
    //add overview glasses to the pixmap
    QPainter p(&pix);
    p.drawPixmap(60,0, legend()->pixmaps().mProjectionErrorPixmap);
  }
  */
  
  QIcon theIcon(pix);
  setIcon(0, theIcon);

}

void QgsLegendLayerFile::setIconAppearance(bool inOverview,
                                           bool editable)
{
  QPixmap newIcon(getOriginalPixmap());

  if (inOverview)
  {
    // Overlay the overview icon on the default icon
    QPixmap myPixmap(QgsApplication::themePath()+"mIconOverview.png");
    QPainter p(&newIcon);
    p.drawPixmap(0,0,myPixmap);
    p.end();
  }
  
  if (editable)
  {
    // Overlay the editable icon on the default icon
    QPixmap myPixmap(QgsApplication::themePath()+"mIconEditable.png");
    QPainter p(&newIcon);
    p.drawPixmap(0,0,myPixmap);
    p.end();
  }

  QIcon theIcon(newIcon);
  setIcon(0, theIcon);

  //also update the icon of the legend layer
  ((QgsLegendLayer*)(parent()->parent()))->updateIcon();
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


void QgsLegendLayerFile::setVisible(bool visible)
{
  mLyr.setVisible(visible);
}

bool QgsLegendLayerFile::isVisible()
{
  return mLyr.visible();
}

void QgsLegendLayerFile::setInOverview(bool inOverview)
{
  mLyr.setInOverview(inOverview);
}

bool QgsLegendLayerFile::isInOverview()
{
  return mLyr.inOverview();
}


void QgsLegendLayerFile::table()
{
  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>(mLyr.layer());
  if (!vlayer)
    return;
  
  QgsAttributeAction& actions = *vlayer->actions();
  
  if (mTableDisplay)
  {
    
    mTableDisplay->raise();

    // Give the table the most recent copy of the actions for this layer.
    mTableDisplay->table()->setAttributeActions(actions);
  }
  else
  {
    // display the attribute table
    QApplication::setOverrideCursor(Qt::waitCursor);
    // TODO: pointer to QgisApp should be passed instead of NULL
    // but we don't have pointer to it. [MD]
    mTableDisplay = new QgsAttributeTableDisplay(vlayer, NULL);
    mTableDisplay->table()->fillTable(vlayer);
    mTableDisplay->table()->setSorting(true);

    connect(mTableDisplay, SIGNAL(deleted()), this, SLOT(invalidateTableDisplay()));

    mTableDisplay->setTitle(tr("Attribute table - ") + name());
    mTableDisplay->show();

    // Give the table the most recent copy of the actions for this layer.
    mTableDisplay->table()->setAttributeActions(actions);
    
    // select rows which should be selected
    selectionChanged();
    
    // etablish the necessary connections between the table and the vector layer
    connect(mTableDisplay->table(), SIGNAL(selected(int)), mLyr.layer(), SLOT(select(int)));
    connect(mTableDisplay->table(), SIGNAL(selectionRemoved()), mLyr.layer(), SLOT(removeSelection()));
    connect(mTableDisplay->table(), SIGNAL(repaintRequested()), mLyr.layer(), SLOT(triggerRepaint()));
    
    QApplication::restoreOverrideCursor();
  }

}

void QgsLegendLayerFile::invalidateTableDisplay()
{
  // from signal deleted() - table doesn't exist anymore, just erase our pointer
  mTableDisplay = 0;
}

void QgsLegendLayerFile::selectionChanged()
{
  if (!mTableDisplay)
    return;

  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>(mLyr.layer());
  const QgsFeatureIds& ids = vlayer->selectedFeaturesIds();
  mTableDisplay->table()->selectRowsWithId(ids);

}

void QgsLegendLayerFile::closeTable(bool onlyGeometryWasChanged)
{
  if (mTableDisplay)
  {
    mTableDisplay->close();
    delete mTableDisplay;
    mTableDisplay = NULL;
  }
}

void QgsLegendLayerFile::saveAsShapefile()
{
  if (mLyr.layer()->type() != QgsMapLayer::VECTOR)
    return;

  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>(mLyr.layer());
  
  // get a name for the shapefile
  // Get a file to process, starting at the current directory
  QSettings settings;
  QString filter =  QString("Shapefiles (*.shp)");
  QString dirName = settings.readEntry("/UI/lastShapefileDir", ".");

  QgsEncodingFileDialog* openFileDialog = new QgsEncodingFileDialog(0,
      tr("Save layer as..."),
      dirName,
      filter,
      QString("UTF-8"));

  // allow for selection of more than one file
  //openFileDialog->setMode(QFileDialog::AnyFile);

  if (openFileDialog->exec() != QDialog::Accepted)
    return;
    
  QString enc = openFileDialog->encoding();
  QString shapefileName = openFileDialog->selectedFile();
  
  if (shapefileName.isNull())
    return;

  // add the extension if not present
  if (shapefileName.find(".shp") == -1)
  {
    shapefileName += ".shp";
  }
  
  QString error = vlayer->saveAsShapefile(shapefileName, enc);
  
  if (error == "DRIVER_NOT_FOUND")
  {
    QMessageBox::warning(0, tr("Driver not found"), tr("ESRI Shapefile driver is not available"));
  }
  else if (error == "ERROR_CREATE_SOURCE")
  {
    QMessageBox::warning(0, tr("Error creating shapefile"),
                         tr("The shapefile could not be created (") +
                             shapefileName + ")");
  }
  else if (error == "ERROR_CREATE_LAYER")
  {
    QMessageBox::warning(0, tr("Error"), tr("Layer creation failed"));
  }
  else
  {
    QMessageBox::information(0, tr("Saving done"), tr("Export to Shapefile has been completed"));
  }
}

void QgsLegendLayerFile::toggleEditing()
{
  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>(mLyr.layer());
  if (!vlayer)
    return;
  
  if (!vlayer->isEditable())
  {
    vlayer->startEditing();
    if(!(vlayer->getDataProvider()->capabilities() & QgsVectorDataProvider::AddFeatures))
    {
      QMessageBox::information(0,tr("Start editing failed"),
                               tr("Provider cannot be opened for editing"));
    }
  }
  else
  {
    // commit or roll back?
    QMessageBox::StandardButton commit = QMessageBox::information(0,tr("Stop editing"),
                                          tr("Do you want to save the changes?"),
                                          QMessageBox::Save | QMessageBox::Discard);  

    if(commit==QMessageBox::Save)
    {
      if(!vlayer->commitChanges())
      {
        QMessageBox::information(0,tr("Error"),tr("Could not commit changes"));
      
        // Leave the in-memory editing state alone,
        // to give the user a chance to enter different values
        // and try the commit again later
      }
    }
    else if(commit==QMessageBox::Discard)
    {
      if(!vlayer->rollBack())
      {
        QMessageBox::information(0,tr("Error"),
                                 tr("Problems during roll back"));
      }
    }
    vlayer->triggerRepaint();
    
  }
  
  updateLegendItem();

}

void QgsLegendLayerFile::layerNameChanged()
{
  QString name = mLyr.layer()->name();
  setText(0, name);

  // set also parent's name
  legend()->setName(this, name);
}
