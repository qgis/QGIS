/***************************************************************************
    qgsprojectio.cpp - Save/Restore QGIS project
     --------------------------------------
    Date                 : 19-Oct-2003
    Copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* qgsprojectio.cpp,v 1.25 2004/02/04 12:03:43 mhugent Exp */
#include <iostream>
#include <fstream>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qdom.h>
#include <qmessagebox.h>
#include <qcolor.h>
#include "qgsmaplayer.h"
#include "qvariant.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsmapcanvas.h"
#include "qgsrect.h"
#include "qgsprojectio.h"
#include "qgssinglesymrenderer.h"
#include "qgsgraduatedsymrenderer.h"
#include "qgscontinuouscolrenderer.h"
#include "qgssymbologyutils.h"
#include "qgssisydialog.h"
#include "qgsgrasydialog.h"
#include "qgscontcoldialog.h"
#include "qgsdlgvectorlayerproperties.h"
#include "qgisapp.h"

QgsProjectIo::QgsProjectIo(QgsMapCanvas *_map, int _action, QgisApp *qgis) 
  : map(_map), action(_action), qgisApp(qgis)

{
}


QgsProjectIo::~QgsProjectIo()
{
}
QString QgsProjectIo::baseName(){
QFileInfo fi(fullPath);
return fi.baseName(true);
}
bool QgsProjectIo::write(){
	if(fullPath.isEmpty()){
		selectFileName();
	}
	//QMessageBox::information(0,"Full Path",fullPath);
	int okToSave = 0;
	if(QFile::exists(fullPath) && (action == SAVEAS)){
		okToSave = QMessageBox::warning(0,QObject::tr("Overwrite File?"),QObject::tr("%1 exists.%2Do you want to overwrite it?").arg(fullPath).arg("\n"), QObject::tr("Yes"), QObject::tr("No"));
	}
	if(okToSave == 0){
	// write the project information to the selected file
		writeXML();
		return true;
	}else{
		return false;
		}
}

bool QgsProjectIo::read(){
	QString path = selectFileName();
	QDomDocument *doc;
	if(!path.isEmpty()){
		doc = new QDomDocument( "qgisdocument" );
		QFile file( path );
		if ( !file.open( IO_ReadOnly ) )
		{
		    return false;
		}
		if ( !doc->setContent( &file) ) 
		{
		    file.close();
		    return false;
		}
		file.close();
		qWarning("opened document");
		// clear the map canvas
		map->removeAll();
		// get the extent
		QDomNodeList extents = doc->elementsByTagName("extent");
		QDomNode extentNode = extents.item(0);
		QDomNode xminNode = extentNode.namedItem("xmin");
		QDomNode yminNode = extentNode.namedItem("ymin");
		QDomNode xmaxNode = extentNode.namedItem("xmax");
		QDomNode ymaxNode = extentNode.namedItem("ymax");
		QDomElement exElement = xminNode.toElement();
		double xmin = exElement.text().toDouble();
		exElement = yminNode.toElement();
		double ymin = exElement.text().toDouble();
		exElement = xmaxNode.toElement();
		double xmax = exElement.text().toDouble();
		exElement = ymaxNode.toElement();
		double ymax = exElement.text().toDouble();
		QgsRect savedExtent(xmin,ymin,xmax,ymax);
		
		
		QDomNodeList nl = doc->elementsByTagName("maplayer");
		QString layerCount;
		layerCount = layerCount.setNum(nl.count());
		//QMessageBox::information(0, "Number of map layers", layerCount);
		QString wk;
		// process the map layer nodes
		for(int i = 0; i < nl.count(); i++){
			QDomNode node = nl.item(i);
			QDomElement element = node.toElement();
			QString type = element.attribute("type");
			QString visible = element.attribute("visible");
			
			//QMessageBox::information(0,"Type of map layer", type);
			// process layer name
			QDomNode mnl = node.namedItem("layername"); 
			QTextStream ts( &wk, IO_WriteOnly );
			ts << mnl.nodeType();
			//QMessageBox::information(0,"Node Type", wk);
			QDomElement mne = mnl.toElement();
			//QMessageBox::information(0,"Layer Name", mne.text());
			QString layerName = mne.text();
		
			//process data source
			mnl = node.namedItem("datasource"); 
			mne = mnl.toElement();
			//QMessageBox::information(0,"Datasource Name", mne.text());
			QString dataSource = mne.text();
			
			//process zorder
			mnl = node.namedItem("zorder"); 
			mne = mnl.toElement();
			//QMessageBox::information(0,"Zorder", mne.text());
			
			// add the layer to the maplayer
			
			if(type == "vector")
			{
			    QString provider;
			    // determine type of vector layer
			    if((dataSource.find("host=") > -1) && (dataSource.find("dbname=") > -1))
			    {
				provider = "postgres";
			    }
			    else
			    {
				provider = "ogr";
			    }
			    QgsVectorLayer *dbl = new QgsVectorLayer(dataSource, layerName, provider);
			    
			    QDomNode singlenode=node.namedItem("singlesymbol");
			    QDomNode graduatednode=node.namedItem("graduatedsymbol");
			    QDomNode continuousnode=node.namedItem("continuoussymbol");

			    if(!singlenode.isNull())//read configuration for single symbol
			    {
				QgsSymbol sy;
				QPen pen;
				QBrush brush;

				QDomNode rinode=singlenode.namedItem("renderitem");

				QDomNode vnode=rinode.namedItem("value");
				QDomElement velement =vnode.toElement();
				QString value=velement.text();

				QDomNode synode=rinode.namedItem("symbol");

				QDomNode outlcnode=synode.namedItem("outlinecolor");
				QDomElement oulcelement=outlcnode.toElement();
				int red = oulcelement.attribute("red").toInt();
				int green = oulcelement.attribute("green").toInt();
				int blue = oulcelement.attribute("blue").toInt();
				pen.setColor(QColor(red,green,blue));

				QDomNode outlstnode=synode.namedItem("outlinestyle");
				QDomElement outlstelement=outlstnode.toElement();
				pen.setStyle(QgsSymbologyUtils::qString2PenStyle(outlstelement.text()));

				QDomNode outlwnode=synode.namedItem("outlinewidth");
				QDomElement outlwelement=outlwnode.toElement();
				pen.setWidth(outlwelement.text().toInt());

				QDomNode fillcnode=synode.namedItem("fillcolor");
				QDomElement fillcelement=fillcnode.toElement();
				red=fillcelement.attribute("red").toInt();
				green = fillcelement.attribute("green").toInt();
				blue = fillcelement.attribute("blue").toInt();
				brush.setColor(QColor(red,green,blue));

				QDomNode fillpnode=synode.namedItem("fillpattern");
				QDomElement fillpelement=fillpnode.toElement();
				brush.setStyle(QgsSymbologyUtils::qString2BrushStyle(fillpelement.text()));

				QDomNode lnode=rinode.namedItem("label");
				QDomElement lnodee=lnode.toElement();
				QString label=lnodee.text();

				//create a renderer and add it to the vector layer
				sy.setBrush(brush);
				sy.setPen(pen);
				QgsRenderItem ri(sy,value,label);
				QgsSingleSymRenderer* srenderer=new QgsSingleSymRenderer();
				srenderer->addItem(ri);
				dbl->setRenderer(srenderer);
				QgsSiSyDialog* sdialog=new QgsSiSyDialog(dbl);
				dbl->setRendererDialog(sdialog);

				QgsDlgVectorLayerProperties* properties = new QgsDlgVectorLayerProperties(dbl);
				dbl->setLayerProperties(properties);
				properties->setLegendType("graduated symbol");
				
				sdialog->apply();
			    }

			    else if(!graduatednode.isNull())//read configuration for graduated symbol
			    {
				QgsGraduatedSymRenderer* grenderer=new QgsGraduatedSymRenderer();

				QDomNode classnode=graduatednode.namedItem("classificationfield");
				int classificationfield=classnode.toElement().text().toInt();
				grenderer->setClassificationField(classificationfield);
				
				QDomNode rangerendernode=graduatednode.namedItem("rangerenderitem");
				while(!rangerendernode.isNull())
				{
				    QgsSymbol sy;
				    QPen pen;
				    QBrush brush;

				    QDomNode lvnode=rangerendernode.namedItem("lowervalue");
				    QString lowervalue=lvnode.toElement().text();

				    QDomNode uvnode=rangerendernode.namedItem("uppervalue");
				    QString uppervalue=uvnode.toElement().text();

				    QDomNode synode=rangerendernode.namedItem("symbol");

				    QDomElement oulcelement=synode.namedItem("outlinecolor").toElement();
				    int red = oulcelement.attribute("red").toInt();
				    int green = oulcelement.attribute("green").toInt();
				    int blue = oulcelement.attribute("blue").toInt();
				    pen.setColor(QColor(red,green,blue));
				    
				    QDomElement oustelement=synode.namedItem("outlinestyle").toElement();
				    pen.setStyle(QgsSymbologyUtils::qString2PenStyle(oustelement.text()));

				    QDomElement oulwelement=synode.namedItem("outlinewidth").toElement();
				    pen.setWidth(oulwelement.text().toInt());

				    QDomElement fillcelement=synode.namedItem("fillcolor").toElement();
				    red = fillcelement.attribute("red").toInt();
				    green = fillcelement.attribute("green").toInt();
				    blue = fillcelement.attribute("blue").toInt();
				    brush.setColor(QColor(red,green,blue));

				    QDomElement fillpelement=synode.namedItem("fillpattern").toElement();
				    brush.setStyle(QgsSymbologyUtils::qString2BrushStyle(fillpelement.text()));
				    
				    QDomElement labelelement=rangerendernode.namedItem("label").toElement();
				    QString label=labelelement.text();

                                    //create a renderitem and add it to the renderer
				    sy.setBrush(brush);
				    sy.setPen(pen);
				    
				    QgsRangeRenderItem* ri=new QgsRangeRenderItem(sy,lowervalue,uppervalue,label);
				    grenderer->addItem(ri);

				    rangerendernode=rangerendernode.nextSibling();
				}

				dbl->setRenderer(grenderer);
				QgsGraSyDialog* gdialog=new QgsGraSyDialog(dbl);
				dbl->setRendererDialog(gdialog);

				QgsDlgVectorLayerProperties* properties = new QgsDlgVectorLayerProperties(dbl);
				dbl->setLayerProperties(properties);
				properties->setLegendType("graduated symbol");

				gdialog->apply();
			    }

			    else if(!continuousnode.isNull())//read configuration for continuous symbol
			    {
				qWarning("continuous node");
				QgsSymbol lsy,usy;
				QPen lpen, upen;
				QBrush lbrush, ubrush;

				QgsContinuousColRenderer* crenderer=new QgsContinuousColRenderer();

				QDomNode classnode=continuousnode.namedItem("classificationfield");
				int classificationfield=classnode.toElement().text().toInt();


				//read the settings for the renderitem of the minimum value
				QDomNode lowernode=continuousnode.namedItem("lowestitem");
				QDomNode litemnode=lowernode.namedItem("renderitem");
				QString lvalue=litemnode.namedItem("value").toElement().text();

				QDomNode lsymbol=litemnode.namedItem("symbol");

				QDomElement loulcelement=lsymbol.namedItem("outlinecolor").toElement();
				int red = loulcelement.attribute("red").toInt();
				int green = loulcelement.attribute("green").toInt();
				int blue = loulcelement.attribute("blue").toInt();
				lpen.setColor(QColor(red,green,blue));

				QDomElement loustelement=lsymbol.namedItem("outlinestyle").toElement();
				lpen.setStyle(QgsSymbologyUtils::qString2PenStyle(loustelement.text()));
				
				QDomElement loulwelement=lsymbol.namedItem("outlinewidth").toElement();
				lpen.setWidth(loulwelement.text().toInt());

				QDomElement lfillcelement=lsymbol.namedItem("fillcolor").toElement();
				red = lfillcelement.attribute("red").toInt();
				green = lfillcelement.attribute("green").toInt();
				blue = lfillcelement.attribute("blue").toInt();
				lbrush.setColor(QColor(red,green,blue));

				QDomElement lfillpelement=lsymbol.namedItem("fillpattern").toElement();
				lbrush.setStyle(QgsSymbologyUtils::qString2BrushStyle(lfillpelement.text()));

				QString llabel=litemnode.namedItem("label").toElement().text();

				//read the settings tor the renderitem of the maximum value
				QDomNode uppernode=continuousnode.namedItem("highestitem");
				QDomNode uitemnode=uppernode.namedItem("renderitem");
				QString uvalue=uitemnode.namedItem("value").toElement().text();

				QDomNode usymbol=uitemnode.namedItem("symbol");
				
				QDomElement uoulcelement=usymbol.namedItem("outlinecolor").toElement();
				red = uoulcelement.attribute("red").toInt();
				green = uoulcelement.attribute("green").toInt();
				blue = uoulcelement.attribute("blue").toInt();
				upen.setColor(QColor(red,green,blue));

				QDomElement uoustelement=usymbol.namedItem("outlinestyle").toElement();
				upen.setStyle(QgsSymbologyUtils::qString2PenStyle(uoustelement.text()));
				
				QDomElement uoulwelement=usymbol.namedItem("outlinewidth").toElement();
				upen.setWidth(uoulwelement.text().toInt());

				QDomElement ufillcelement=usymbol.namedItem("fillcolor").toElement();
				red = ufillcelement.attribute("red").toInt();
				qWarning("red: " + QString::number(red));
				green = ufillcelement.attribute("green").toInt();
				qWarning("green: " + QString::number(green));
				blue = ufillcelement.attribute("blue").toInt();
				qWarning("blue: " + QString::number(blue));
				ubrush.setColor(QColor(red,green,blue));

				QDomElement ufillpelement=usymbol.namedItem("fillpattern").toElement();
				ubrush.setStyle(QgsSymbologyUtils::qString2BrushStyle(ufillpelement.text()));

				QString ulabel=uitemnode.namedItem("label").toElement().text();

				//add all together
				lsy.setPen(lpen);
				lsy.setBrush(lbrush);
				usy.setPen(upen);
				usy.setBrush(ubrush);

				QgsRenderItem* litem=new QgsRenderItem(lsy,lvalue,llabel);
				QgsRenderItem* uitem=new QgsRenderItem(usy,uvalue,ulabel);

				crenderer->setMinimumItem(litem);
				crenderer->setMaximumItem(uitem);

				dbl->setRenderer(crenderer);
				QgsContColDialog* cdialog=new QgsContColDialog(dbl);
				dbl->setRendererDialog(cdialog);

				QgsDlgVectorLayerProperties* properties = new QgsDlgVectorLayerProperties(dbl);
				dbl->setLayerProperties(properties);
				properties->setLegendType("continuous color");

				cdialog->apply();

			    }
			    
			    dbl->setVisible(visible == "1");
			    qWarning("adde den Layer");
          dbl->initContextMenu(qgisApp);
			    map->addLayer(dbl);
			} 
			else if ( type == "raster" ) 
			{
				QgsRasterLayer *myRasterLayer = new QgsRasterLayer(dataSource, layerName);
				myRasterLayer->initContextMenu(qgisApp);
				map->addLayer(myRasterLayer);
                                
				myRasterLayer->setVisible(visible == "1");
                                
				mnl = node.namedItem("rasterproperties"); 
                                
				QDomNode snode = mnl.namedItem("showDebugOverlayFlag");
				QDomElement myElement = snode.toElement();
                                QVariant myQVariant = (QVariant) myElement.attribute("boolean");
				myRasterLayer->setShowDebugOverlayFlag(myQVariant.toBool());

				snode = mnl.namedItem("drawingStyle");
				myElement = snode.toElement();
				myRasterLayer->setDrawingStyle(myElement.text());				
                                
				snode = mnl.namedItem("invertHistogramFlag");
				myElement = snode.toElement();
                                myQVariant = (QVariant) myElement.attribute("boolean");
				myRasterLayer->setInvertHistogramFlag(myQVariant.toBool());

				snode = mnl.namedItem("stdDevsToPlotDouble");
				myElement = snode.toElement();
				myRasterLayer->setStdDevsToPlot(myElement.text().toDouble());                                                         

				snode = mnl.namedItem("transparencyLevelInt");
				myElement = snode.toElement();
				myRasterLayer->setTransparency(myElement.text().toInt());  

				snode = mnl.namedItem("redBandNameQString");
				myElement = snode.toElement();
				myRasterLayer->setRedBandName(myElement.text());                                                                         				
				snode = mnl.namedItem("greenBandNameQString");
				myElement = snode.toElement();
				myRasterLayer->setGreenBandName(myElement.text()); 
                                
				snode = mnl.namedItem("blueBandNameQString");
				myElement = snode.toElement();
				myRasterLayer->setBlueBandName(myElement.text());               

				snode = mnl.namedItem("grayBandNameQString");
				myElement = snode.toElement();
				myRasterLayer->setGrayBandName(myElement.text());                                                                                   			
                                
			}
			map->setExtent(savedExtent);
		}
		return true;
	}
}
	
QString QgsProjectIo::selectFileName(){
if(action == SAVE && fullPath.isEmpty()){
	action = SAVEAS;
	}
switch(action){
	case OPEN:
	fullPath = QFileDialog::getOpenFileName("./", QObject::tr("QGis files (*.qgs)"), 0,  0, QObject::tr("Choose a QGIS project file to open") );
	break;
	case SAVEAS:
		fullPath = QFileDialog::getSaveFileName("./", QObject::tr("QGis files (*.qgs)"), 0,  0, QObject::tr("Choose a filename to save") );
	break;
	}
	return fullPath;
}

void QgsProjectIo::setFileName(QString fn){
	fullPath = fn;
	}
        
QString QgsProjectIo::fullPathName(){
	return fullPath;
	}
        
        
        
void QgsProjectIo::writeXML(){
	std::ofstream xml(fullPath);
	if(!xml.fail()){
		xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
		xml << "<!DOCTYPE qgis SYSTEM \"http://mrcc.com/qgis.dtd\">" << std::endl;
		xml << "<qgis projectname=\"default project\">\n";
		xml << "<title>QGis Project File</title>\n";
		
		xml << "<extent>\n";
		QgsRect extent = map->extent();
		
		xml << "\t<xmin>" << extent.xMin() << "</xmin>\n";
		xml << "\t<ymin>" << extent.yMin() << "</ymin>\n";
		xml << "\t<xmax>" << extent.xMax() << "</xmax>\n";
		xml << "\t<ymax>" << extent.yMax() << "</ymax>\n";
		xml << "</extent>\n";
		
		xml << "<projectlayers layercount=\"" << map->layerCount() << "\"> \n";
		// write the layers
		for(int i = 0; i < map->layerCount(); i++){
			QgsMapLayer *lyr = map->getZpos(i);
			bool isDatabase = false;
			xml << "\t<maplayer type=\"";
			switch(lyr->type()){
				case QgsMapLayer::VECTOR:
					xml << "vector";
					break;
				case QgsMapLayer::RASTER:
					xml << "raster";
					break;
				case QgsMapLayer::DATABASE:
					xml << "database";
					isDatabase = true;
					break;
			}
			xml << "\" visible=\"";
			if(lyr->visible()){
				xml << "1";
			}else{
				xml << "0";
			}
			xml << "\">\n";
			if(isDatabase){
				// cast the layer to a qgsdatabaselayer
				// TODO fix this so database layers are properly saved/restored 
				// when name is changed in legend
				/* QgsDatabaseLayer *dblyr = (QgsDatabaseLayer *)lyr;
				xml << "\t\t<layername>" + dblyr->schemaName() << "." <<
					dblyr->geometryTableName() << "</layername>\n"; */
				xml << "\t\t<layername>" + lyr->name() + "</layername>\n";
			}else{            
				xml << "\t\t<layername>" + lyr->name() + "</layername>\n";
			}
			xml << "\t\t<datasource>" + lyr->source() + "</datasource>\n";
			xml << "\t\t<zorder>" << i << "</zorder>\n";
			if ( lyr->type() != QgsMapLayer::RASTER ) 
			{
			    QgsVectorLayer* layer=dynamic_cast<QgsVectorLayer*>(lyr);
			    if(!layer)
			    {
				qWarning("Warning, cast failed in QgsProjectIo, line 309");
			    }
			    QgsSingleSymRenderer* srenderer=dynamic_cast<QgsSingleSymRenderer*>(layer->renderer());
			    QgsGraduatedSymRenderer* grenderer=dynamic_cast<QgsGraduatedSymRenderer*>(layer->renderer());
			    QgsContinuousColRenderer* crenderer=dynamic_cast<QgsContinuousColRenderer*>(layer->renderer());
			    if(srenderer)
			    {
				xml << "\t\t<singlesymbol>\n";
				xml << "\t\t\t<renderitem>\n";
				xml << "\t\t\t\t<value>"+srenderer->item()->value()+"</value>\n";
				QgsSymbol* symbol=srenderer->item()->getSymbol();
				xml << "\t\t\t\t<symbol>\n";
				xml << "\t\t\t\t\t<outlinecolor red=\""+QString::number(symbol->pen().color().red())+"\" green=\""+QString::number(symbol->pen().color().green())+"\" blue=\""+QString::number(symbol->pen().color().blue())+"\" />\n";
				xml << "\t\t\t\t\t<outlinestyle>"+QgsSymbologyUtils::penStyle2QString(symbol->pen().style())+"</outlinestyle>\n";
				xml << "\t\t\t\t\t<outlinewidth>"+QString::number(symbol->pen().width())+"</outlinewidth>\n"; 
				xml << "\t\t\t\t\t<fillcolor red=\""+QString::number(symbol->brush().color().red())+"\" green=\""+QString::number(symbol->brush().color().green())+"\" blue=\""+QString::number(symbol->brush().color().blue())+"\" />\n";
				xml << "\t\t\t\t\t<fillpattern>"+QgsSymbologyUtils::brushStyle2QString(symbol->brush().style())+"</fillpattern>\n";
				xml << "\t\t\t\t</symbol>\n";
				xml << "\t\t\t\t<label>"+srenderer->item()->label()+"</label>\n";
				xml << "\t\t\t</renderitem>\n";
				xml << "\t\t</singlesymbol>\n";
			    }
			    else if(grenderer)
			    {
				xml << "\t\t<graduatedsymbol>\n";
				xml << "\t\t\t<classificationfield>"+QString::number(grenderer->classificationField())+"</classificationfield>\n";
				for(std::list<QgsRangeRenderItem*>::iterator it=grenderer->items().begin();it!=grenderer->items().end();++it)
				{
				    xml << "\t\t\t<rangerenderitem>\n";
				    xml << "\t\t\t\t<lowervalue>"+(*it)->value()+"</lowervalue>\n";
				    xml << "\t\t\t\t<uppervalue>"+(*it)->upper_value()+"</uppervalue>\n";
				    xml << "\t\t\t\t<symbol>\n";
				    QgsSymbol* symbol=(*it)->getSymbol();
				    xml << "\t\t\t\t\t<outlinecolor red=\""+QString::number(symbol->pen().color().red())+"\" green=\""+QString::number(symbol->pen().color().green())+"\" blue=\""+QString::number(symbol->pen().color().blue())+"\" />\n";
				    xml << "\t\t\t\t\t<outlinestyle>"+QgsSymbologyUtils::penStyle2QString(symbol->pen().style())+"</outlinestyle>\n";
				    xml << "\t\t\t\t\t<outlinewidth>"+QString::number(symbol->pen().width())+"</outlinewidth>\n";
				    xml << "\t\t\t\t\t<fillcolor red=\""+QString::number(symbol->brush().color().red())+"\" green=\""+QString::number(symbol->brush().color().green())+"\" blue=\""+QString::number(symbol->brush().color().blue())+"\" />\n";
				    xml << "\t\t\t\t\t<fillpattern>"+QgsSymbologyUtils::brushStyle2QString(symbol->brush().style())+"</fillpattern>\n";
				    xml << "\t\t\t\t</symbol>\n";
				    xml << "\t\t\t\t<label>"+(*it)->label()+"</label>\n";
				    xml << "\t\t\t</rangerenderitem>\n";
				}
				xml << "\t\t</graduatedsymbol>\n";
			    }
			    else if(crenderer)
			    {
				xml << "\t\t<continuoussymbol>\n"; 
				xml << "\t\t\t<classificationfield>"+QString::number(crenderer->classificationField())+"</classificationfield>\n";


				QgsRenderItem* lowestitem=crenderer->minimumItem();
				QgsSymbol* lsymbol=lowestitem->getSymbol();
				xml << "\t\t\t<lowestitem>\n";
				xml << "\t\t\t\t<renderitem>\n";
				xml << "\t\t\t\t<value>"+lowestitem->value()+"</value>\n";
				xml << "\t\t\t\t\t<symbol>\n";
				xml << "\t\t\t\t\t\t<outlinecolor red=\""+QString::number(lsymbol->pen().color().red())+"\" green=\""+QString::number(lsymbol->pen().color().green())+"\" blue=\""+QString::number(lsymbol->pen().color().blue())+"\" />\n";
				xml << "\t\t\t\t\t\t<outlinestyle>"+QgsSymbologyUtils::penStyle2QString(lsymbol->pen().style())+"</outlinestyle>\n";
				xml << "\t\t\t\t\t\t<outlinewidth>"+QString::number(lsymbol->pen().width())+"</outlinewidth>\n"; 
				xml << "\t\t\t\t\t\t<fillcolor red=\""+QString::number(lsymbol->brush().color().red())+"\" green=\""+QString::number(lsymbol->brush().color().green())+"\" blue=\""+QString::number(lsymbol->brush().color().blue())+"\" />\n";
				xml << "\t\t\t\t\t\t<fillpattern>"+QgsSymbologyUtils::brushStyle2QString(lsymbol->brush().style())+"</fillpattern>\n";
				xml << "\t\t\t\t\t</symbol>\n";
				xml << "\t\t\t\t\t<label>"+lowestitem->label()+"</label>\n";
				xml << "\t\t\t\t</renderitem>\n";
				xml << "\t\t\t</lowestitem>\n";
				
				QgsRenderItem* highestitem=crenderer->maximumItem();
				QgsSymbol* hsymbol=highestitem->getSymbol();
				xml << "\t\t\t<highestitem>\n";
				xml << "\t\t\t\t<renderitem>\n";
				xml << "\t\t\t\t<value>"+highestitem->value()+"</value>\n";
				xml << "\t\t\t\t\t<symbol>\n";
				xml << "\t\t\t\t\t\t<outlinecolor red=\""+QString::number(hsymbol->pen().color().red())+"\" green=\""+QString::number(hsymbol->pen().color().green())+"\" blue=\""+QString::number(hsymbol->pen().color().blue())+"\" />\n";
				xml << "\t\t\t\t\t\t<outlinestyle>"+QgsSymbologyUtils::penStyle2QString(hsymbol->pen().style())+"</outlinestyle>\n";
				xml << "\t\t\t\t\t\t<outlinewidth>"+QString::number(hsymbol->pen().width())+"</outlinewidth>\n"; 
				xml << "\t\t\t\t\t\t<fillcolor red=\""+QString::number(hsymbol->brush().color().red())+"\" green=\""+QString::number(hsymbol->brush().color().green())+"\" blue=\""+QString::number(hsymbol->brush().color().blue())+"\" />\n";
				xml << "\t\t\t\t\t\t<fillpattern>"+QgsSymbologyUtils::brushStyle2QString(hsymbol->brush().style())+"</fillpattern>\n";
				xml << "\t\t\t\t\t</symbol>\n";
				xml << "\t\t\t\t\t<label>"+highestitem->label()+"</label>\n";
				xml << "\t\t\t\t</renderitem>\n";
				xml << "\t\t\t</highestitem>\n";
				xml << "\t\t</continuoussymbol>\n";
			    }
	      
			    /*xml << "\t\t<symbol>\n";
				QgsSymbol *sym = lyr->symbol();
				xml << "\t\t\t<linewidth>" << sym->lineWidth() << "</linewidth>\n";
				QColor outlineColor = sym->color();
				xml << "\t\t\t<outlinecolor red=\"" << outlineColor.red() << "\" green=\"" 
						<< outlineColor.green() << "\" blue=\"" << outlineColor.blue() << "\" />\n";
				QColor fillColor = sym->fillColor();
				xml << "\t\t\t<fillcolor red=\"" << fillColor.red() << "\" green=\"" 
						<< fillColor.green() << "\" blue=\"" << fillColor.blue() << "\" />\n";
				
						xml << "\t\t</symbol>\n";*/
			}
                        else //raster layer properties
                        {
                          //cast the maplayer to rasterlayer
                          QgsRasterLayer *myRasterLayer = (QgsRasterLayer *) lyr;
                          //Raster flag to indicate whether debug infor overlay should be rendered onto the raster
                     
                          xml << "\t\t<rasterproperties>\n";
                            xml << "\t\t\t<showDebugOverlayFlag boolean=\"" ;
                            if (myRasterLayer->getShowDebugOverlayFlag())
                            {
                              xml << "true\"/>\n";
                            }
                            else
                            {
                              xml << "false\"/>\n";
                            }
                            
                            // The drawing style for the layer
                            xml << "\t\t\t<drawingStyle>" << myRasterLayer->getDrawingStyleAsQString() << "</drawingStyle>\n" ; 
                            //Raster : flag indicating whether the histogram should be inverted or not 
                            xml << "\t\t\t<invertHistogramFlag boolean=\"" ;
                            if (myRasterLayer->getInvertHistogramFlag())
                            {
                              xml << "true\"/>\n";
                            }
                            else
                            {
                              xml << "false\"/>\n";
                            }                              
                            //Raster : Number of stddev to plot (0) to ignore -->
                            xml << "\t\t\t<stdDevsToPlotDouble>" << myRasterLayer->getStdDevsToPlot() << "</stdDevsToPlotDouble>\n" ;
                            //Raster transparency for this layer should be 0-255 -->
                            xml << "\t\t\t<transparencyLevelInt>" << myRasterLayer->getTransparency() << "</transparencyLevelInt>\n" ;     
                            //Raster : the band to be associated with the color red - usually red -->
                            xml << "\t\t\t<redBandNameQString>" << myRasterLayer->getRedBandName() << "</redBandNameQString>\n" ;                             
                            //Raster : the band to be associated with the color green - usually green -->
                            xml << "\t\t\t<greenBandNameQString>" << myRasterLayer->getGreenBandName() << "</greenBandNameQString>\n" ; 
                            //Raster : the band to be associated with the color blue - usually blue -->
                            xml << "\t\t\t<blueBandNameQString>" << myRasterLayer->getBlueBandName() << "</blueBandNameQString>\n" ; 
                            //Raster :  the band to be associated with the grayscale only ouput - usually gray  -->
                            xml << "\t\t\t<grayBandNameQString>" << myRasterLayer->getGrayBandName() << "</grayBandNameQString>\n" ; 
                          xml << "\t\t</rasterproperties>\n";                          
                        }
			xml << "\t</maplayer>\n";
		}
		xml << "</projectlayers>\n";
		xml << "</qgis>\n";
		xml.close();
	}else{
	}
}
