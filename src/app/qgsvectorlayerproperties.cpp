/***************************************************************************
                          qgsdlgvectorlayerproperties.cpp
                   Unified property dialog for vector layers
                             -------------------
    begin                : 2004-01-28
    copyright            : (C) 2004 by Gary E.Sherman
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
 /* $Id$ */

#include "qgsvectorlayerproperties.h"

#include "qgsattributeactiondialog.h"
#include "qgscontexthelp.h"
#include "qgscontinuouscolordialog.h"
#include "qgscoordinatetransform.h"
#include "qgsgraduatedsymboldialog.h"
#include "qgslabel.h"
#include "qgslabeldialog.h"
#include "qgslayerprojectionselector.h"
#include "qgslogger.h"
#include "qgssinglesymboldialog.h"
#include "qgsuniquevaluedialog.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include "qgsconfig.h"
#ifdef HAVE_POSTGRESQL
#include "qgspgquerybuilder.h"
#include "../providers/postgres/qgspostgresprovider.h"
#endif

#include <QMessageBox>

QgsVectorLayerProperties::QgsVectorLayerProperties(QgsVectorLayer * lyr, 
                                                         QWidget * parent, 
                                                         Qt::WFlags fl)
: QDialog(parent, fl),
      layer(lyr), 
      mRendererDialog(0)
{
  setupUi(this);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));
  connect(this, SIGNAL(accepted()), this, SLOT(apply()));

  // Create the Label dialog tab
  QVBoxLayout *layout = new QVBoxLayout( labelOptionsFrame );
  layout->setMargin(0);
  labelDialog = new QgsLabelDialog ( layer->label(), labelOptionsFrame);
  layout->addWidget( labelDialog );
  labelOptionsFrame->setLayout(layout);
  connect(labelDialog, SIGNAL(labelSourceSet()), 
          this, SLOT(setLabelCheckBox()));

  // Create the Actions dialog tab
  QgsVectorDataProvider *dp = dynamic_cast<QgsVectorDataProvider *>(layer->getDataProvider());
  QVBoxLayout *actionLayout = new QVBoxLayout( actionOptionsFrame );
  actionLayout->setMargin(0);
  QgsFieldMap fields = dp->fields();
  actionDialog = new QgsAttributeActionDialog ( layer->actions(), fields, 
                                                actionOptionsFrame );
  actionLayout->addWidget( actionDialog );

  reset();
  if(layer->getDataProvider())//enable spatial index button group if supported by provider
  {
      int capabilities=layer->getDataProvider()->capabilities();
      if(!(capabilities&QgsVectorDataProvider::CreateSpatialIndex))
      {
	  indexGroupBox->setEnabled(false);
      }
  }

  leSpatialRefSys->setText(layer->srs().proj4String());
  leSpatialRefSys->setCursorPosition(0);
  
  connect(sliderTransparency, SIGNAL(valueChanged(int)), this, SLOT(sliderTransparency_valueChanged(int)));

} // QgsVectorLayerProperties ctor

QgsVectorLayerProperties::~QgsVectorLayerProperties()
{
  disconnect(labelDialog, SIGNAL(labelSourceSet()), 
          this, SLOT(setLabelCheckBox()));  
}
void QgsVectorLayerProperties::sliderTransparency_valueChanged(int theValue)
{
  //set the transparency percentage label to a suitable value
  int myInt = static_cast < int >((theValue / 255.0) * 100);  //255.0 to prevent integer division
  lblTransparencyPercent->setText(tr("Transparency: ") + QString::number(myInt) + "%");
}//sliderTransparency_valueChanged

void QgsVectorLayerProperties::setLabelCheckBox()
{
  labelCheckBox->setCheckState(Qt::Checked);
}

void QgsVectorLayerProperties::alterLayerDialog(const QString & dialogString)
{

    widgetStackRenderers->removeWidget(mRendererDialog);
    delete mRendererDialog;
    mRendererDialog=0;
    if(dialogString == tr("Single Symbol"))
    {
	mRendererDialog = new QgsSingleSymbolDialog(layer);
    }
    else if(dialogString == tr("Graduated Symbol"))
    {
	mRendererDialog = new QgsGraduatedSymbolDialog(layer);
    }
    else if(dialogString == tr("Continuous Color"))
    {
	mRendererDialog = new QgsContinuousColorDialog(layer);
    }
    else if(dialogString == tr("Unique Value"))
    {
	mRendererDialog = new QgsUniqueValueDialog(layer);
    }
    widgetStackRenderers->addWidget(mRendererDialog);
    widgetStackRenderers->setCurrentWidget(mRendererDialog);  
}

void QgsVectorLayerProperties::setLegendType(QString type)
{
  legendtypecombobox->setCurrentText(type);
}

void QgsVectorLayerProperties::setDisplayField(QString name)
{
  displayFieldComboBox->setCurrentText(name);
}

//! @note in raster props, this metho d is called sync()
void QgsVectorLayerProperties::reset( void )
{
  // populate the general information
  txtDisplayName->setText(layer->name());
  pbnQueryBuilder->setWhatsThis(tr("This button opens the PostgreSQL query builder and allows you to create a subset of features to display on the map canvas rather than displaying all features in the layer"));
  txtSubsetSQL->setWhatsThis(tr("The query used to limit the features in the layer is shown here. This is currently only supported for PostgreSQL layers. To enter or modify the query, click on the Query Builder button"));

  //we are dealing with a pg layer here so that we can enable the sql box
  QgsVectorDataProvider *dp = dynamic_cast<QgsVectorDataProvider *>(layer->getDataProvider());
  //see if we are dealing with a pg layer here
  if(layer->providerType() == "postgres")
  {
    grpSubset->setEnabled(true);
    txtSubsetSQL->setText(layer->subsetString());
    // if the user is allowed to type an adhoc query, the app will crash if the query
    // is bad. For this reason, the sql box is disabled and the query must be built
    // using the query builder, either by typing it in by hand or using the buttons, etc
    // on the builder. If the ability to enter a query directly into the box is required,
    // a mechanism to check it must be implemented.
    txtSubsetSQL->setEnabled(false); 
    pbnQueryBuilder->setEnabled(true);
  }
  else
  {
    grpSubset->setEnabled(false);
  }

  //get field list for display field combo
  const QgsFieldMap& myFields = dp->fields();
  for (QgsFieldMap::const_iterator it = myFields.begin(); it != myFields.end(); ++it)
  {
    displayFieldComboBox->insertItem( it->name() );
  }   

  // set up the scale based layer visibility stuff....
  chkUseScaleDependentRendering->setChecked(layer->scaleBasedVisibility());
  spinMinimumScale->setValue((int)layer->minScale());
  spinMaximumScale->setValue((int)layer->maxScale());

  // symbology initialization
  if(legendtypecombobox->count()==0)
    {
      legendtypecombobox->insertItem(tr("Single Symbol"));
      if(myFields.size()>0)
      {
	  legendtypecombobox->insertItem(tr("Graduated Symbol"));
	  legendtypecombobox->insertItem(tr("Continuous Color"));
	  legendtypecombobox->insertItem(tr("Unique Value"));
      }
    }

  //find out the type of renderer in the vectorlayer, create a dialog with these settings and add it to the form
  delete mRendererDialog;
  mRendererDialog=0;
  QString rtype=layer->renderer()->name();
  if(rtype=="Single Symbol")
  {
      mRendererDialog=new QgsSingleSymbolDialog(layer);
      legendtypecombobox->setCurrentIndex(0);
  }
  else if(rtype=="Graduated Symbol")
  {
      mRendererDialog=new QgsGraduatedSymbolDialog(layer);
      legendtypecombobox->setCurrentIndex(1);
  }
  else if(rtype=="Continuous Color")
  {
      mRendererDialog=new QgsContinuousColorDialog(layer);
      legendtypecombobox->setCurrentIndex(2);
  }
  else if(rtype == "Unique Value")
  {
      mRendererDialog=new QgsUniqueValueDialog(layer);
      legendtypecombobox->setCurrentIndex(3);
  }
  
  if(mRendererDialog)
  {
      widgetStackRenderers->addWidget(mRendererDialog);
      widgetStackRenderers->setCurrentWidget(mRendererDialog);
  }
  

  QObject::connect(legendtypecombobox, SIGNAL(activated(const QString &)), this, SLOT(alterLayerDialog(const QString &)));

  // reset fields in label dialog
  layer->label()->setFields ( layer->getDataProvider()->fields() );
  
  //set the metadata contents
  teMetadata->setText(getMetadata());
  actionDialog->init();
  labelDialog->init();
  labelCheckBox->setChecked(layer->labelOn());
  //set the transparency slider
  sliderTransparency->setValue(255 - layer->getTransparency());
  //update the transparency percentage label
  sliderTransparency_valueChanged(255 - layer->getTransparency());

} // reset()


//
// methods reimplemented from qt designer base class
//

void QgsVectorLayerProperties::on_buttonBox_helpRequested()
{
  QgsContextHelp::run(context_id);
}

void QgsVectorLayerProperties::apply()
{
  //
  // Set up sql subset query if applicable
  //
#ifdef HAVE_POSTGRESQL
  // Tim commented out the next line because dp is not actually used anywhere....
  //QgsVectorDataProvider *dp = dynamic_cast<QgsVectorDataProvider *>(layer->getDataProvider());
  //see if we are dealing with a pg layer here
  if(layer->providerType() == "postgres")
  {
    grpSubset->setEnabled(true);
    // set the subset sql for the layer
    layer->setSubsetString(txtSubsetSQL->text());   
    // update the metadata with the updated sql subset
    teMetadata->setText(getMetadata());
    // update the extents of the layer (fetched from the provider)
    layer->updateExtents(); 
  }
#endif
  // set up the scale based layer visibility stuff....
  layer->setScaleBasedVisibility(chkUseScaleDependentRendering->isChecked());
  layer->setMinScale(spinMinimumScale->value());
  layer->setMaxScale(spinMaximumScale->value());

  // update the display field
  layer->setDisplayField(displayFieldComboBox->currentText());

  actionDialog->apply();

  labelDialog->apply();
  layer->setLabelOn(labelCheckBox->isChecked());
  layer->setLayerName(displayName());


  QgsSingleSymbolDialog *sdialog = dynamic_cast < QgsSingleSymbolDialog * >(widgetStackRenderers->currentWidget());
  QgsGraduatedSymbolDialog *gdialog = dynamic_cast < QgsGraduatedSymbolDialog * >(widgetStackRenderers->currentWidget());
  QgsContinuousColorDialog *cdialog = dynamic_cast < QgsContinuousColorDialog * >(widgetStackRenderers->currentWidget());
  QgsUniqueValueDialog* udialog = dynamic_cast< QgsUniqueValueDialog * >(widgetStackRenderers->currentWidget()); 

  if (sdialog)
    {
      sdialog->apply();
    } 
  else if (gdialog)
  {
      gdialog->apply();
  }
  else if (cdialog)
    {
      cdialog->apply();
    }
  else if(udialog)
  {
      udialog->apply();
  }
  layer->setTransparency(static_cast < unsigned int >(255 - sliderTransparency->value()));
  
  // update symbology
  emit refreshLegend(layer->getLayerID(), false);
  
  layer->triggerRepaint();

}

void QgsVectorLayerProperties::on_pbnQueryBuilder_clicked()
{
#ifdef HAVE_POSTGRESQL
  // launch the query builder using the PostgreSQL connection
  // from the provider

  // get the data provider
  QgsVectorDataProvider *dp =
    dynamic_cast<QgsVectorDataProvider *>(layer->getDataProvider());
  // cast to postgres provider type
  QgsPostgresProvider * myPGProvider = (QgsPostgresProvider *) dp;
  // create the query builder object using the table name
  // and postgres connection from the provider
  QgsDataSourceURI uri(myPGProvider->dataSourceUri());
  QgsPgQueryBuilder *pqb = new QgsPgQueryBuilder(&uri, this);
       
  // Set the sql in the query builder to the same in the prop dialog
  // (in case the user has already changed it)
  pqb->setSql(txtSubsetSQL->text());
  // Open the query builder
  if(pqb->exec())
  {
    // if the sql is changed, update it in the prop subset text box
    txtSubsetSQL->setText(pqb->sql());
    //TODO If the sql is changed in the prop dialog, the layer extent should be recalculated

    // The datasource for the layer needs to be updated with the new sql since this gets
    // saved to the project file. This should happen at the map layer level...

  }
  // delete the query builder object
  delete pqb;
#endif
}

void QgsVectorLayerProperties::on_pbnIndex_clicked()
{
    QgsVectorDataProvider* pr=layer->getDataProvider();
    if(pr)
    {
	setCursor(Qt::WaitCursor);
	bool errval=pr->createSpatialIndex();
	setCursor(Qt::ArrowCursor);
	if(errval)
	{
	    QMessageBox::information(this, tr("Spatial Index"), tr("Creation of spatial index successfull"));
	}
	else
	{
           // TODO: Remind the user to use OGR >= 1.2.6 and Shapefile
	   QMessageBox::information(this, tr("Spatial Index"), tr("Creation of spatial index failed")); 
	}
    }
}

QString QgsVectorLayerProperties::getMetadata()
{
  QString myMetadataQString = "<html><body>";
  myMetadataQString += "<table width=\"100%\">";
  
  //-------------
  
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("General:");
  myMetadataQString += "</td></tr>";

  // data comment
  if (!(layer->dataComment().isEmpty()))
  {
    myMetadataQString += "<tr><td bgcolor=\"white\">";
    myMetadataQString += tr("Layer comment: ") + 
        layer->dataComment();
    myMetadataQString += "</td></tr>";
  }

  //storage type
  myMetadataQString += "<tr><td bgcolor=\"white\">";
  myMetadataQString += tr("Storage type of this layer : ") + 
                       layer->storageType();
  myMetadataQString += "</td></tr>";

  // data source
  myMetadataQString += "<tr><td bgcolor=\"white\">";
  myMetadataQString += tr("Source for this layer : ") +
                       layer->publicSource();
  myMetadataQString += "</td></tr>";

  //geom type

  QGis::VectorType vectorType = layer->vectorType();

  if ( vectorType < 0 || vectorType > QGis::Polygon )
  {
      QgsDebugMsg( "Invalid vector type" );
  }
  else
  {
      QString vectorTypeString( QGis::qgisVectorGeometryType[layer->vectorType()] );

      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += tr("Geometry type of the features in this layer : ") + 
          vectorTypeString;
      myMetadataQString += "</td></tr>";
  }


      //feature count
  myMetadataQString += "<tr><td bgcolor=\"white\">";
  myMetadataQString += tr("The number of features in this layer : ") + 
                       QString::number(layer->featureCount());
  myMetadataQString += "</td></tr>";
  //capabilities
  myMetadataQString += "<tr><td bgcolor=\"white\">";
  myMetadataQString += tr("Editing capabilities of this layer : ") + 
                       layer->capabilitiesString();
  myMetadataQString += "</td></tr>";

  //-------------

  QgsRect myExtent = layer->extent();  
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Extents:");
  myMetadataQString += "</td></tr>";
  //extents in layer cs  TODO...maybe make a little nested table to improve layout...
  myMetadataQString += "<tr><td bgcolor=\"white\">";
  myMetadataQString += tr("In layer spatial reference system units : ") + 
                       tr("xMin,yMin ") + 
                       QString::number(myExtent.xMin()) + 
                       "," + 
                       QString::number( myExtent.yMin()) +
                       tr(" : xMax,yMax ") + 
                       QString::number(myExtent.xMax()) + 
                       "," + 
                       QString::number(myExtent.yMax());
  myMetadataQString += "</td></tr>";

  //extents in project cs

  try
  {
    /*    
    // TODO: currently disabled, will revisit later [MD]
    QgsRect myProjectedExtent = coordinateTransform->transformBoundingBox(layer->extent());
    myMetadataQString += "<tr><td bgcolor=\"white\">";
    myMetadataQString += tr("In project spatial reference system units : ") + 
                         tr("xMin,yMin ") + 
                         QString::number(myProjectedExtent.xMin()) + 
                         "," + 
                         QString::number( myProjectedExtent.yMin()) +
                         tr(" : xMax,yMax ") + 
                         QString::number(myProjectedExtent.xMax()) + 
                         "," + 
                         QString::number(myProjectedExtent.yMax());
    myMetadataQString += "</td></tr>";
    */

    // 
    // Display layer spatial ref system
    //
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Layer Spatial Reference System:");
    myMetadataQString += "</td></tr>";  
    myMetadataQString += "<tr><td bgcolor=\"white\">";
    myMetadataQString += layer->srs().proj4String().replace(QRegExp("\"")," \"");                       
    myMetadataQString += "</td></tr>";

    // 
    // Display project (output) spatial ref system
    //  
    /*
    // TODO: disabled for now, will revisit later [MD]
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Project (Output) Spatial Reference System:");
    myMetadataQString += "</td></tr>";  
    myMetadataQString += "<tr><td bgcolor=\"white\">";
    myMetadataQString += coordinateTransform->destSRS().proj4String().replace(QRegExp("\"")," \"");                       
    myMetadataQString += "</td></tr>";
    */

  }
  catch(QgsCsException &cse)
  {
    UNUSED(cse);
    QgsDebugMsg( cse.what() );

    myMetadataQString += "<tr><td bgcolor=\"white\">";
    myMetadataQString += tr("In project spatial reference system units : ");
    myMetadataQString += " (Invalid transformation of layer extents) ";
    myMetadataQString += "</td></tr>";

  }

      
  //
  // Add the info about each field in the attribute table
  //
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Attribute field info:");
  myMetadataQString += "</td></tr>";
  myMetadataQString += "<tr><td bgcolor=\"white\">";

  // Start a nested table in this trow
  myMetadataQString += "<table width=\"100%\">";
  myMetadataQString += "<tr><th bgcolor=\"black\">";
  myMetadataQString += "<font color=\"white\">" + tr("Field") + "</font>";
  myMetadataQString += "</th>";
  myMetadataQString += "<th bgcolor=\"black\">";
  myMetadataQString += "<font color=\"white\">" + tr("Type") + "</font>";
  myMetadataQString += "</th>";
  myMetadataQString += "<th bgcolor=\"black\">";
  myMetadataQString += "<font color=\"white\">" + tr("Length") + "</font>";
  myMetadataQString += "</th>";
  myMetadataQString += "<th bgcolor=\"black\">";
  myMetadataQString += "<font color=\"white\">" + tr("Precision") + "</font>";
  myMetadataQString += "</th>";      
  myMetadataQString += "<th bgcolor=\"black\">";
  myMetadataQString += "<font color=\"white\">" + tr("Comment") + "</font>";
  myMetadataQString += "</th>";
 
  //get info for each field by looping through them
  QgsVectorDataProvider *myDataProvider = dynamic_cast<QgsVectorDataProvider *>(layer->getDataProvider());
  const QgsFieldMap& myFields = myDataProvider->fields();
  for (QgsFieldMap::const_iterator it = myFields.begin(); it != myFields.end(); ++it)
  {
    const QgsField& myField = *it;
    
    myMetadataQString += "<tr><td bgcolor=\"white\">";
    myMetadataQString += myField.name();
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"white\">";
    myMetadataQString += myField.typeName();
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"white\">";
    myMetadataQString += QString("%1").arg(myField.length());
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"white\">";
    myMetadataQString += QString("%1").arg(myField.precision());
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"white\">";
    myMetadataQString += QString("%1").arg(myField.comment());
    myMetadataQString += "</td></tr>";
  } 

  //close field list
  myMetadataQString += "</table>"; //end of nested table
  myMetadataQString += "</td></tr>"; //end of stats container table row
  //
  // Close the table
  //

  myMetadataQString += "</table>";
  myMetadataQString += "</body></html>";
  return myMetadataQString;

}



void QgsVectorLayerProperties::on_pbnChangeSpatialRefSys_clicked()
{
    QgsLayerProjectionSelector * mySelector = new QgsLayerProjectionSelector(this);
    mySelector->setSelectedSRSID(layer->srs().srsid());
    if(mySelector->exec())
    {
      QgsSpatialRefSys srs(mySelector->getCurrentSRSID(), QgsSpatialRefSys::QGIS_SRSID);
      layer->setSrs(srs);
    }
    else
    {
      QApplication::restoreOverrideCursor();
    }
    delete mySelector;

    leSpatialRefSys->setText(layer->srs().proj4String());
    leSpatialRefSys->setCursorPosition(0);
}
