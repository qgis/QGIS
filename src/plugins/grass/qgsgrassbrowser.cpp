/*******************************************************************
                              qgsgrassbrowser.cpp
                             -------------------
    begin                : February, 2006
    copyright            : (C) 2006 by Radim Blazek
    email                : radim.blazek@gmail.com
********************************************************************/
/********************************************************************
 This program is free software; you can redistribute it and/or modify  
 it under the terms of the GNU General Public License as published by 
 the Free Software Foundation; either version 2 of the License, or     
 (at your option) any later version.                                   
*******************************************************************/
#include <iostream>
#include <vector>

#include <QApplication>
#include <QStyle>
#include <qdir.h>
#include <qfile.h>
#include <qsettings.h>
#include <qstringlist.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qnamespace.h>
#include <qevent.h>
#include <qsize.h>
#include <qicon.h>
#include <QTreeView>
#include <QHeaderView>
#include <QMainWindow>
#include <QActionGroup>
#include <QToolBar>
#include <QAction>
#include <QTextBrowser>
#include <QSplitter>
#include <QProcess>
#include <QScrollBar>

#include "qgis.h"
#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgsrasterlayer.h"

extern "C" {
#include <grass/gis.h>
#include <grass/Vect.h>
}

#include "../../src/providers/grass/qgsgrass.h"
#include "qgsgrassmodel.h"
#include "qgsgrassbrowser.h"
#include "qgsgrassselect.h"
#include "qgsgrassutils.h"

QgsGrassBrowser::QgsGrassBrowser ( QgisInterface *iface, 
	 QWidget * parent, Qt::WFlags f )
     : QMainWindow(parent, Qt::WType_Dialog), mIface(iface)
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassBrowser()" << std::endl;
#endif

  QActionGroup *ag = new QActionGroup ( this );
  QToolBar *tb = addToolBar(tr("Tools"));

  mActionAddMap = new QAction( 
    getThemeIcon("grass_add_map.png"), 
    tr("Add selected map to canvas"), this);
  mActionAddMap->setEnabled(false); 
  ag->addAction ( mActionAddMap );
  tb->addAction ( mActionAddMap );
  connect ( mActionAddMap, SIGNAL(triggered()), this, SLOT(addMap()) );

  mActionCopyMap = new QAction( 
    getThemeIcon("grass_copy_map.png"), 
    tr("Copy selected map"), this);
  mActionCopyMap->setEnabled(false); 
  ag->addAction ( mActionCopyMap );
  tb->addAction ( mActionCopyMap );
  connect ( mActionCopyMap, SIGNAL(triggered()), this, SLOT(copyMap()) );

  mActionRenameMap = new QAction( 
    getThemeIcon("grass_rename_map.png"), 
    tr("Rename selected map"), this);
  mActionRenameMap->setEnabled(false); 
  ag->addAction ( mActionRenameMap );
  tb->addAction ( mActionRenameMap );
  connect ( mActionRenameMap, SIGNAL(triggered()), this, SLOT(renameMap()) );

  mActionDeleteMap = new QAction( 
    getThemeIcon("grass_delete_map.png"), 
    tr("Delete selected map"), this);
  mActionDeleteMap->setEnabled(false); 
  ag->addAction ( mActionDeleteMap );
  tb->addAction ( mActionDeleteMap );
  connect ( mActionDeleteMap, SIGNAL(triggered()), this, SLOT(deleteMap()) );

  mActionSetRegion = new QAction( 
    getThemeIcon("grass_set_region.png"), 
    tr("Set current region to selected map"), this);
  mActionSetRegion->setEnabled(false); 
  ag->addAction ( mActionSetRegion );
  tb->addAction ( mActionSetRegion );
  connect ( mActionSetRegion, SIGNAL(triggered()), this, SLOT(setRegion()) );

  mActionRefresh = new QAction( 
    getThemeIcon("grass_refresh.png"), 
    tr("Refresh"), this);
  ag->addAction ( mActionRefresh );
  tb->addAction ( mActionRefresh );
  connect ( mActionRefresh, SIGNAL(triggered()), this, SLOT(refresh()) );

  // Add model
  mModel = new QgsGrassModel ( this );

  mTree = new QTreeView(0);
  mTree->header()->hide();
  mTree->setModel(mModel);

  mTextBrowser = new QTextBrowser(0);
  mTextBrowser->setTextFormat(Qt::RichText);
  mTextBrowser->setReadOnly(TRUE);

  mSplitter = new QSplitter(0);
  mSplitter->addWidget(mTree);
  mSplitter->addWidget(mTextBrowser);

  this->setCentralWidget(mSplitter);

  connect ( mTree->selectionModel(), 
    SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
    this, SLOT(selectionChanged(QItemSelection,QItemSelection)) );

  connect ( mTree->selectionModel(), 
    SIGNAL(currentChanged(QModelIndex,QModelIndex)),
    this, SLOT(currentChanged(QModelIndex,QModelIndex)) );

  connect ( mTree, SIGNAL(doubleClicked(QModelIndex)),
    this, SLOT(doubleClicked(QModelIndex)) );
}

QgsGrassBrowser::~QgsGrassBrowser() { }

void QgsGrassBrowser::refresh()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassBrowser::refresh()" << std::endl;
#endif

  mModel->refresh();
  mTree->update();
}

void QgsGrassBrowser::addMap()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassBrowser::addMap()" << std::endl;
#endif

  QModelIndexList indexes = mTree->selectionModel()->selectedIndexes();
  bool mapSelected = false;

  QList<QModelIndex>::const_iterator it = indexes.begin();
  for (; it != indexes.end(); ++it)
  {
    int type = mModel->itemType(*it);
    QString uri = mModel->uri(*it);
    QString mapset = mModel->itemMapset(*it);
    QString map = mModel->itemMap(*it);
    if ( type == QgsGrassModel::Raster )
    {
      std::cerr << "add raster: " << uri.ascii() << std::endl;
      mIface->addRasterLayer(uri, map);
      mapSelected = true;
    }
    else if ( type == QgsGrassModel::Vector )
    {
      QgsGrassUtils::addVectorLayers ( mIface, 
        QgsGrass::getDefaultGisdbase(),
        QgsGrass::getDefaultLocation(),
        mapset, map );
    } 
    else if ( type == QgsGrassModel::VectorLayer )
    {

      QStringList list = QgsGrassSelect::vectorLayers(
        QgsGrass::getDefaultGisdbase(),
        QgsGrass::getDefaultLocation(),
        mModel->itemMapset(*it), map );

      // TODO: common method for vector names
      QStringList split = QStringList::split ( '/', uri );
      QString layer = split.last();

      QString name = QgsGrassUtils::vectorLayerName ( 
        map, layer, list.size() );

      mIface->addVectorLayer( uri, name, "grass");
      mapSelected = true;
    }
    else if ( type == QgsGrassModel::Region )
    {
      struct Cell_head window;
      if ( !getItemRegion (*it, &window) ) continue;
      writeRegion ( &window );
    }
  }
}

void QgsGrassBrowser::doubleClicked(const QModelIndex & index)
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassBrowser::doubleClicked()" << std::endl;
#endif

  addMap();
}

QString QgsGrassBrowser::formatMessage( QString msg )
{
  return msg.replace("<","&lt;").replace(">","&gt;").replace("\n","<br>");
}

void QgsGrassBrowser::copyMap()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassBrowser::copyMap()" << std::endl;
#endif

  QModelIndexList indexes = mTree->selectionModel()->selectedIndexes();

  QList<QModelIndex>::const_iterator it = indexes.begin();
  for (; it != indexes.end(); ++it)
  {
    int type = mModel->itemType(*it);
    QString mapset = mModel->itemMapset(*it);
    QString map = mModel->itemMap(*it);

    QString typeName;
    QString element;
    if ( type == QgsGrassModel::Raster ) 
    {
      element = "cell";
      typeName = "rast";
    } 
    else if ( type == QgsGrassModel::Vector )
    {
      element = "vector";
      typeName = "vect";
    }
    else if ( type == QgsGrassModel::Region ) 
    {
      element = "windows";
      typeName = "region";
    }

    QgsGrassElementDialog ed;
    bool ok;
    QString source;
    QString suggest;
    if ( mapset == QgsGrass::getDefaultMapset() )
    {
      source = map;
    } 
    else
    {
      suggest = map;
    }
    QString newName = ed.getItem ( element, tr("New name"),
      tr("New name"), suggest, source, &ok );

    if ( !ok ) return;

    QString module = "g.copy";
#ifdef WIN32
    module.append(".exe");
#endif
    QProcess process(this);
    QStringList args(typeName + "=" + map + "@" + mapset + "," + newName );
    process.start(module, args );
    if ( !process.waitForFinished() || process.exitCode() != 0 )
    {
      QString output ( process.readAllStandardOutput () );
      QString error ( process.readAllStandardError () );
      QMessageBox::warning( 0, tr("Warning"), tr("Cannot copy map ")
        + map + "@" + mapset 
        + tr("<br>command: ") + module + " " + args.join(" ")
        + "<br>" + formatMessage(output)
        + "<br>" + formatMessage(error) ); 
    }
    else
    {
      refresh();
    }
  }
}

void QgsGrassBrowser::renameMap()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassBrowser::renameMap()" << std::endl;
#endif

  QModelIndexList indexes = mTree->selectionModel()->selectedIndexes();

  QList<QModelIndex>::const_iterator it = indexes.begin();
  for (; it != indexes.end(); ++it)
  {
    int type = mModel->itemType(*it);
    QString mapset = mModel->itemMapset(*it);
    QString map = mModel->itemMap(*it);

    if ( mapset != QgsGrass::getDefaultMapset() ) continue; // should not happen

    QString typeName;
    QString element;
    if ( type == QgsGrassModel::Raster ) 
    {
      element = "cell";
      typeName = "rast";
    } 
    else if ( type == QgsGrassModel::Vector )
    {
      element = "vector";
      typeName = "vect";
    }
    else if ( type == QgsGrassModel::Region ) 
    {
      element = "windows";
      typeName = "region";
    }

    QgsGrassElementDialog ed;
    bool ok;
    QString newName = ed.getItem ( element, tr("New name"),
      tr("New name"), "", map, &ok );

    if ( !ok ) return;

    QString module = "g.rename";
#ifdef WIN32
    module.append(".exe");
#endif
    QProcess process(this);
    QStringList args(typeName + "=" + map + "," + newName );
    process.start(module, QStringList( typeName + "=" + map + "," + newName ) );
    if ( !process.waitForFinished() || process.exitCode() != 0 )
    {
      QString output ( process.readAllStandardOutput () );
      QString error ( process.readAllStandardError () );
      QMessageBox::warning( 0, tr("Warning"), tr("Cannot rename map ")
        + map  
        + tr("<br>command: ") + module + " " + args.join(" ")
        + "<br>" + formatMessage(output)
        + "<br>" + formatMessage(error) ); 
    }
    else
    {
      refresh();
    }
  }
}

void QgsGrassBrowser::deleteMap()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassBrowser::deleteMap()" << std::endl;
#endif

  QModelIndexList indexes = mTree->selectionModel()->selectedIndexes();

  QList<QModelIndex>::const_iterator it = indexes.begin();
  for (; it != indexes.end(); ++it)
  {
    int type = mModel->itemType(*it);
    QString mapset = mModel->itemMapset(*it);
    QString map = mModel->itemMap(*it);

    QString typeName;
    if ( type == QgsGrassModel::Raster ) typeName = "rast";
    else if ( type == QgsGrassModel::Vector ) typeName = "vect";
    else if ( type == QgsGrassModel::Region ) typeName = "region";

    if ( mapset != QgsGrass::getDefaultMapset() ) 
    {
      continue; // should not happen
    }

    QMessageBox::StandardButton ret = QMessageBox::question ( 0, tr("Warning"),
      tr("Delete map <b>") + map + "</b>",
      QMessageBox::Ok | QMessageBox::Cancel );

    if ( ret == QMessageBox::Cancel ) continue;

    QString module = "g.remove";
#ifdef WIN32
    module.append(".exe");
#endif
    QProcess process(this);
    QStringList args(typeName + "=" + map );
    process.start(module, QStringList( typeName + "=" + map ) );
    if ( !process.waitForFinished() || process.exitCode() != 0 )
    {
      QString output ( process.readAllStandardOutput () );
      QString error ( process.readAllStandardError () );
      QMessageBox::warning( 0, tr("Warning"), tr("Cannot delete map ")
        + map  
        + tr("<br>command: ") + module + " " + args.join(" ")
        + "<br>" + formatMessage(output)
        + "<br>" + formatMessage(error) ); 
    }
    else
    {
      refresh();
    }
  }
}

void QgsGrassBrowser::setRegion()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassBrowser::setRegion()" << std::endl;
#endif

  struct Cell_head window;

  QModelIndexList indexes = mTree->selectionModel()->selectedIndexes();

  // TODO multiple selection - extent region to all maps
  QList<QModelIndex>::const_iterator it = indexes.begin();
  for (; it != indexes.end(); ++it)
  {
    if ( !getItemRegion (*it, &window) ) return;
  }
  writeRegion ( &window );
}

void QgsGrassBrowser::writeRegion(struct Cell_head *window )
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassBrowser::writeRegion()" << std::endl;
#endif

  QgsGrass::setMapset( QgsGrass::getDefaultGisdbase(),
    QgsGrass::getDefaultLocation(),
    QgsGrass::getDefaultMapset() );

  if ( G_put_window ( window ) == -1 )
  { 
    QMessageBox::warning( 0, tr("Warning"), 
      tr("Cannot write new region") ); 
    return;
  }
  emit regionChanged();
}

bool QgsGrassBrowser::getItemRegion( QModelIndex index, struct Cell_head *window )
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassBrowser::setRegion()" << std::endl;
#endif

  int type = mModel->itemType(index);
  QString mapset = mModel->itemMapset(index);
  QString map = mModel->itemMap(index);

  int mapType = QgsGrass::Raster; //default in case no case matches
  switch (type) {
        case QgsGrassModel::Raster :
          mapType = QgsGrass::Raster;
          break;
        case QgsGrassModel::Vector :
          mapType = QgsGrass::Vector;
          break;
        case QgsGrassModel::Region :
          mapType = QgsGrass::Region;
          break;
        default:
          break;
  }

  return QgsGrass::mapRegion ( mapType, QgsGrass::getDefaultGisdbase(),
    QgsGrass::getDefaultLocation(), mapset, map, window ); 
}

void QgsGrassBrowser::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassBrowser::selectionChanged()" << std::endl;
#endif

  mActionAddMap->setEnabled(false);
  mActionCopyMap->setEnabled(false);
  mActionRenameMap->setEnabled(false);
  mActionDeleteMap->setEnabled(false);
  mActionSetRegion->setEnabled(false);

  QModelIndexList indexes = mTree->selectionModel()->selectedIndexes();

  mTextBrowser->clear();

  QList<QModelIndex>::const_iterator it = indexes.begin();
  for (; it != indexes.end(); ++it)
  {
    mTextBrowser->append ( mModel->itemInfo(*it) );
    mTextBrowser->verticalScrollBar()->setValue(0);

    int type = mModel->itemType(*it);
    if ( type == QgsGrassModel::Raster || 
      type == QgsGrassModel::Vector || 
      type == QgsGrassModel::VectorLayer )
    {
      mActionAddMap->setEnabled(true);
    }
    if ( type == QgsGrassModel::Raster || type == QgsGrassModel::Vector || type == QgsGrassModel::Region )
    {
      mActionSetRegion->setEnabled(true);
      mActionCopyMap->setEnabled(true);

      QString mapset = mModel->itemMapset(*it);
      if ( mapset == QgsGrass::getDefaultMapset() ) 
      {
        mActionDeleteMap->setEnabled(true);
        mActionRenameMap->setEnabled(true);
      }
    }
  }
}

void QgsGrassBrowser::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassBrowser::currentChanged()" << std::endl;
#endif
}

void QgsGrassBrowser::setLocation( const QString &gisbase, const QString &location )
{
  mModel->setLocation(gisbase, location);
}
