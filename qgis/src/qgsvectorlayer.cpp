/***************************************************************************
                               qgsvectorlayer.cpp
  This class implements a generic means to display vector layers. The features
  and attributes are read from the data store using a "data provider" plugin.
  QgsVectorLayer can be used with any data store for which an appropriate
  plugin is available.
                              -------------------
          begin                : Oct 29, 2003
          copyright            : (C) 2003 by Gary E.Sherman
          email                : sherman at mrcc.com
 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id$ */

#include <iostream>
#include <iosfwd>
#include <cfloat>
#include <cstring>
#include <sstream>
#include <memory>

// for htonl
#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

#include <qapplication.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qpointarray.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qlibrary.h>
#include <qpicture.h>
#include <qsettings.h>

#include "qgisapp.h"
#include "qgsproject.h"
#include "qgsrect.h"
#include "qgspoint.h"
#include "qgsmaptopixel.h"
#include "qgsvectorlayer.h"
#include "qgsidentifyresults.h"
#include "qgsattributetable.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgslegenditem.h"
#include "qgsdlgvectorlayerproperties.h"
#include "qgsrenderer.h"
#include "qgssinglesymrenderer.h"
#include "qgsgraduatedsymrenderer.h"
#include "qgscontinuouscolrenderer.h"
#include "qgsgraduatedmarenderer.h"
#include "qgssimarenderer.h"
#include "qgsuniquevalrenderer.h"
#include "qgsuvalmarenderer.h"
#include "qgsrenderitem.h"
#include "qgssisydialog.h"
#include "qgsproviderregistry.h"
#include "qgsrect.h"
#include "qgslabelattributes.h"
#include "qgslabel.h"
#include "qgscoordinatetransform.h"
#include "qgsattributedialog.h"
//#include "wkbheader.h"

#ifdef TESTPROVIDERLIB
#include <dlfcn.h>
#endif


static const char * const ident_ = "$Id$";

// typedef for the QgsDataProvider class factory
typedef QgsDataProvider * create_it(const char * uri);



QgsVectorLayer::QgsVectorLayer(QString vectorLayerPath,
                               QString baseName,
                               QString providerKey)
    : QgsMapLayer(VECTOR, baseName, vectorLayerPath),
    providerKey(providerKey),
    tabledisplay(0),
    m_renderer(0),
    m_propertiesDialog(0),
    m_rendererDialog(0),
    ir(0),                    // initialize the identify results pointer
    mEditable(false),
    mModified(false)
{

  // if we're given a provider type, try to create and bind one to this layer
  if ( ! providerKey.isEmpty() )
  {
    setDataProvider( providerKey );
  }

  // Default for the popup menu
  popMenu = 0;

  // Get the update threshold from user settings. We
  // do this only on construction to avoid the penality of
  // fetching this each time the layer is drawn. If the user
  // changes the threshold from the preferences dialog, it will
  // have no effect on existing layers
  QSettings settings;
  updateThreshold = settings.readNumEntry("qgis/map/updateThreshold", 1000);
} // QgsVectorLayer ctor



QgsVectorLayer::~QgsVectorLayer()
{
#ifdef QGISDEBUG
  std::cerr << "In QgsVectorLayer destructor" << std::endl;
#endif

  valid=false;

  if(isEditable())
  {
    stopEditing();
  }

  if (tabledisplay)
  {
    tabledisplay->close();
    delete tabledisplay;
  }
  if (m_renderer)
  {
    delete m_renderer;
  }
  if (m_rendererDialog)
  {
    delete m_rendererDialog;
  }
  if (m_propertiesDialog)
  {
    delete m_propertiesDialog;
  }
  // delete the provider object
  delete dataProvider;
  // delete the popu pmenu
  delete popMenu;
  // delete the provider lib pointer
  delete myLib;
}

bool QgsVectorLayer::projectionsEnabled()
{
  if (QgsProject::instance()->readNumEntry("SpatialRefSys","/ProjectionsEnabled",0)!=0)
  {
    return true;
  }
  else
  {
    return false;
  }
}
QString QgsVectorLayer::getProjectionWKT()
{
  //delegate to the provider
  if (valid)
  {
    return dataProvider->getProjectionWKT();
  }
  else
  {
    return NULL;
  }
}

QString QgsVectorLayer::providerType()
{
  return providerKey;
}

/**
 * sets the preferred display field based on some fuzzy logic
 */
void QgsVectorLayer::setDisplayField(QString fldName)
{
  // If fldName is provided, use it as the display field, otherwise
  // determine the field index for the feature column of the identify
  // dialog. We look for fields containing "name" first and second for
  // fields containing "id". If neither are found, the first field
  // is used as the node.
  QString idxName="";
  QString idxId="";

  std::vector < QgsField > fields = dataProvider->fields();
  if(!fldName.isEmpty())
  {
    // find the index for this field
    fieldIndex = fldName;
    /*
       for(int i = 0; i < fields.size(); i++)
       {
       if(QString(fields[i].name()) == fldName)
       {
       fieldIndex = i;
       break;
       }
       }
       */
  }
  else
  {
    int j = 0;
    for (int j = 0; j < fields.size(); j++)
    {

      QString fldName = fields[j].name();
#ifdef QGISDEBUG
      std::cerr << "Checking field " << fldName << " of " << fields.size() << " total" << std::endl;
#endif
      // Check the fields and keep the first one that matches.
      // We assume that the user has organized the data with the
      // more "interesting" field names first. As such, name should
      // be selected before oldname, othername, etc.
      if (fldName.find("name", false) > -1)
      {
        if(idxName.isEmpty())
        {
          idxName = fldName;
        }
      }
      if (fldName.find("descrip", false) > -1)
      {
        if(idxName.isEmpty())
        {
          idxName = fldName;
        }
      }
      if (fldName.find("id", false) > -1)
      {
        if(idxId.isEmpty())
        {
          idxId = fldName;
        }
      }
    }

    //if there were no fields in the dbf just return - otherwise qgis segfaults!
    if (fields.size() == 0) return;

    if (idxName.length() > 0)
    {
      fieldIndex = idxName;
    }
    else
    {
      if (idxId.length() > 0)
      {
        fieldIndex = idxId;
      }
      else
      {
        fieldIndex = fields[0].name();
      }
    }

    // set this to be the label field as well
    setLabelField(fieldIndex);
  }
}

void QgsVectorLayer::drawLabels(QPainter * p, QgsRect * viewExtent, QgsMapToPixel * theMapToPixelTransform, QPaintDevice* dst)
{
    drawLabels(p, viewExtent, theMapToPixelTransform, dst, 1.);
}

// NOTE this is a temporary method added by Tim to prevent label clipping
// which was occurring when labeller was called in the main draw loop
// This method will probably be removed again in the near future!
void QgsVectorLayer::drawLabels(QPainter * p, QgsRect * viewExtent, QgsMapToPixel * theMapToPixelTransform, QPaintDevice* dst, double scale)
{
#ifdef QGISDEBUG
  qWarning("Starting draw of labels");
#endif

  if ( /*1 == 1 */ m_renderer && mLabelOn)
  {
    std::list<int> attributes=m_renderer->classificationAttributes();
    // Add fields required for labels
    mLabel->addRequiredFields ( &attributes );

#ifdef QGISDEBUG
    qWarning("Selecting features based on view extent");
#endif

    int featureCount = 0;
    // select the records in the extent. The provider sets a spatial filter
    // and sets up the selection set for retrieval
    dataProvider->reset();
    dataProvider->select(viewExtent);

    //main render loop
    QgsFeature *fet;

    while((fet = dataProvider->getNextFeature(attributes)))
    {
      // Render label
      if ( fet != 0 )
      {
        if(mDeleted.find(fet->featureId())==mDeleted.end())//don't render labels of deleted features
        {
          bool sel=mSelected.find(fet->featureId()) != mSelected.end();
          mLabel->renderLabel ( p, viewExtent, theMapToPixelTransform, dst, fet, sel, 0, scale);
        }
      }
      delete fet;
      featureCount++;
    }

    //render labels of not-commited features
    for(std::list<QgsFeature*>::iterator it=mAddedFeatures.begin();it!=mAddedFeatures.end();++it)
    {
      bool sel=mSelected.find((*it)->featureId()) != mSelected.end();
      mLabel->renderLabel ( p, viewExtent, theMapToPixelTransform, dst, *it, sel, 0, scale);
    }



#ifdef QGISDEBUG
    std::cerr << "Total features processed is " << featureCount << std::endl;
#endif
    qApp->processEvents();

  }
}

void QgsVectorLayer::draw(QPainter * p, QgsRect * viewExtent, QgsMapToPixel * theMapToPixelTransform, QPaintDevice* dst)
{
    draw ( p, viewExtent, theMapToPixelTransform, dst, 1., 1. );
}

void QgsVectorLayer::draw(QPainter * p, QgsRect * viewExtent, QgsMapToPixel * theMapToPixelTransform, QPaintDevice* dst, double widthScale, double symbolScale)
{
  if ( /*1 == 1 */ m_renderer)
  {
    // painter is active (begin has been called
    /* Steps to draw the layer
       1. get the features in the view extent by SQL query
       2. read WKB for a feature
       3. transform
       4. draw
    */

    QPen pen;
    /*Pointer to a marker image*/
    QPicture marker;
    /*Scale factor of the marker image*/
    double markerScaleFactor=1.;

    // select the records in the extent. The provider sets a spatial filter
    // and sets up the selection set for retrieval
#ifdef QGISDEBUG
    qWarning("Selecting features based on view extent");
#endif
    dataProvider->reset();
    dataProvider->select(viewExtent);
    int featureCount = 0;
    //  QgsFeature *ftest = dataProvider->getFirstFeature();
#ifdef QGISDEBUG
    qWarning("Starting draw of features");
#endif
    QgsFeature *fet;
    unsigned char *feature;
    bool attributesneeded = m_renderer->needsAttributes();

    double *x;
    double *y;
    int *nPoints;
    int *numRings;
    int *numPolygons;
    int numPoints;
    int numLineStrings;
    int idx, jdx, kdx;
    unsigned char *ptr;
    char lsb;
    QgsPoint pt;
    QPointArray *pa;
    int wkbType;

    bool projectionsEnabledFlag = projectionsEnabled();
    std::list<int> attributes=m_renderer->classificationAttributes();

    mDrawingCancelled=false; //pressing esc will change this to true

    while((fet = dataProvider->getNextFeature(attributes)))
    {
      qApp->processEvents(); //so we can trap for esc press
      if (mDrawingCancelled) return;
      // If update threshold is greater than 0, check to see if
      // the threshold has been exceeded
      if(updateThreshold > 0)
      {
        //copy the drawing buffer every updateThreshold elements
        if(0 == featureCount % updateThreshold)
        {
          bitBlt(dst,0,0,p->device(),0,0,-1,-1,Qt::CopyROP,false);
        }
      }

      if (fet == 0)
      {
#ifdef QGISDEBUG
        std::cerr << "get next feature returned null\n";
#endif

      }
      else
      {
        if(mDeleted.find(fet->featureId())==mDeleted.end())
        {
          bool sel=mSelected.find(fet->featureId()) != mSelected.end();
          m_renderer->renderFeature(p, fet, &marker, &markerScaleFactor, sel);
	  double scale = markerScaleFactor * symbolScale;
          drawFeature(p,fet,theMapToPixelTransform,&marker, scale, widthScale, projectionsEnabledFlag);
          ++featureCount;
          delete fet;
        }
      }
    }
    //also draw the not yet commited features
    for(std::list<QgsFeature*>::iterator it=mAddedFeatures.begin();it!=mAddedFeatures.end();++it)
    {
      bool sel=mSelected.find((*it)->featureId()) != mSelected.end();
      m_renderer->renderFeature(p, fet, &marker, &markerScaleFactor, sel);
      double scale = markerScaleFactor * symbolScale;
      drawFeature(p,*it,theMapToPixelTransform,&marker,scale, widthScale, projectionsEnabledFlag);
    }

#ifdef QGISDEBUG
    std::cerr << "Total features processed is " << featureCount << std::endl;
#endif
    qApp->processEvents();
  }
  else
  {
#ifdef QGISDEBUG
    qWarning("Warning, QgsRenderer is null in QgsVectorLayer::draw()");
#endif

  }
}


QgsVectorLayer::endian_t QgsVectorLayer::endian()
{
  //     char *chkEndian = new char[4];
  //     memset(chkEndian, '\0', 4);
  //     chkEndian[0] = 0xE8;

  //     int *ce = (int *) chkEndian;
  //     int retVal;
  //     if (232 == *ce)
  //       retVal = NDR;
  //     else
  //       retVal = XDR;

  //     delete [] chkEndian;

  return (htonl(1) == 1) ? QgsVectorLayer::XDR : QgsVectorLayer::NDR ;
}

void QgsVectorLayer::identify(QgsRect * r)
{
  QApplication::setOverrideCursor(Qt::waitCursor);
  dataProvider->select(r, true);
  int featureCount = 0;
  QgsFeature *fet;
  unsigned char *feature;

  if ( !isEditable() ) {
      // display features falling within the search radius
      if(ir)
      {
	delete ir;
      }

      ir = 0;
      while ((fet = dataProvider->getNextFeature(true)))
      {
	featureCount++;
	if (featureCount == 1)
	{
	  ir = new QgsIdentifyResults(mActions);
	}

	QListViewItem *featureNode = ir->addNode("foo");
	featureNode->setText(0, fieldIndex);
	std::vector < QgsFeatureAttribute > attr = fet->attributeMap();
	for (int i = 0; i < attr.size(); i++)
	{
#ifdef QGISDEBUG
	  std::cout << attr[i].fieldName() << " == " << fieldIndex << std::endl;
#endif
	  if (attr[i].fieldName().lower() == fieldIndex)
	  {
	    featureNode->setText(1, attr[i].fieldValue());
	  }
	  ir->addAttribute(featureNode, attr[i].fieldName(), attr[i].fieldValue());
	}
	delete fet;
      }

#ifdef QGISDEBUG
      std::cout << "Feature count on identify: " << featureCount << std::endl;
#endif

      //also test the not commited features //todo: eliminate copy past code
      /*for(std::list<QgsFeature*>::iterator it=mAddedFeatures.begin();it!=mAddedFeatures.end();++it)
      {
	  if((*it)->intersects(r))
	  {
	      featureCount++;
	      if (featureCount == 1)
	      {
		  ir = new QgsIdentifyResults(mActions);
	      }

	      QListViewItem *featureNode = ir->addNode("foo");
	      featureNode->setText(0, fieldIndex);
	      std::vector < QgsFeatureAttribute > attr = (*it)->attributeMap();
	      for (int i = 0; i < attr.size(); i++)
	      {
#ifdef QGISDEBUG
		  std::cout << attr[i].fieldName() << " == " << fieldIndex << std::endl;
#endif
		  if (attr[i].fieldName().lower() == fieldIndex)
		  {
		      featureNode->setText(1, attr[i].fieldValue());
		  }
		  ir->addAttribute(featureNode, attr[i].fieldName(), attr[i].fieldValue());
	      } 
	  }
	  }*/

      if (ir)
      {
	  ir->setTitle(name());
	  if (featureCount == 1)
	      ir->showAllAttributes();
	  // restore the identify window position and show it
	  ir->restorePosition();
      }
      
      if (featureCount == 0)
      {
	  QMessageBox::information(0, tr("No features found"), tr("No features were found in the active layer at the point you clicked"));
      }
  } 
  else { // Edit attributes 
      // TODO: what to do if more features were selected? - nearest?
      if ( (fet = dataProvider->getNextFeature(true)) ) {
	  // Was already changed?
	  std::map<int,std::map<QString,QString> >::iterator it = mChangedAttributes.find(fet->featureId());

	  std::vector < QgsFeatureAttribute > old;
	  if ( it != mChangedAttributes.end() ) {
	      std::map<QString,QString> oldattr = (*it).second;
	      for( std::map<QString,QString>::iterator ait = oldattr.begin(); ait!=oldattr.end(); ++ait ) {
		  old.push_back ( QgsFeatureAttribute ( (*ait).first, (*ait).second ) );
	      }
	  } else {
	      old = fet->attributeMap();
	  }
	  QgsAttributeDialog ad( &old );

	  if ( ad.exec()==QDialog::Accepted ) {
	      std::map<QString,QString> attr;
	      for(int i=0;i<old.size();++i) {
		  attr.insert ( std::make_pair( old[i].fieldName(), ad.value(i) ) );
	      }
	      // Remove old if exists
	      it = mChangedAttributes.find(fet->featureId());

	      if ( it != mChangedAttributes.end() ) { // found
		  mChangedAttributes.erase ( it );
	      }
		  
	      mChangedAttributes.insert ( std::make_pair( fet->featureId(), attr ) );
              mModified=true;
	  }
      }
  }
  QApplication::restoreOverrideCursor();
}

void QgsVectorLayer::table()
{
  if (tabledisplay)
  {
    tabledisplay->raise();
    // Give the table the most recent copy of the actions for this
    // layer.
    tabledisplay->table()->setAttributeActions(mActions);
  }
  else
  {
    // display the attribute table
    QApplication::setOverrideCursor(Qt::waitCursor);
    tabledisplay = new QgsAttributeTableDisplay(this);
    connect(tabledisplay, SIGNAL(deleted()), this, SLOT(invalidateTableDisplay()));
    fillTable(tabledisplay->table());
    tabledisplay->table()->setSorting(true);


    tabledisplay->setTitle(tr("Attribute table - ") + name());
    tabledisplay->show();
    tabledisplay->table()->clearSelection();  //deselect the first row

    // Give the table the most recent copy of the actions for this
    // layer.
    tabledisplay->table()->setAttributeActions(mActions);

    QObject::disconnect(tabledisplay->table(), SIGNAL(selectionChanged()), tabledisplay->table(), SLOT(handleChangedSelections()));

    for (std::set<int>::iterator it = mSelected.begin(); it != mSelected.end(); ++it)
      {
        tabledisplay->table()->selectRowWithId(*it);
#ifdef QGISDEBUG
        qWarning("selecting row with id " + QString::number(*it));
#endif

      }

    QObject::connect(tabledisplay->table(), SIGNAL(selectionChanged()), tabledisplay->table(), SLOT(handleChangedSelections()));

    //etablish the necessary connections between the table and the shapefilelayer
    QObject::connect(tabledisplay->table(), SIGNAL(selected(int)), this, SLOT(select(int)));
    QObject::connect(tabledisplay->table(), SIGNAL(selectionRemoved()), this, SLOT(removeSelection()));
    QObject::connect(tabledisplay->table(), SIGNAL(repaintRequested()), this, SLOT(triggerRepaint()));
    QApplication::restoreOverrideCursor();
  }

} // QgsVectorLayer::table

void QgsVectorLayer::fillTable(QgsAttributeTable* t)
{
    if(t&&dataProvider)
    {
	int row = 0;
	int id;

	// set up the column headers
	QHeader *colHeader = t->horizontalHeader();
	
	dataProvider->reset();
	QgsFeature *fet;
	fet = dataProvider->getNextFeature(true);
	if(fet)//use the description of the feature to set up the column headers
	{
	    std::vector < QgsFeatureAttribute > attributes = fet->attributeMap();
	    int numFields = attributes.size();
	    t->setNumCols(numFields+1);
	    t->setNumRows(dataProvider->featureCount()+mAddedFeatures.size()-mDeleted.size());
	    colHeader->setLabel(0, "id");
	    for (int h = 1; h <= numFields; h++)
	    {
		colHeader->setLabel(h, attributes[h-1].fieldName());
	    }
	    delete fet;
	}
	else//no feature yet, use the description of the provider to set up the column headers
	{
	    std::vector < QgsField > fields = dataProvider->fields();
	    int numFields = fields.size();
	    t->setNumCols(numFields+1);
	    t->setNumRows(dataProvider->featureCount()+mAddedFeatures.size()-mDeleted.size());
	    colHeader->setLabel(0, "id");
	    for (int h = 1; h <= numFields; h++)
	    {
		colHeader->setLabel(h, fields[h - 1].name());
	    }
	}

	//go through the features and fill the values into the table
	dataProvider->reset();
	while ((fet = dataProvider->getNextFeature(true)))
	{
	    id=fet->featureId();
	    if(mDeleted.find(id)==mDeleted.end())
	    {
		//id-field
		t->setText(row, 0, QString::number(id));
		t->insertFeatureId(fet->featureId(), row);  //insert the id into the search tree of qgsattributetable
		std::vector < QgsFeatureAttribute > attr = fet->attributeMap();
		for (int i = 0; i < attr.size(); i++)
		{
		    // get the field values
		    t->setText(row, i + 1, attr[i].fieldValue());
#ifdef QGISDEBUG
		    //qWarning("Setting value for "+QString::number(i+1)+"//"+QString::number(row)+"//"+attr[i].fieldValue());
#endif
		}
		row++;
	    }
	    delete fet;
	}

	//also consider the not commited features
	for(std::list<QgsFeature*>::iterator it=mAddedFeatures.begin();it!=mAddedFeatures.end();++it)
	{
	    //id-field
	    tabledisplay->table()->setText(row, 0, QString::number((*it)->featureId()));
	    tabledisplay->table()->insertFeatureId((*it)->featureId(), row);  //insert the id into the search tree of qgsattributetable
	    std::vector < QgsFeatureAttribute > attr = (*it)->attributeMap();
	    for (int i = 0; i < attr.size(); i++)
	    {
		// get the field values
		tabledisplay->table()->setText(row, i + 1, attr[i].fieldValue());
	    }
	    row++;
	}
	
	dataProvider->reset();
	
    }
}

void QgsVectorLayer::select(int number)
{
  mSelected.insert(number);
}

void QgsVectorLayer::select(QgsRect * rect, bool lock)
{
  QApplication::setOverrideCursor(Qt::waitCursor);
  // normalize the rectangle
  rect->normalize();
  if (tabledisplay)
  {
    QObject::disconnect(tabledisplay->table(), SIGNAL(selectionChanged()), tabledisplay->table(), SLOT(handleChangedSelections()));
    QObject::disconnect(tabledisplay->table(), SIGNAL(selected(int)), this, SLOT(select(int))); //disconnecting because of performance reason
  }

  if (lock == false)
  {
    removeSelection();        //only if ctrl-button is not pressed
    if (tabledisplay)
    {
      tabledisplay->table()->clearSelection();
    }
  }

  dataProvider->select(rect, true);

  QgsFeature *fet;

  while (fet = dataProvider->getNextFeature(true))
  {
    if(mDeleted.find(fet->featureId())==mDeleted.end())//don't select deleted features
    {
      select(fet->featureId());
      if (tabledisplay)
      {
        tabledisplay->table()->selectRowWithId(fet->featureId());
      }
    }
  }

  //also test the not commited features
  for(std::list<QgsFeature*>::iterator it=mAddedFeatures.begin();it!=mAddedFeatures.end();++it)
  {
    if((*it)->intersects(rect))
    {
      select((*it)->featureId());
      if (tabledisplay)
      {
        tabledisplay->table()->selectRowWithId((*it)->featureId());
      }
    }
  }

  if (tabledisplay)
  {
    QObject::connect(tabledisplay->table(), SIGNAL(selectionChanged()), tabledisplay->table(), SLOT(handleChangedSelections()));
    QObject::connect(tabledisplay->table(), SIGNAL(selected(int)), this, SLOT(select(int)));  //disconnecting because of performance reason
  }
  triggerRepaint();
  QApplication::restoreOverrideCursor();
}

void QgsVectorLayer::invertSelection()
{
    QApplication::setOverrideCursor(Qt::waitCursor);
    if (tabledisplay)
    {
	QObject::disconnect(tabledisplay->table(), SIGNAL(selectionChanged()), tabledisplay->table(), SLOT(handleChangedSelections()));
	QObject::disconnect(tabledisplay->table(), SIGNAL(selected(int)), this, SLOT(select(int))); //disconnecting because of performance reason
    }

    //copy the ids of selected features to tmp
    std::list<int> tmp;
    for(std::set<int>::iterator iter=mSelected.begin();iter!=mSelected.end();++iter)
    {
	tmp.push_back(*iter);
    }

    removeSelection();
    if (tabledisplay)
    {
	tabledisplay->table()->clearSelection();
    }

    QgsFeature *fet;
    dataProvider->reset();

    while (fet = dataProvider->getNextFeature(true))
    {
	if(mDeleted.find(fet->featureId())==mDeleted.end())//don't select deleted features
	{
	    select(fet->featureId());
	}
    }
    
    for(std::list<QgsFeature*>::iterator iter=mAddedFeatures.begin();iter!=mAddedFeatures.end();++iter)
    {
	select((*iter)->featureId());
    }

    for(std::list<int>::iterator iter=tmp.begin();iter!=tmp.end();++iter)
    {
	mSelected.erase(*iter);
    }
  
    if(tabledisplay)
    {
	for(std::set<int>::iterator iter=mSelected.begin();iter!=mSelected.end();++iter)
	{
	    tabledisplay->table()->selectRowWithId(*iter);
	}
    }

    if (tabledisplay)
    {
	QObject::connect(tabledisplay->table(), SIGNAL(selectionChanged()), tabledisplay->table(), SLOT(handleChangedSelections()));
	QObject::connect(tabledisplay->table(), SIGNAL(selected(int)), this, SLOT(select(int)));  //disconnecting because of performance reason
    }
    
    triggerRepaint();
    QApplication::restoreOverrideCursor();
}

void QgsVectorLayer::removeSelection()
{
  mSelected.clear();
}

void QgsVectorLayer::triggerRepaint()
{
#ifdef QGISDEBUG
    std::cerr << "QgsVectorLayer: triggerRepaint called" << std::endl;
#endif
  
  emit repaintRequested();

#ifdef QGISDEBUG
    std::cerr << "QgsVectorLayer: repaintRequested emitted" << std::endl;
#endif
}

void QgsVectorLayer::invalidateTableDisplay()
{
  tabledisplay = 0;
}

QgsVectorDataProvider * QgsVectorLayer::getDataProvider()
{
  return dataProvider;
}


void QgsVectorLayer::showLayerProperties()
{
  // Set wait cursor while the property dialog is created
  // and initialized
  qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
  if (! m_propertiesDialog)
  {
#ifdef QGISDEBUG
    std::cerr << "Creating new QgsDlgVectorLayerProperties object\n";
#endif
    m_propertiesDialog = new QgsDlgVectorLayerProperties(this);
    // Make sure that the UI starts out with the correct display
    // field value
#ifdef QGISDEBUG
    std::cerr << "Setting display field in prop dialog\n";
#endif
    m_propertiesDialog->setDisplayField(displayField());
  }
#ifdef QGISDEBUG
  std::cerr << "Resetting prop dialog\n";
#endif
  m_propertiesDialog->reset();
#ifdef QGISDEBUG
  std::cerr << "Raising prop dialog\n";
#endif
  m_propertiesDialog->raise();
#ifdef QGISDEBUG
  std::cerr << "Showing prop dialog\n";
#endif
  m_propertiesDialog->show();
  // restore normal cursor
  qApp->restoreOverrideCursor();
} // QgsVectorLayer::showLayerProperties()


QgsRenderer *QgsVectorLayer::renderer()
{
  return m_renderer;
}

QDialog *QgsVectorLayer::rendererDialog()
{
  return m_rendererDialog;
}

void QgsVectorLayer::setRenderer(QgsRenderer * r)
{
  if (r != m_renderer)
  {
    if (m_renderer)           //delete any previous renderer
    {
      delete m_renderer;
    }

    m_renderer = r;
  }
}

void QgsVectorLayer::setRendererDialog(QDialog * dialog)
{
  if (dialog != m_rendererDialog)
  {
    if (m_rendererDialog)
    {
      delete m_rendererDialog;
    }
    m_rendererDialog = dialog;
  }
}

QGis::VectorType QgsVectorLayer::vectorType()
{
  if (dataProvider)
  {
    int type = dataProvider->geometryType();
    switch (type)
    {
    case QGis::WKBPoint:
      return QGis::Point;
    case QGis::WKBLineString:
      return QGis::Line;
    case QGis::WKBPolygon:
      return QGis::Polygon;
    case QGis::WKBMultiPoint:
      return QGis::Point;
    case QGis::WKBMultiLineString:
      return QGis::Line;
    case QGis::WKBMultiPolygon:
      return QGis::Polygon;
    }
  }
  else
  {
#ifdef QGISDEBUG
    qWarning("warning, pointer to dataProvider is null in QgsVectorLayer::vectorType()");
#endif

  }
}

QgsDlgVectorLayerProperties *QgsVectorLayer::propertiesDialog()
{
  return m_propertiesDialog;
}


void QgsVectorLayer::initContextMenu_(QgisApp * app)
{
  myPopupLabel->setText( tr("<center><b>Vector Layer</b></center>") );

  popMenu->insertItem(tr("&Open attribute table"), app, SLOT(attributeTable()));

  popMenu->insertSeparator(); // XXX should this move to QgsMapLayer::initContextMenu()?

  int cap=dataProvider->capabilities();
  if((cap&QgsVectorDataProvider::AddFeatures)
     ||(cap&QgsVectorDataProvider::DeleteFeatures))
  {
    popMenu->insertItem(tr("Start editing"),this,SLOT(startEditing()));
    popMenu->insertItem(tr("Stop editing"),this,SLOT(stopEditing()));
  }

  // XXX Can we ask the provider if it wants to add things to the context menu?
  if(cap&QgsVectorDataProvider::SaveAsShapefile)
  {
    // add the save as shapefile menu item
    popMenu->insertSeparator();
    popMenu->insertItem(tr("Save as shapefile..."), this, SLOT(saveAsShapefile()));
  }

} // QgsVectorLayer::initContextMenu_(QgisApp * app)



// XXX why is this here?  This should be generalized up to QgsMapLayer
QPopupMenu *QgsVectorLayer::contextMenu()
{
  return popMenu;
}

QgsRect QgsVectorLayer::bBoxOfSelected()
{
  QgsRect rect(DBL_MAX, DBL_MAX, -DBL_MAX, -DBL_MAX);
  dataProvider->reset();

  QgsFeature *fet;
  unsigned char *feature;

  double *x;
  double *y;
  int *nPoints;
  int *numRings;
  int *numPolygons;
  int numPoints;
  int numLineStrings;
  int idx, jdx, kdx;
  unsigned char *ptr;
  char lsb;
  QgsPoint pt;
  QPointArray *pa;
  int wkbType;

  while ((fet = dataProvider->getNextFeature(false)))
  {
    if (mSelected.find(fet->featureId()) != mSelected.end())
    {
      feature = fet->getGeometry();
      wkbType = (int) feature[1];


      switch (wkbType)
      {
      case WKBPoint:
        x = (double *) (feature + 5);
        y = (double *) (feature + 5 + sizeof(double));
        if (*x < rect.xMin())
        {
          rect.setXmin(*x);
        }
        if (*x > rect.xMax())
        {
          rect.setXmax(*x);
        }
        if (*y < rect.yMin())
        {
          rect.setYmin(*y);
        }
        if (*y > rect.yMax())
        {
          rect.setYmax(*y);
        }
        break;

      case WKBLineString:
        // get number of points in the line
        ptr = feature + 5;
        nPoints = (int *) ptr;
        ptr = feature + 1 + 2 * sizeof(int);
        for (idx = 0; idx < *nPoints; idx++)
        {
          x = (double *) ptr;
          ptr += sizeof(double);
          y = (double *) ptr;
          ptr += sizeof(double);
          if (*x < rect.xMin())
          {
            rect.setXmin(*x);
          }
          if (*x > rect.xMax())
          {
            rect.setXmax(*x);
          }
          if (*y < rect.yMin())
          {
            rect.setYmin(*y);
          }
          if (*y > rect.yMax())
          {
            rect.setYmax(*y);
          }
        }
        break;

      case WKBMultiLineString:
        numLineStrings = (int) (feature[5]);
        ptr = feature + 9;
        for (jdx = 0; jdx < numLineStrings; jdx++)
        {
          // each of these is a wbklinestring so must handle as such
          lsb = *ptr;
          ptr += 5;   // skip type since we know its 2
          nPoints = (int *) ptr;
          ptr += sizeof(int);
          for (idx = 0; idx < *nPoints; idx++)
          {
            x = (double *) ptr;
            ptr += sizeof(double);
            y = (double *) ptr;
            ptr += sizeof(double);
            if (*x < rect.xMin())
            {
              rect.setXmin(*x);
            }
            if (*x > rect.xMax())
            {
              rect.setXmax(*x);
            }
            if (*y < rect.yMin())
            {
              rect.setYmin(*y);
            }
            if (*y > rect.yMax())
            {
              rect.setYmax(*y);
            }
          }
        }
        break;

      case WKBPolygon:
        // get number of rings in the polygon
        numRings = (int *) (feature + 1 + sizeof(int));
        ptr = feature + 1 + 2 * sizeof(int);
        for (idx = 0; idx < *numRings; idx++)
        {
          // get number of points in the ring
          nPoints = (int *) ptr;
          ptr += 4;
          for (jdx = 0; jdx < *nPoints; jdx++)
          {
            // add points to a point array for drawing the polygon
            x = (double *) ptr;
            ptr += sizeof(double);
            y = (double *) ptr;
            ptr += sizeof(double);
            if (*x < rect.xMin())
            {
              rect.setXmin(*x);
            }
            if (*x > rect.xMax())
            {
              rect.setXmax(*x);
            }
            if (*y < rect.yMin())
            {
              rect.setYmin(*y);
            }
            if (*y > rect.yMax())
            {
              rect.setYmax(*y);
            }
          }
        }
        break;

      case WKBMultiPolygon:
        // get the number of polygons
        ptr = feature + 5;
        numPolygons = (int *) ptr;
        for (kdx = 0; kdx < *numPolygons; kdx++)
        {
          //skip the endian and feature type info and
          // get number of rings in the polygon
          ptr = feature + 14;
          numRings = (int *) ptr;
          ptr += 4;
          for (idx = 0; idx < *numRings; idx++)
          {
            // get number of points in the ring
            nPoints = (int *) ptr;
            ptr += 4;
            for (jdx = 0; jdx < *nPoints; jdx++)
            {
              // add points to a point array for drawing the polygon
              x = (double *) ptr;
              ptr += sizeof(double);
              y = (double *) ptr;
              ptr += sizeof(double);
              if (*x < rect.xMin())
              {
                rect.setXmin(*x);
              }
              if (*x > rect.xMax())
              {
                rect.setXmax(*x);
              }
              if (*y < rect.yMin())
              {
                rect.setYmin(*y);
              }
              if (*y > rect.yMax())
              {
                rect.setYmax(*y);
              }
            }
          }
        }
        break;

      default:
#ifdef QGISDEBUG
        std::cout << "UNKNOWN WKBTYPE ENCOUNTERED\n";
#endif
        break;

      }
      delete[]feature;
    }
  }
  return rect;
}

void QgsVectorLayer::setLayerProperties(QgsDlgVectorLayerProperties * properties)
{
  if (m_propertiesDialog)
  {
    delete m_propertiesDialog;
  }

  m_propertiesDialog = properties;
  // Make sure that the UI gets the correct display
  // field value
  m_propertiesDialog->setDisplayField(displayField());
}



QgsFeature * QgsVectorLayer::getFirstFeature(bool fetchAttributes) const
{
  if ( ! dataProvider )
  {
    std::cerr << __FILE__ << ":" << __LINE__
    << " QgsVectorLayer::getFirstFeature() invoked with null dataProvider\n";
    return 0x0;
  }

  return dataProvider->getFirstFeature( fetchAttributes );
} // QgsVectorLayer::getFirstFeature


QgsFeature * QgsVectorLayer::getNextFeature(bool fetchAttributes) const
{
  if ( ! dataProvider )
  {
    std::cerr << __FILE__ << ":" << __LINE__
    << " QgsVectorLayer::getNextFeature() invoked with null dataProvider\n";
    return 0x0;
  }

  return dataProvider->getNextFeature( fetchAttributes );
} // QgsVectorLayer::getNextFeature



bool QgsVectorLayer::getNextFeature(QgsFeature &feature, bool fetchAttributes) const
{
  if ( ! dataProvider )
  {
    std::cerr << __FILE__ << ":" << __LINE__
    << " QgsVectorLayer::getNextFeature() invoked with null dataProvider\n";
    return false;
  }

  return dataProvider->getNextFeature( feature, fetchAttributes );
} // QgsVectorLayer::getNextFeature



long QgsVectorLayer::featureCount() const
{
  if ( ! dataProvider )
  {
    std::cerr << __FILE__ << ":" << __LINE__
    << " QgsVectorLayer::featureCount() invoked with null dataProvider\n";
    return 0;
  }

  return dataProvider->featureCount();
} // QgsVectorLayer::featureCount

long QgsVectorLayer::updateFeatureCount() const
{
  if ( ! dataProvider )
  {
    std::cerr << __FILE__ << ":" << __LINE__
    << " QgsVectorLayer::updateFeatureCount() invoked with null dataProvider\n";
    return 0;
  }
  return dataProvider->updateFeatureCount();
}

void QgsVectorLayer::updateExtents()
{
  if ( ! dataProvider )
  {
    std::cerr << __FILE__ << ":" << __LINE__
    << " QgsVectorLayer::updateFeatureCount() invoked with null dataProvider\n";
  }
  else
  {
#ifdef QGISDEBUG
    qDebug("QgsVectorLayer: Getting current extents from the provider");
    qDebug(dataProvider->extent()->stringRep());
#endif
    // get the extent of the layer from the provider
    layerExtent.setXmin(dataProvider->extent()->xMin());
    layerExtent.setYmin(dataProvider->extent()->yMin());
    layerExtent.setXmax(dataProvider->extent()->xMax());
    layerExtent.setYmax(dataProvider->extent()->yMax());
  }
  
  // Send this (hopefully) up the chain to the map canvas
  emit recalculateExtents();
}

QString QgsVectorLayer::subsetString()
{
  if ( ! dataProvider )
  {
    std::cerr << __FILE__ << ":" << __LINE__
    << " QgsVectorLayer::subsetString() invoked with null dataProvider\n";
    return 0;
  }
  return dataProvider->subsetString();
}
void QgsVectorLayer::setSubsetString(QString subset)
{
  if ( ! dataProvider )
  {
    std::cerr << __FILE__ << ":" << __LINE__
    << " QgsVectorLayer::setSubsetString() invoked with null dataProvider\n";
  }
  else
  {
    dataProvider->setSubsetString(subset);
    updateExtents();
  }
  //trigger a recalculate extents request to any attached canvases
#ifdef QGISDEBUG
  std::cout << "Subset query changed, emitting recalculateExtents() signal" << std::endl;
#endif
  // emit the signal  to inform any listeners that the extent of this
  // layer has changed
  emit recalculateExtents();
}
int QgsVectorLayer::fieldCount() const
{
  if ( ! dataProvider )
  {
    std::cerr << __FILE__ << ":" << __LINE__
    << " QgsVectorLayer::fieldCount() invoked with null dataProvider\n";
    return 0;
  }

  return dataProvider->fieldCount();
} // QgsVectorLayer::fieldCount


std::vector<QgsField> const& QgsVectorLayer::fields() const
{
  if ( ! dataProvider )
  {
    std::cerr << __FILE__ << ":" << __LINE__
    << " QgsVectorLayer::fields() invoked with null dataProvider\n";

    static std::vector<QgsField> bogus; // empty, bogus container
    return bogus;
  }

  return dataProvider->fields();
} // QgsVectorLayer::fields()


bool QgsVectorLayer::addFeature(QgsFeature* f)
{
  if(dataProvider)
  {
    //set the endian properly
    int end=endian();
    memcpy(f->getGeometry(),&end,1);

    //assign a temporary id to the feature
    int tempid;
    if(mAddedFeatures.size()==0)
    {
      tempid=findFreeId();
    }
    else
    {
      std::list<QgsFeature*>::const_reverse_iterator rit=mAddedFeatures.rbegin();
      tempid=(*rit)->featureId()+1;
    }
#ifdef QGISDEBUG
    qWarning("assigned feature id "+QString::number(tempid));
#endif
    f->setFeatureId(tempid);
    mAddedFeatures.push_back(f);
    mModified=true;

    //hide and delete the table because it is not up to date any more
    if (tabledisplay)
    {
      tabledisplay->close();
      delete tabledisplay;
      tabledisplay=0;
    }

    return true;
  }
  return false;
}


QString QgsVectorLayer::getDefaultValue(const QString& attr,
                                        QgsFeature* f)
{
  return dataProvider->getDefaultValue(attr, f);
}

bool QgsVectorLayer::deleteSelectedFeatures()
{
    if(!(dataProvider->capabilities()&QgsVectorDataProvider::DeleteFeatures))
    {
	QMessageBox::information(0, tr("Provider does not support deletion"), tr("Data provider does not support deleting features"));
	return false;
    }

    if(!isEditable())
    {
	QMessageBox::information(0, tr("Layer not editable"), tr("The current layer is not editable. Choose 'start editing' in the legend item right click menu"));
	return false;
    }

    for(std::set<int>::iterator it=mSelected.begin();it!=mSelected.end();++it)
    {
      bool notcommitedfeature=false;
      //first test, if the feature with this id is a not-commited feature
      for(std::list<QgsFeature*>::iterator iter=mAddedFeatures.begin();iter!=mAddedFeatures.end();++iter)
      {
        if((*it)==(*iter)->featureId())
        {
          mAddedFeatures.erase(iter);
          notcommitedfeature=true;
          break;
        }
      }
      if(notcommitedfeature)
      {
        break;
      }
      mDeleted.insert(*it);
    }

    if(mSelected.size()>0)
    {
	mModified=true;
	mSelected.clear();
	triggerRepaint();

	//hide and delete the table because it is not up to date any more
	if (tabledisplay)
	{
	    tabledisplay->close();
	    delete tabledisplay;
	    tabledisplay=0;
	}

    }

  return true;
}

QgsLabel * QgsVectorLayer::label()
{
  return mLabel;
}

void QgsVectorLayer::setLabelOn ( bool on )
{
  mLabelOn = on;
}

bool QgsVectorLayer::labelOn ( void )
{
  return mLabelOn;
}

void QgsVectorLayer::startEditing()
{
  if(dataProvider)
  {
      if(!(dataProvider->capabilities()&QgsVectorDataProvider::AddFeatures))
      {
	  QMessageBox::information(0,"Start editing failed","Provider cannot be opened for editing",QMessageBox::Ok);
      }
      else
      {
	  mEditable=true;
	  if(isValid())
	  {
	      updateItemPixmap();
	  }
      }
  }
}

void QgsVectorLayer::stopEditing()
{
  if(dataProvider)
  {
    if(mModified)
    {
      //commit or roll back?
      int commit=QMessageBox::information(0,"Stop editing","Do you want to save the changes?",QMessageBox::Yes,QMessageBox::No);
      if(commit==QMessageBox::Yes)
      {
        if(!commitChanges())
        {
          QMessageBox::information(0,"Error","Could not commit changes",QMessageBox::Ok);
        }
        else
        {
          //hide and delete the table because it is not up to date any more
          if (tabledisplay)
          {
            tabledisplay->close();
            delete tabledisplay;
            tabledisplay=0;
          }
        }
      }
      else if(commit==QMessageBox::No)
      {
        if(!rollBack())
        {
          QMessageBox::information(0,"Error","Problems during roll back",QMessageBox::Ok);
        }
        //hide and delete the table because it is not up to date any more
        if (tabledisplay)
        {
          tabledisplay->close();
          delete tabledisplay;
          tabledisplay=0;
        }
      }
      triggerRepaint();
    }
    mEditable=false;
    mModified=false;
    if(isValid())
    {
      updateItemPixmap();
    }
  }
}

// return state of scale dependent rendering. True if features should
// only be rendered if between mMinimumScale and mMaximumScale
bool QgsVectorLayer::scaleDependentRender()
{
  return mScaleDependentRender;
}


// Return the minimum scale at which the layer is rendered
int QgsVectorLayer::minimumScale()
{
  return mMinimumScale;
}
// Return the maximum scale at which the layer is rendered
int QgsVectorLayer::maximumScale()
{
  return mMaximumScale;
}



bool QgsVectorLayer::readXML_( QDomNode & layer_node )
{
#ifdef QGISDEBUG
  std::cerr << "Datasource in QgsVectorLayer::readXML_: " << dataSource << std::endl;
#endif
  // process the attribute actions
  mActions.readXML(layer_node);

  //process provider key
  QDomNode pkeyNode = layer_node.namedItem("provider");

  if (pkeyNode.isNull())
  {
    providerKey = "";
  }
  else
  {
    QDomElement pkeyElt = pkeyNode.toElement();
    providerKey = pkeyElt.text();
  }

  // determine type of vector layer
  if ( ! providerKey.isNull() )
  {
    // if the provider string isn't empty, then we successfully
    // got the stored provider
  }
  else if ((dataSource.find("host=") > -1) &&
           (dataSource.find("dbname=") > -1))
  {
    providerKey = "postgres";
  }
  else
  {
    providerKey = "ogr";
  }

  setDataProvider( providerKey );

  //read provider encoding
  QDomNode encodingNode = layer_node.namedItem("encoding");
  if(!encodingNode.isNull()&&dataProvider)
  {
      dataProvider->setEncoding(encodingNode.toElement().text());
  }

  // get and set the display field if it exists.
  QDomNode displayFieldNode = layer_node.namedItem("displayfield");
  if (!displayFieldNode.isNull())
  {
    QDomElement e = displayFieldNode.toElement();
    setDisplayField(e.text());
  }

  // create and bind a renderer to this layer

  QDomNode singlenode = layer_node.namedItem("singlesymbol");
  QDomNode graduatednode = layer_node.namedItem("graduatedsymbol");
  QDomNode continuousnode = layer_node.namedItem("continuoussymbol");
  QDomNode singlemarkernode = layer_node.namedItem("singlemarker");
  QDomNode graduatedmarkernode = layer_node.namedItem("graduatedmarker");
  QDomNode uniquevaluenode = layer_node.namedItem("uniquevalue");
  QDomNode labelnode = layer_node.namedItem("label");
  QDomNode uniquemarkernode = layer_node.namedItem("uniquevaluemarker");

  //std::auto_ptr<QgsRenderer> renderer; actually the renderer SHOULD NOT be
  //deleted when this function finishes, otherwise the application will
  //crash
  // XXX this seems to be a dangerous implementation; should re-visit design
  QgsRenderer * renderer;

  if (!singlenode.isNull())
  {
    // renderer.reset( new QgsSingleSymRenderer );
    renderer = new QgsSingleSymRenderer;
    renderer->readXML(singlenode, *this);
  }
  else if (!graduatednode.isNull())
  {
    //renderer.reset( new QgsGraduatedSymRenderer );
    renderer = new QgsGraduatedSymRenderer;
    renderer->readXML(graduatednode, *this);
  }
  else if (!continuousnode.isNull())
  {
    //renderer.reset( new QgsContinuousColRenderer );
    renderer = new QgsContinuousColRenderer;
    renderer->readXML(continuousnode, *this);
  }
  else if (!singlemarkernode.isNull())
  {
    //renderer.reset( new QgsSiMaRenderer );
    renderer = new QgsSiMaRenderer;
    renderer->readXML(singlemarkernode, *this);
  }
  else if (!graduatedmarkernode.isNull())
  {
    //renderer.reset( new QgsGraduatedMaRenderer );
    renderer = new QgsGraduatedMaRenderer;
    renderer->readXML(graduatedmarkernode, *this);
  }
  else if (!uniquevaluenode.isNull())
  {
    //renderer.reset( new QgsUniqueValRenderer );
    renderer = new QgsUniqueValRenderer;
    renderer->readXML(uniquevaluenode, *this);
  }
  else if(!uniquemarkernode.isNull())
  {
    renderer = new QgsUValMaRenderer;
    renderer->readXML(uniquemarkernode, *this);
  }

  // Test if labeling is on or off
  QDomElement element = labelnode.toElement();
  int labelOn = element.text().toInt();
  if (labelOn < 1)
  {
    setLabelOn(false);
  }
  else
  {
    setLabelOn(true);
  }

#if QGISDEBUG
  std::cout << "Testing if qgsvectorlayer can call label readXML routine" << std::endl;
#endif

  QDomNode labelattributesnode = layer_node.namedItem("labelattributes");

  if(!labelattributesnode.isNull())
  {
#if QGISDEBUG
    std::cout << "qgsvectorlayer calling label readXML routine" << std::endl;
#endif
    mLabel->readXML(labelattributesnode);
  }

  return valid;               // should be true if read successfully

} // void QgsVectorLayer::readXML_



void
QgsVectorLayer:: setDataProvider( QString const & provider )
{
  // XXX should I check for and possibly delete any pre-existing providers?
  // XXX How often will that scenario occur?

  providerKey = provider;     // XXX is this necessary?  Usually already set
  // XXX when execution gets here.

  // load the plugin
  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();
  QString ogrlib = pReg->library(provider);

  //QString ogrlib = libDir + "/libpostgresprovider.so";

  const char *cOgrLib = (const char *) ogrlib;

#ifdef TESTPROVIDERLIB
  // test code to help debug provider loading problems
  //  void *handle = dlopen(cOgrLib, RTLD_LAZY);
  void *handle = dlopen(cOgrLib, RTLD_LAZY | RTLD_GLOBAL);
  if (!handle)
  {
    std::cout << "Error in dlopen: " << dlerror() << std::endl;

  }
  else
  {
    std::cout << "dlopen suceeded" << std::endl;
    dlclose(handle);
  }

#endif

  // load the data provider
  myLib = new QLibrary((const char *) ogrlib);
#ifdef QGISDEBUG
  std::cout << "Library name is " << myLib->library() << std::endl;
#endif
  bool loaded = myLib->load();

  if (loaded)
  {
#ifdef QGISDEBUG
    std::cout << "Loaded data provider library" << std::endl;
    std::cout << "Attempting to resolve the classFactory function" << std::endl;
#endif
    create_it * classFactory = (create_it *) myLib->resolve("classFactory");

    valid = false;            // assume the layer is invalid until we
    // determine otherwise
    if (classFactory)
    {
#ifdef QGISDEBUG
      std::cout << "Getting pointer to a dataProvider object from the library\n";
#endif
      //XXX - This was a dynamic cast but that kills the Windows
      //      version big-time with an abnormal termination error
      dataProvider = (QgsVectorDataProvider*)(classFactory((const
                                              char*)(dataSource.utf8())));

      if (dataProvider)
      {
#ifdef QGISDEBUG
        std::cout << "Instantiated the data provider plugin\n";
#endif
        if (dataProvider->isValid())
        {
          valid = true;
          
          // TODO: Check if the provider has the capability to send fullExtentCalculated
          connect(dataProvider, SIGNAL( fullExtentCalculated() ), 
                  this,           SLOT( updateExtents() ) 
                 );

          // Connect the repaintRequested chain from the data provider to this map layer
          // in the hope that the map canvas will notice       
          connect(dataProvider, SIGNAL( repaintRequested() ), 
                  this,           SLOT( triggerRepaint() ) 
                 );
          
          // get the extent
          QgsRect *mbr = dataProvider->extent();

          // show the extent
          QString s = mbr->stringRep();
#ifdef QGISDEBUG
          std::cout << "Extent of layer: " << s << std::endl;
#endif
          // store the extent
          layerExtent.setXmax(mbr->xMax());
          layerExtent.setXmin(mbr->xMin());
          layerExtent.setYmax(mbr->yMax());
          layerExtent.setYmin(mbr->yMin());

          // get and store the feature type
          geometryType = dataProvider->geometryType();

          // look at the fields in the layer and set the primary
          // display field using some real fuzzy logic
          setDisplayField();

          if (providerKey == "postgres")
          {
#ifdef QGISDEBUG
            std::cout << "Beautifying layer name " << layerName << std::endl;
#endif
            // adjust the display name for postgres layers
            layerName = layerName.mid(layerName.find(".") + 1);
            layerName = layerName.left(layerName.find("("));
#ifdef QGISDEBUG
            std::cout << "Beautified name is " << layerName << std::endl;
#endif

          }

          // upper case the first letter of the layer name
          layerName = layerName.left(1).upper() + layerName.mid(1);

          // label
          mLabel = new QgsLabel ( dataProvider->fields() );
          mLabelOn = false;

          //
          // Get the layers project info and set up the QgsCoordinateTransform for this layer
          //
          QString mySourceWKT = getProjectionWKT();
          //get the project projection, defaulting to this layer's projection
          //if none exists....
          QString myDestWKT = QgsProject::instance()->readEntry("SpatialRefSys","/WKT",mySourceWKT);
          //set up the coordinat transform - in the case of raster this is mainly used to convert
          //the inverese projection of the map extents of the canvas when zzooming in etc. so
          //that they match the coordinate system of this layer
          mCoordinateTransform = new QgsCoordinateTransform(mySourceWKT,myDestWKT);
        }
      }
      else
      {
#ifdef QGISDEBUG
        std::cout << "Unable to instantiate the data provider plugin\n";
#endif
        valid = false;
      }
    }
  }
  else
  {
    valid = false;
#ifdef QGISDEBUG
    std::cout << "Failed to load " << "../providers/libproviders.so" << "\n";
#endif

  }

} // QgsVectorLayer:: setDataProvider




/* virtual */ bool QgsVectorLayer::writeXML_( QDomNode & layer_node,
    QDomDocument & document )
{
  // first get the layer element so that we can append the type attribute

  QDomElement mapLayerNode = layer_node.toElement();

  if ( mapLayerNode.isNull() || ("maplayer" != mapLayerNode.nodeName()) )
  {
    const char * nn = mapLayerNode.nodeName(); // debugger probe

    qDebug( "QgsVectorLayer::writeXML() can't find <maplayer>" );

    return false;
  }

  mapLayerNode.setAttribute( "type", "vector" );

  // add provider node

  QDomElement provider  = document.createElement( "provider" );
  QDomText providerText = document.createTextNode( providerType() );
  provider.appendChild( providerText );
  layer_node.appendChild( provider );

  //provider encoding
  QDomElement encoding = document.createElement("encoding");
  QDomText encodingText = document.createTextNode(dataProvider->encoding());
  encoding.appendChild( encodingText );
  layer_node.appendChild( encoding );

  // add the display field

  QDomElement dField  = document.createElement( "displayfield" );
  QDomText dFieldText = document.createTextNode( displayField() );
  dField.appendChild( dFieldText );
  layer_node.appendChild( dField );

  // add label node

  QDomElement label  = document.createElement( "label" );
  QDomText labelText = document.createTextNode( "" );

  if ( labelOn() )
  {
    labelText.setData( "1" );
  }
  else
  {
    labelText.setData( "0" );
  }
  label.appendChild( labelText );

  layer_node.appendChild( label );

  // add attribute actions

  mActions.writeXML(layer_node, document);

  // renderer specific settings

  QgsRenderer * myRenderer;
  if( myRenderer = renderer())
  {
    myRenderer->writeXML(layer_node, document);
  }
  else
  {
    std::cerr << __FILE__ << ":" << __LINE__
    << " no renderer\n";

    // XXX return false?
  }

  // Now we get to do all that all over again for QgsLabel

  // XXX Since this is largely a cut-n-paste from the previous, this
  // XXX therefore becomes a candidate to be generalized into a separate
  // XXX function.  I think.

  QgsLabel * myLabel;

  if ( myLabel = this->label() )
  {
    std::stringstream labelXML;

    myLabel->writeXML(labelXML);

    QDomDocument labelDOM;

    std::string rawXML;
    std::string temp_str;
    QString     errorMsg;
    int         errorLine;
    int         errorColumn;

    // start with bogus XML header
    rawXML  = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";

    temp_str = labelXML.str();

    const char * temp_str_c = temp_str.c_str(); // debugger probe point

    rawXML   += temp_str;

#ifdef QGISDEBUG
    std::cout << rawXML << std::endl << std::flush;
#endif
    const char * s = rawXML.c_str(); // debugger probe

    if ( ! labelDOM.setContent( rawXML, &errorMsg, &errorLine, &errorColumn ) )
    {
      qDebug( "XML import error at line %d column %d " + errorMsg, errorLine, errorColumn );

      return false;
    }

    // lastChild() because the first two nodes are the <xml> and
    // <!DOCTYPE> nodes; the label node follows that, and is (hopefully)
    // the last node.
    QDomNode labelDOMNode = document.importNode( labelDOM.lastChild(), true );

    if ( ! labelDOMNode.isNull() )
    {
      layer_node.appendChild( labelDOMNode );
    }
    else
    {
      qDebug( "not able to import label DOM node" );

      // XXX return false?
    }

  }

  return true;
} // bool QgsVectorLayer::writeXML_


/** we wouldn't have to do this if slots were inherited */
void QgsVectorLayer::inOverview( bool b )
{
  QgsMapLayer::inOverview( b );
}

int QgsVectorLayer::findFreeId()
{
  int freeid=-INT_MAX;
  int fid;
  if(dataProvider)
  {
    dataProvider->reset();
    QgsFeature *fet;
    while ((fet = dataProvider->getNextFeature(true)))
    {
      fid=fet->featureId();
      if(fid>freeid)
      {
        freeid=fid;
      }
      delete fet;
    }
#ifdef QGISDEBUG
    qWarning("freeid is: "+QString::number(freeid+1));
#endif
    return freeid+1;
  }
  else
  {
#ifdef QGISDEBUG
    qWarning("Error, dataProvider is 0 in QgsVectorLayer::findFreeId");
#endif
    return -1;
  }
}

bool QgsVectorLayer::commitChanges()
{
  if(dataProvider)
  {
#ifdef QGISDEBUG
    qWarning("in QgsVectorLayer::commitChanges");
#endif
    bool returnvalue=true;
    std::list<QgsFeature*> addedlist;
    for(std::list<QgsFeature*>::iterator it=mAddedFeatures.begin();it!=mAddedFeatures.end();++it)
    {
      addedlist.push_back(*it);
    }
    if(!dataProvider->addFeatures(addedlist))
    {
      returnvalue=false;
    }
    for(std::list<QgsFeature*>::iterator it=mAddedFeatures.begin();it!=mAddedFeatures.end();++it)
    {
      delete *it;
    }
    mAddedFeatures.clear();

    if( mChangedAttributes.size() > 0 ) {
        if ( !dataProvider->changeAttributeValues ( mChangedAttributes ) ) {
          QMessageBox::warning(0,"Warning","Could change attributes");
	}
	mChangedAttributes.clear();
    }

    if(mDeleted.size()>0)
    {
      std::list<int> deletelist;
      for(std::set<int>::iterator it=mDeleted.begin();it!=mDeleted.end();++it)
        {
          deletelist.push_back(*it);
          mSelected.erase(*it);//just in case the feature is still selected
        }
      if(!dataProvider->deleteFeatures(deletelist))
      {
        returnvalue=false;
      }
    }
    return returnvalue;
  }
  else
  {
    return false;
  }
}

bool QgsVectorLayer::rollBack()
{
  for(std::list<QgsFeature*>::iterator it=mAddedFeatures.begin();it!=mAddedFeatures.end();++it)
  {
    delete *it;
    mSelected.erase((*it)->featureId());
  }
  mAddedFeatures.clear();
  for(std::set<int>::iterator it=mDeleted.begin();it!=mDeleted.end();++it)
    {
      mSelected.erase(*it);
    }
  mDeleted.clear();
  return true;
}

bool QgsVectorLayer::snapPoint(QgsPoint& point, double tolerance)
{
  if(tolerance<=0||!dataProvider)
  {
    return false;
  }
  double mindist=tolerance*tolerance;//current minimum distance
  double mindistx=point.x();
  double mindisty=point.y();
  QgsFeature* fet;
  QgsPoint vertexFeature;//the closest vertex of a feature
  double minvertexdist;//the distance between 'point' and 'vertexFeature'

  QgsRect selectrect(point.x()-tolerance,point.y()-tolerance,point.x()+tolerance,point.y()+tolerance);
  dataProvider->reset();
  dataProvider->select(&selectrect);
  while ((fet = dataProvider->getNextFeature(false)))
  {
    vertexFeature=fet->closestVertex(point);
    minvertexdist=vertexFeature.sqrDist(point.x(),point.y());
    if(minvertexdist<mindist)
    {
      mindistx=vertexFeature.x();
      mindisty=vertexFeature.y();
      mindist=minvertexdist;
    }
  }
  //also go through the not commited features
  for(std::list<QgsFeature*>::iterator iter=mAddedFeatures.begin();iter!=mAddedFeatures.end();++iter)
  {
      vertexFeature=(*iter)->closestVertex(point);
      minvertexdist=vertexFeature.sqrDist(point.x(),point.y());
      if(minvertexdist<mindist)
      {
	  mindistx=vertexFeature.x();
	  mindisty=vertexFeature.y();
	  mindist=minvertexdist;
      }
  }
  point.setX(mindistx);
  point.setY(mindisty);
}

void QgsVectorLayer::drawFeature(QPainter* p, QgsFeature* fet, QgsMapToPixel * theMapToPixelTransform, QPicture* marker, double markerScaleFactor, double widthScale, bool projectionsEnabledFlag)
{
  unsigned char *feature;
  bool attributesneeded = m_renderer->needsAttributes();

  double *x;
  double *y;
  int wkbType;
  QgsPoint pt,myProjectedPoint;
  QgsPoint ptFrom, ptTo;
  QgsPoint trimmedFrom, trimmedTo;

  QPen pen;

  // Scale pen width if necessary
  if ( widthScale != 1. ) {
      pen = p->pen(); 
      pen.setWidth ( (int) (widthScale * pen.width() ) ) ;
      //std::cout << "width = " << pen.width() << std::endl;
      p->setPen ( pen );
  }

  feature = fet->getGeometry();

  memcpy(&wkbType, (feature+1), sizeof(wkbType));

  switch (wkbType)
  {
  case WKBPoint:
    {
      x = (double *) (feature + 5);
      y = (double *) (feature + 5 + sizeof(double));
      //    std::cout << "transforming point\n";
      pt.setX(*x);
      pt.setY(*y);
      if (projectionsEnabledFlag)
      {
        //reproject the point to the map coordinate system
        try {
        myProjectedPoint=mCoordinateTransform->transform(pt);
      }
      catch (QgsCsException &e)
      {
        qDebug( "Transform error caught in %s line %d:\n%s", __FILE__, __LINE__, e.what());
      }        
        //transform from projected coordinate system to pixel position on map canvas
        theMapToPixelTransform->transform(&myProjectedPoint);
      }
      else
      {
        myProjectedPoint=theMapToPixelTransform->transform(pt);
      }
      //std::cout << "drawing marker for feature " << featureCount << "\n";
      //p->drawRect(static_cast<int>(myProjectedPoint.x()-markerScaleFactor*3), static_cast<int>(myProjectedPoint.y()-markerScaleFactor*3), static_cast<int>(markerScaleFactor*6), static_cast<int>(markerScaleFactor*6));
      //
      p->save();
      p->scale(markerScaleFactor,markerScaleFactor);
      p->drawPicture((int)(static_cast<int>(myProjectedPoint.x()) / markerScaleFactor - marker->boundingRect().x() - marker->boundingRect().width() / 2),
                     (int)(static_cast<int>(myProjectedPoint.y()) / markerScaleFactor - marker->boundingRect().y() - marker->boundingRect().height() / 2),
                     *marker);
      p->restore();

      // p->resetXForm(); // Don't use this, some transformations must be keept !

      break;
    }
  case WKBLineString:
    {
      unsigned char *ptr;
      int *nPoints;
      int idx;

      // get number of points in the line
      ptr = feature + 5;
      nPoints = (int *) ptr;
      ptr = feature + 1 + 2 * sizeof(int);
      for (idx = 0; idx < *nPoints; idx++)
      {
        x = (double *) ptr;
        ptr += sizeof(double);
        y = (double *) ptr;
        ptr += sizeof(double);
        // transform the point
        pt.setX(*x);
        pt.setY(*y);
        if (projectionsEnabledFlag)
        {
          //reproject the point to the map coordinate system
          try 
	  {
	    ptTo=mCoordinateTransform->transform(pt);
	  }
	  catch (QgsCsException &e)
	  {
	    qDebug( "Transform error caught in %s line %d:\n%s", 
		    __FILE__, __LINE__, e.what());
	  }
          //transform from projected coordinate system to pixel 
	  // position on map canvas
          theMapToPixelTransform->transform(&ptTo);
        }
        else
        {
          ptTo=theMapToPixelTransform->transform(pt);
        }
        if (idx == 0)
	  ptFrom = ptTo;
	else
	{
	  if (QgsMapToPixel::trimLine(ptFrom, ptTo, trimmedFrom, trimmedTo))
	    p->drawLine(static_cast<int>(trimmedFrom.x()),
			static_cast<int>(trimmedFrom.y()),
			static_cast<int>(trimmedTo.x()),
			static_cast<int>(trimmedTo.y()));
	  ptFrom = ptTo;
	}
      }
      break;
    }
  case WKBMultiLineString:
    {
      unsigned char *ptr;
      int idx, jdx, numLineStrings;
      int *nPoints;
      char lsb;

      numLineStrings = (int) (feature[5]);
      ptr = feature + 9;
      for (jdx = 0; jdx < numLineStrings; jdx++)
      {
        // each of these is a wbklinestring so must handle as such
        lsb = *ptr;
        ptr += 5; // skip type since we know its 2
        nPoints = (int *) ptr;
        ptr += sizeof(int);
        for (idx = 0; idx < *nPoints; idx++)
        {
          x = (double *) ptr;
          ptr += sizeof(double);
          y = (double *) ptr;
          ptr += sizeof(double);
          // transform the point
          pt.setX(*x);
          pt.setY(*y);
          if (projectionsEnabledFlag)
          {
            //reproject the point to the map coordinate system
	    try 
	    {
	      ptTo=mCoordinateTransform->transform(pt);
	    }
	    catch (QgsCsException &e)
	    {
	      qDebug( "Transform error caught in %s line %d:\n%s", 
		      __FILE__, __LINE__, e.what());
	    }
            //transform from projected coordinate system to pixel 
	    // position on map canvas
            theMapToPixelTransform->transform(&ptTo);
          }
          else
          {
            ptTo=theMapToPixelTransform->transform(pt);
          }
	  if (idx == 0)
	    ptFrom = ptTo;
	  else
	  {
	    if (QgsMapToPixel::trimLine(ptFrom, ptTo, trimmedFrom, trimmedTo))
	      p->drawLine(static_cast<int>(trimmedFrom.x()),
			  static_cast<int>(trimmedFrom.y()),
			  static_cast<int>(trimmedTo.x()),
			  static_cast<int>(trimmedTo.y()));
	    ptFrom = ptTo;
	  }
        }
      }
      break;
    }
  case WKBPolygon:

    {
      unsigned char *ptr;
      int idx, jdx;
      int *numRings, *nPoints;
      QPointArray *pa;

      // get number of rings in the polygon
      numRings = (int *) (feature + 1 + sizeof(int));

      if ( ! *numRings )  // sanity check for zero rings in polygon
      {
        break;
      }

      int *ringStart; // index of first point for each ring
      int *ringNumPoints; // number of points in each ring
      ringStart = new int[*numRings];
      ringNumPoints = new int[*numRings];

      int x0, y0, pdx;
      pdx = 0;
      ptr = feature + 1 + 2 * sizeof(int); // set pointer to the first ring
      for (idx = 0; idx < *numRings; idx++)
      {
        // get number of points in the ring
        nPoints = (int *) ptr;
        ringStart[idx] = pdx;
        ringNumPoints[idx] = *nPoints;
        ptr += 4;
        if ( idx == 0 )
        {
          pa = new QPointArray(*nPoints);
        }
        else
        {
          pa->resize ( pa->size() + *nPoints + 1 ); // better to calc size for all rings before?
        }
        for (jdx = 0; jdx < *nPoints; jdx++)
        {
          // add points to a point array for drawing the polygon
          x = (double *) ptr;
          ptr += sizeof(double);
          y = (double *) ptr;
#ifdef QGISX11DEBUG
          std::cout << "Transforming " << *x << "," << *y << " to ";
#endif
          ptr += sizeof(double);
          pt.setX(*x);
          pt.setY(*y);
          if (projectionsEnabledFlag)
          {
            //reproject the point to the map coordinate system
                      try {
          myProjectedPoint=mCoordinateTransform->transform(pt);
                }
      catch (QgsCsException &e)
      {
        qDebug( "Transform error caught in %s line %d:\n%s", __FILE__, __LINE__, e.what());
      }
            //transform from projected coordinate system to pixel position on map canvas
            theMapToPixelTransform->transform(&myProjectedPoint);
          }
          else
          {
            myProjectedPoint=theMapToPixelTransform->transform(pt);
          }
          pa->setPoint(pdx++, static_cast<int>(myProjectedPoint.x()), static_cast<int>(myProjectedPoint.y()));
        }
        if ( idx == 0 )
        { // remember last outer ring point
	    x0 = static_cast<int>(myProjectedPoint.x());
	    y0 = static_cast<int>(myProjectedPoint.y());
        }
        else
        { // return to x0,y0 (inner rings - islands)
          pa->setPoint(pdx++, x0, y0);
        }
      }
      // draw the polygon fill
      pen = p->pen(); // store current pen
      p->setPen ( Qt::NoPen ); // no boundary
      p->drawPolygon(*pa);

      // draw outline
      p->setPen ( pen );
      p->setBrush ( Qt::NoBrush );
      for (idx = 0; idx < *numRings; idx++)
      {
        p->drawPolygon( *pa, FALSE, ringStart[idx], ringNumPoints[idx]);
      }

      delete pa;
      delete [] ringStart;
      delete [] ringNumPoints;

      break;

    }
  case WKBMultiPolygon:
    {
      unsigned char *ptr;
      int idx, jdx, kdx;
      int *numPolygons, *numRings, *nPoints;
      QPointArray *pa;

      // get the number of polygons
      ptr = feature + 5;
      numPolygons = (int *) ptr;
      ptr = feature + 9;
      for (kdx = 0; kdx < *numPolygons; kdx++)
      {
        //skip the endian and feature type info and
        // get number of rings in the polygon
        ptr+=5;
        numRings = (int *) ptr;
        ptr += 4;
        for (idx = 0; idx < *numRings; idx++)
        {
          // get number of points in the ring
          nPoints = (int *) ptr;
          ptr += 4;
          pa = new QPointArray(*nPoints);
          for (jdx = 0; jdx < *nPoints; jdx++)
          {
            // add points to a point array for drawing the polygon
            x = (double *) ptr;
            ptr += sizeof(double);
            y = (double *) ptr;
            ptr += sizeof(double);
#ifdef QGISX11DEBUG
            std::cout << "Transforming " << *x << "," << *y << " to ";
#endif
            pt.setX(*x);
            pt.setY(*y);
            if (projectionsEnabledFlag)
            {
              //reproject the point to the map coordinate system
          try {
          myProjectedPoint=mCoordinateTransform->transform(pt);
                }
      catch (QgsCsException &e)
      {
        qDebug( "Transform error caught in %s line %d:\n%s", __FILE__, __LINE__, e.what());
      }
              //transform from projected coordinate system to pixel position on map canvas
              theMapToPixelTransform->transform(&myProjectedPoint);
            }
            else
            {
              myProjectedPoint=theMapToPixelTransform->transform(pt);
            }
            pa->setPoint(jdx, static_cast<int>(myProjectedPoint.x()), static_cast<int>(myProjectedPoint.y()));
          }
          // draw the ring
          p->drawPolygon(*pa);
          delete pa;
        }
      }
      break;
    }
  default:
#ifdef QGISDEBUG
    std::cout << "UNKNOWN WKBTYPE ENCOUNTERED\n";
#endif
    break;
  }
}

void QgsVectorLayer::saveAsShapefile()
{
  // call the dataproviders saveAsShapefile method
  dataProvider->saveAsShapefile();
  //  QMessageBox::information(0,"Save As Shapefile", "Someday...");
}

bool QgsVectorLayer::commitAttributeChanges(const std::set<QString>& deleted,
					    const std::map<QString,QString>& added,
					    std::map<int,std::map<QString,QString> >& changed)
{
    bool returnvalue=true;

    if(dataProvider)
    {
	if(dataProvider->capabilities()&QgsVectorDataProvider::DeleteAttributes)
	{
	    //delete attributes in all not commited features
	    for(std::list<QgsFeature*>::iterator iter=mAddedFeatures.begin();iter!=mAddedFeatures.end();++iter)
	    {
		for(std::set<QString>::const_iterator it=deleted.begin();it!=deleted.end();++it)
		{
		    (*iter)->deleteAttribute(*it);
		}
	    }
	    //and then in the provider
	    if(!dataProvider->deleteAttributes(deleted))
	    {
		returnvalue=false;
	    }
	}

	if(dataProvider->capabilities()&QgsVectorDataProvider::AddAttributes)
	{
	    //add attributes in all not commited features
	    for(std::list<QgsFeature*>::iterator iter=mAddedFeatures.begin();iter!=mAddedFeatures.end();++iter)
	    {
		for(std::map<QString,QString>::const_iterator it=added.begin();it!=added.end();++it)
		{
		    (*iter)->addAttribute(it->first);
		}
	    }
	    //and then in the provider
	    if(!dataProvider->addAttributes(added))
	    {
		returnvalue=false;
	    }
	}

	if(dataProvider->capabilities()&QgsVectorDataProvider::ChangeAttributeValues)
	{
	    //change values of the not commited features
	    for(std::list<QgsFeature*>::iterator iter=mAddedFeatures.begin();iter!=mAddedFeatures.end();++iter)
	    {
		std::map<int,std::map<QString,QString> >::iterator it=changed.find((*iter)->featureId());
		if(it!=changed.end())
		{
		    for(std::map<QString,QString>::const_iterator iterat=it->second.begin();iterat!=it->second.end();++iterat)
		    {
			(*iter)->changeAttributeValue(iterat->first,iterat->second);
		    }
		    changed.erase(it);
		}
	    }

	    //and then those of the commited ones
	    if(!dataProvider->changeAttributeValues(changed))
	    {
		returnvalue=false;
	    }
	}
    }
    else
    {
	returnvalue=false;
    }
    return returnvalue;
}
