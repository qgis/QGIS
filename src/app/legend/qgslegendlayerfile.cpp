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
#include "qgsrasterlayer.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

// attribute table
#include "qgsattributetable.h"
#include "qgsattributetabledisplay.h"

#include "qgsencodingfiledialog.h"

#include <QApplication>
#include <QMenu>
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

  // Add check if vector layer when connecting to selectionChanged slot
  // Ticket #811 - racicot
  QgsMapLayer *currentLayer = mLyr.layer();
  QgsVectorLayer *isVectLyr = dynamic_cast < QgsVectorLayer * >(currentLayer);
  if (isVectLyr)
  {
    // get notifications of changed selection - used to update attribute table
    connect(mLyr.layer(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    // get notifications of modified layer - used to close table as it's out of sync
    connect(mLyr.layer(), SIGNAL(wasModified(bool)), this, SLOT(closeTable(bool)));
  }  
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

void QgsLegendLayerFile::showInOverview()
{
  // toggle current status
  setInOverview( ! isInOverview() );
  
  legend()->updateMapCanvasLayerSet();
  legend()->updateOverview();
}


void QgsLegendLayerFile::table()
{
  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>(mLyr.layer());
  if (!vlayer)
  {
    QMessageBox::information(0, tr("Not a vector layer"),
      tr("To open an attribute table, you must select a vector layer in the legend"));
    return;
  }

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
    // but be can get it using this ugly hack. [jef]
    // TODO: do this cleanly
    QgisApp *app = NULL;

    QList<QWidget *> list = QApplication::topLevelWidgets();

    int i;
    for(i=0; i<list.size(); i++)
      if( list[i]->windowTitle().startsWith("Quantum GIS") )
      {
        app=reinterpret_cast<QgisApp*>(list[i]);
        break;
      }

      mTableDisplay = new QgsAttributeTableDisplay(vlayer, app);
      try
      {
        mTableDisplay->table()->fillTable(vlayer);
      }
      catch(std::bad_alloc& ba)
      {
        Q_UNUSED(ba);
        QMessageBox::critical(0, tr("bad_alloc exception"), tr("Filling the attribute table has been stopped because there was no more virtual memory left"));
      }

      connect(mTableDisplay, SIGNAL(deleted()), this, SLOT(invalidateTableDisplay()));

      mTableDisplay->setTitle(tr("Attribute table - ") + vlayer->name());
      mTableDisplay->show();

      // Give the table the most recent copy of the actions for this layer.
      mTableDisplay->table()->setAttributeActions(actions);

      // select rows which should be selected
      selectionChanged();

      // etablish the necessary connections between the table and the vector layer
      connect(mTableDisplay->table(), SIGNAL(selected(int, bool)), mLyr.layer(), SLOT(select(int, bool)));
      connect(mTableDisplay->table(), SIGNAL(selectionRemoved(bool)), mLyr.layer(), SLOT(removeSelection(bool)));
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
  saveAsShapefileGeneral(FALSE);
}

void QgsLegendLayerFile::saveSelectionAsShapefile()
{
  saveAsShapefileGeneral(TRUE);
}

void QgsLegendLayerFile::saveAsShapefileGeneral(bool saveOnlySelection)
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
  openFileDialog->setAcceptMode(QFileDialog::AcceptSave);

  // allow for selection of more than one file
  //openFileDialog->setMode(QFileDialog::AnyFile);

  if (openFileDialog->exec() != QDialog::Accepted)
    return;
    
  
  QString encoding = openFileDialog->encoding();
  QString shapefileName = openFileDialog->selectedFiles().first();
  settings.writeEntry("/UI/lastShapefileDir", QFileInfo(shapefileName).absolutePath());
  
  
  if (shapefileName.isNull())
    return;

  // add the extension if not present
  if (shapefileName.find(".shp") == -1)
  {
    shapefileName += ".shp";
  }
  
  // overwrite the file - user will already have been prompted
  // to verify they want to overwrite by the file dialog above
  if (QFile::exists(shapefileName))
  {
      if (!QgsVectorFileWriter::deleteShapeFile(shapefileName))
      {
        return;
      }
  }
  // ok if the file existed it should be deleted now so we can continue...
  QApplication::setOverrideCursor(Qt::waitCursor);
  
  QgsVectorFileWriter::WriterError error;
  error = QgsVectorFileWriter::writeAsShapefile(vlayer, shapefileName, encoding, saveOnlySelection);
  
  QApplication::restoreOverrideCursor();
  
  switch (error)
  {
    case QgsVectorFileWriter::NoError:
      QMessageBox::information(0, tr("Saving done"), tr("Export to Shapefile has been completed"));
      break;
    
    case QgsVectorFileWriter::ErrDriverNotFound:
      QMessageBox::warning(0, tr("Driver not found"), tr("ESRI Shapefile driver is not available"));
      break;
  
    case QgsVectorFileWriter::ErrCreateDataSource:
      QMessageBox::warning(0, tr("Error creating shapefile"),
                           tr("The shapefile could not be created (") + shapefileName + ")");
      break;
    
    case QgsVectorFileWriter::ErrCreateLayer:
      QMessageBox::warning(0, tr("Error"), tr("Layer creation failed"));
      break;
    case QgsVectorFileWriter::ErrAttributeTypeUnsupported:
      QMessageBox::warning(0, tr("Error"), 
          tr("Layer attribute table contains unsupported datatype(s)"));
      break;
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
    else
    {
      vlayer->triggerRepaint();
    }
  }
  else
  {
    if(vlayer->isModified())
    {

      // commit or roll back?
      QMessageBox::StandardButton commit = QMessageBox::information(0,tr("Stop editing"), tr("Do you want to save the changes?"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);  

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
      else //cancel
	{
	  return;
	}
    }
    else //layer not modified
    {
      vlayer->rollBack();
    }
    vlayer->triggerRepaint();

  }

  updateLegendItem();

}

bool QgsLegendLayerFile::isEditing()
{
  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>(mLyr.layer());
  return vlayer && vlayer->isEditable();
} 

void QgsLegendLayerFile::layerNameChanged()
{
  QString name = mLyr.layer()->name();
  setText(0, name);

  // set also parent's name
  legend()->setName(this, name);
}


void QgsLegendLayerFile::addToPopupMenu(QMenu& theMenu, QAction* toggleEditingAction)
{
  QgsMapLayer* lyr = layer();
  QString iconsPath = QgsApplication::themePath();

  // zoom to layer extent
  theMenu.addAction(QIcon(iconsPath+QString("/mActionZoomToLayer.png")),
                    tr("&Zoom to layer extent"), legend(), SLOT(legendLayerZoom()));
  
  // show in overview
  QAction* showInOverviewAction = theMenu.addAction(tr("&Show in overview"), this, SLOT(showInOverview()));
  showInOverviewAction->setCheckable(true);
  showInOverviewAction->blockSignals(true);
  showInOverviewAction->setChecked(mLyr.inOverview());
  showInOverviewAction->blockSignals(false);
  
  // remove from canvas
  theMenu.addAction(QIcon(iconsPath+QString("/mActionRemove.png")),
                    tr("&Remove"), legend(), SLOT(legendLayerRemove()));

  theMenu.addSeparator();

  if (lyr->type() == QgsMapLayer::VECTOR)
  {
    QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>(lyr);
    
    // attribute table
    theMenu.addAction(tr("&Open attribute table"), this, SLOT(table()));
    
    // editing
    int cap = vlayer->getDataProvider()->capabilities();
    if ((cap & QgsVectorDataProvider::AddFeatures)
        ||(cap & QgsVectorDataProvider::DeleteFeatures))
    {
      if(toggleEditingAction)
	{
	  theMenu.addAction(toggleEditingAction);
	}
    }
    
    // save as shapefile
    theMenu.addAction(tr("Save as shapefile..."), this, SLOT(saveAsShapefile()));

    QAction* saveSelectionAction = theMenu.addAction(tr("Save selection as shapefile..."), this, SLOT(saveSelectionAsShapefile()));
    if (vlayer->selectedFeatureCount() == 0)
    {
      saveSelectionAction->setEnabled(false);
    }

    theMenu.addSeparator();
  }
  else if (lyr->type() == QgsMapLayer::RASTER)
  {
    // TODO: what was this for?
    //QgsRasterLayer* rlayer = dynamic_cast<QgsRasterLayer*>(lyr);
    //theMenu.addAction(tr("&Convert to..."), rlayer, SLOT(convertTo()));
  }
     
  // properties goes on bottom of menu for consistency with normal ui standards
  // e.g. kde stuff
  theMenu.addAction(tr("&Properties"), legend(), SLOT(legendLayerShowProperties()));
}
