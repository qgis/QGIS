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
#include <qstring.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qtextstream.h>
#include <qtable.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qspinbox.h>
#include <qwidgetstack.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <qgroupbox.h>
#include <qwhatsthis.h>


#include "qgis.h"
#include "qgsrect.h"
#include "qgsfield.h"
#include "qgsdlgvectorlayerproperties.h"
#include "qgsvectordataprovider.h"
#ifdef HAVE_POSTGRESQL
#include "../providers/postgres/qgspostgresprovider.h"
#endif
#include "qgsvectorlayer.h"
#include "qgssinglesymrenderer.h"
#include "qgsgraduatedmarenderer.h"
#include "qgsgraduatedsymrenderer.h"
#include "qgscontinuouscolrenderer.h"
#include "qgsuniquevalrenderer.h"
#include "qgsuvalmarenderer.h"
#include "qgssimarenderer.h"
#include "qgssimadialog.h"
#include "qgslegenditem.h"
#include "qgssisydialog.h"
#include "qgsgramadialog.h"
#include "qgsgrasydialog.h"
#include "qgscontcoldialog.h"
#include "qgsuvaldialog.h"
#include "qgsuvalmadialog.h"
#include "qobjectlist.h"
#include "qgsgramadialog.h"
#include "qgslabelattributes.h"
#include "qgslabel.h"
#include "qgslabeldialog.h"
#include "qgsattributeactiondialog.h"
#ifdef HAVE_POSTGRESQL
#include "qgspgquerybuilder.h"
#endif

QgsDlgVectorLayerProperties::QgsDlgVectorLayerProperties(QgsVectorLayer * lyr, QWidget * parent, const char *name, bool modal):QgsDlgVectorLayerPropertiesBase(parent, name, modal), layer(lyr), rendererDirty(false), bufferDialog(layer->rendererDialog()),
bufferRenderer(layer->
               renderer())
{

  // populate the general information
  QString source = layer->source();
  source = source.left(source.find("password"));
  lblSource->setText(source);
  txtDisplayName->setText(layer->name());
  // set whats this stuff
  QWhatsThis::add(lblSource, tr("The source of the data (path name or database connection information)"));
  QWhatsThis::add(pbnQueryBuilder, tr("This button opens the PostgreSQL query builder and allows you to create a subset of features to display on the map canvas rather than displaying all features in the layer"));
  QWhatsThis::add(txtSubsetSQL, tr("The query used to limit the features in the layer is shown here. This is currently only supported for PostgreSQL layers. To enter or modify the query, click on the Query Builder button"));
  QWhatsThis::add(lblGeometryType, tr("Geometry type of the features in this layer"));
  QWhatsThis::add(lblFeatureCount, tr("The number of features in this layer"));
  // display type and feature count
  lblGeometryType->setText(QGis::qgisVectorGeometryType[layer->vectorType()]);
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
  QString numFeatures;
  numFeatures = numFeatures.setNum(layer->featureCount());
  lblFeatureCount->setText(numFeatures);
  QgsRect *extent = dp->extent();
  QString ll;
  //QTextOStream (&ll) << extent->xMin() << ", " << extent->yMin();
  lblLowerLeft->setText(ll.sprintf("%16f, %16f", extent->xMin(), extent->yMin()));
  QString ur;
//  QTextOStream (&ur) << extent->xMax() << ", " << extent->yMax();
  lblUpperRight->setText(ur.sprintf("%16f, %16f", extent->xMax(), extent->yMax()));
  std::vector<QgsField> fields = dp->fields();
  // populate the table and the display field drop-down with the field
  // information

  for (int i = 0; i < fields.size(); i++)
  {
    QgsField fld = fields[i];
    QListViewItem *lvi = new QListViewItem(listViewFields, fld.name(),
                                           fld.type(), QString("%1").arg(fld.length()),
                                           QString("%1").arg(fld.precision()));
    displayFieldComboBox->insertItem( fld.name() );
  }

  // set up the scale based layer visibility stuff....
  chkUseScaleDependentRendering->setChecked(lyr->scaleBasedVisibility());
  spinMinimumScale->setValue(lyr->minScale());
  spinMaximumScale->setValue(lyr->maxScale());

  // symbology initialization
  legendtypecombobox->insertItem(tr("Single Symbol"));
  legendtypecombobox->insertItem(tr("Graduated Symbol"));
  legendtypecombobox->insertItem(tr("Continuous Color"));
  legendtypecombobox->insertItem(tr("Unique Value"));
  if( layer->vectorType()==QGis::Point )
  {
      legendtypecombobox->insertItem(tr("Single Marker"));
      legendtypecombobox->insertItem(tr("Graduated Marker"));
      legendtypecombobox->insertItem(tr("Unique Value Marker"));
  }

  QVBoxLayout *layout = new QVBoxLayout( labelOptionsFrame );
  labelDialog = new QgsLabelDialog ( layer->label(),labelOptionsFrame);
  layout->addWidget( labelDialog );

  QGridLayout *actionLayout = new QGridLayout( actionOptionsFrame );
  actionDialog = new QgsAttributeActionDialog ( layer->actions(), fields,
                                                actionOptionsFrame );
  actionLayout->addWidget( actionDialog,0,0 );

  QObject::connect(legendtypecombobox, SIGNAL(activated(const QString &)), this, SLOT(alterLayerDialog(const QString &)));

  //insert the renderer dialog of the vector layer into the widget stack
  widgetStackRenderers->addWidget(bufferDialog);
  widgetStackRenderers->raiseWidget(bufferDialog);

}

QgsDlgVectorLayerProperties::~QgsDlgVectorLayerProperties()
{
  widgetStackRenderers->removeWidget(bufferDialog);
  if (rendererDirty)
    {
      delete bufferDialog;
      delete bufferRenderer;
    }
}

void QgsDlgVectorLayerProperties::alterLayerDialog(const QString & dialogString)
{
/*#ifdef QGISDEBUG
    qDebug( "%s:%d QgsDlgVectorLayerProperties::alterLayerDialog(%s)", 
            __FILE__, __LINE__, dialogString );
	    #endif*/
  if (rendererDirty)
    {
      widgetStackRenderers->removeWidget(bufferDialog);
      delete bufferDialog;
      delete bufferRenderer;
    }
  //create a new Dialog
  if (dialogString == tr("Single Symbol"))
    {
      bufferRenderer = new QgsSingleSymRenderer();
  } else if (dialogString == tr("Graduated Symbol"))
    {
      bufferRenderer = new QgsGraduatedSymRenderer();
  } else if (dialogString == tr("Continuous Color"))
    {
      bufferRenderer = new QgsContinuousColRenderer();
  } else if (dialogString == tr("Single Marker"))
  {
    // On win32 we can't support svg without qt commercial
    // so we beg a bit (i know its tacky to repeat this for
    // each case, but i'm in a hurry to get the 0.5 release
    // out - besides this will go away when the money roles in...)
#ifdef WIN32
    QString msg;
    QTextOStream(&msg) << tr("In order for QGIS to support SVG markers under Windows, we need to build QGIS")
      << "\n" << tr(" using the commercial version of Qt. As this project is developed by volunteers")
      << "\n" << tr(" donating their time, we don't have the financial resources to purchase Qt")
      << "\n" << tr(" commercial.  If you would like to help us, please visit the QGIS sourceforge")
      << "\n" << tr(" home page to make a donation");
    QMessageBox::warning(this, tr("No SVG Support"), msg);
    // use the single symbol renderer
      bufferRenderer = new QgsSingleSymRenderer();
      legendtypecombobox->setCurrentText("Single Symbol");
#else
      bufferRenderer = new QgsSiMaRenderer();
#endif
  } else if (dialogString == tr("Graduated Marker"))
  {
      bufferRenderer = new QgsGraduatedMaRenderer();
  } else if(dialogString == tr("Unique Value"))
  {
      bufferRenderer = new QgsUniqueValRenderer();
  } else if(dialogString == tr("Unique Value Marker"))
  {
#ifdef WIN32
    QString msg;
    QTextOStream(&msg) << tr("In order for QGIS to support SVG markers under Windows, we need to build QGIS")
      << "\n" << tr(" using the commercial version of Qt. As this project is developed by volunteers")
      << "\n" << tr(" donating their time, we don't have the financial resources to purchase Qt")
      << "\n" << tr(" commercial.  If you would like to help us, please visit the QGIS sourceforge")
      << "\n" << tr(" home page to make a donation");
    QMessageBox::warning(this, tr("No SVG Support"), msg);

    // use the single symbol renderer
      bufferRenderer = new QgsSingleSymRenderer();
      legendtypecombobox->setCurrentText("Single Symbol");
#else
      bufferRenderer = new QgsUValMaRenderer();
#endif
  }
  bufferRenderer->initializeSymbology(layer, this);

  widgetStackRenderers->addWidget(bufferDialog);
  widgetStackRenderers->raiseWidget(bufferDialog);
  rendererDirty = true;
}

void QgsDlgVectorLayerProperties::setRendererDirty(bool enabled)
{
  rendererDirty = enabled;
}


QDialog *QgsDlgVectorLayerProperties::getBufferDialog()
{
  return bufferDialog;
}

QgsRenderer *QgsDlgVectorLayerProperties::getBufferRenderer()
{
  return bufferRenderer;
}

void QgsDlgVectorLayerProperties::setLegendType(QString type)
{
  legendtypecombobox->setCurrentText(tr(type));
}

void QgsDlgVectorLayerProperties::setDisplayField(QString name)
{
  displayFieldComboBox->setCurrentText(name);
}

void QgsDlgVectorLayerProperties::reset( void )
{
    actionDialog->init();
    labelDialog->init();
    labelCheckBox->setChecked(layer->labelOn());
}
//
// methods reimplemented from qt designer base class
//

void QgsDlgVectorLayerProperties::pbnCancel_clicked()
{
 reject();
}
void QgsDlgVectorLayerProperties::btnHelp_clicked()
{

}
void QgsDlgVectorLayerProperties::pbnOK_clicked()
{
  //make sure changes are applied
  pbnApply_clicked();
  //
  if (rendererDirty)
    {
      widgetStackRenderers->removeWidget(bufferDialog);
      delete bufferDialog;
      delete bufferRenderer;
      bufferDialog = layer->rendererDialog();
      bufferRenderer = layer->renderer();
      widgetStackRenderers->addWidget(bufferDialog);
      widgetStackRenderers->raiseWidget(bufferDialog);
      rendererDirty = false;
      //restore the right name in the combobox
      if(bufferRenderer)
      {
          legendtypecombobox->setCurrentText(tr(bufferRenderer->name()));
      }
    }
  reject();
}
void QgsDlgVectorLayerProperties::pbnApply_clicked()
{
  //
  // Set up sql subset query if applicable
  //
#ifdef HAVE_POSTGRESQL
  QgsVectorDataProvider *dp = dynamic_cast<QgsVectorDataProvider *>(layer->getDataProvider());
  //see if we are dealing with a pg layer here
  if(layer->providerType() == "postgres")
  {
    grpSubset->setEnabled(true);
    // set the subset sql for the layer
    layer->setSubsetString(txtSubsetSQL->text());   
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

  if (rendererDirty)
    {
      layer->setRenderer(bufferRenderer);
      layer->setRendererDialog(bufferDialog);
    }

  actionDialog->apply();

  labelDialog->apply();
  layer->setLabelOn(labelCheckBox->isChecked());
  layer->setLayerName(displayName());


  QgsSiSyDialog *sdialog = dynamic_cast < QgsSiSyDialog * >(layer->rendererDialog());
  QgsGraSyDialog *gdialog = dynamic_cast < QgsGraSyDialog * >(layer->rendererDialog());
  QgsContColDialog *cdialog = dynamic_cast < QgsContColDialog * >(layer->rendererDialog());
  QgsSiMaDialog* smdialog = dynamic_cast < QgsSiMaDialog * >(layer->rendererDialog());
  QgsGraMaDialog* gmdialog = dynamic_cast< QgsGraMaDialog * >(layer->rendererDialog());
  QgsUValDialog* udialog = dynamic_cast< QgsUValDialog * > (layer->rendererDialog());
  QgsUValMaDialog* umdialog = dynamic_cast< QgsUValMaDialog * > (layer->rendererDialog());

  if (sdialog)
    {
      sdialog->apply();
  } else if (gdialog)
    {
      gdialog->apply();
  } else if (cdialog)
    {
      cdialog->apply();
    }
  else if(smdialog)
  {
      smdialog->apply();
  }
  else if(gmdialog)
  {
      gmdialog->apply();
  }
  else if(udialog)
  {
      udialog->apply();
  }
  else if(umdialog)
  {
      umdialog->apply();
  }

  rendererDirty = false;
}

void QgsDlgVectorLayerProperties::pbnQueryBuilder_clicked()
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
  QgsPgQueryBuilder *pqb =
      new QgsPgQueryBuilder(myPGProvider->getURI());
       
  // Set the sql in the query builder to the same in the prop dialog
  // (in case the user has already changed it)
  pqb->setSql(txtSubsetSQL->text());
  // Open the query builder
  if(pqb->exec())
  {
    // if the sql is changed, update it in the prop subset text box
    txtSubsetSQL->setText(pqb->sql());
    //TODO If the sql is changed in the prop dialog, the layer extent should be recalculated
  }
  // delete the query builder object
  delete pqb;
#endif
}
