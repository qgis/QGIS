
/**
 * 
 * Gary Sherman
 **/
 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 #include <iostream>
 #include <fstream>
 #include <qfiledialog.h>
 #include <qdom.h>
 #include <qmessagebox.h>
 #include <qcolor.h>
 #include "qgsmaplayer.h"
 #include "qgsdatabaselayer.h"
#include "qgsmapcanvas.h"
#include "qgsprojectio.h"
 

QgsProjectIo::QgsProjectIo(QgsMapCanvas *_map, int _action) : map(_map), action(_action)

{
}


QgsProjectIo::~QgsProjectIo()
{
}
void QgsProjectIo::write(){
	selectFileName();
	//QMessageBox::information(0,"Full Path",fullPath);
	int okToSave = 0;
	if(QFile::exists(fullPath)){
		okToSave = QMessageBox::warning(0,"Overwrite File?",fullPath + " exists. \nDo you want to overwrite it?", "Yes", "No");
	}
	if(okToSave == 0){
	// write the project information to the selected file
	writeXML();
	}
}
void QgsProjectIo::read(){
	QString path = selectFileName();
	QDomDocument *doc;
	if(!path.isEmpty()){
	doc = new QDomDocument( "qgisdocument" );
    QFile file( path );
    if ( !file.open( IO_ReadOnly ) )
        return;
    if ( !doc->setContent( &file ) ) {
        file.close();
        return;
    }
    file.close();
	
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
		
		//process symbology
		mnl = node.namedItem("symbol"); 
		QDomNode snode = mnl.namedItem("linewidth");
		QDomElement lineElement = snode.toElement();
		int lineWidth = lineElement.text().toInt();
		
		
		snode = mnl.namedItem("outlinecolor");
		QDomElement colorElement = snode.toElement();
		int olRed = colorElement.attribute("red").toInt();
		int olGreen = colorElement.attribute("green").toInt();
		int olBlue = colorElement.attribute("blue").toInt();
		
		snode = mnl.namedItem("fillcolor");
		colorElement = snode.toElement();
		int fillRed = colorElement.attribute("red").toInt();
		int fillGreen = colorElement.attribute("green").toInt();
		int fillBlue = colorElement.attribute("blue").toInt();
		
		QgsSymbol *sym = new QgsSymbol();
		sym->setFillColor( QColor(fillRed, fillGreen, fillBlue));
		sym->setColor(QColor(olRed, olGreen, olBlue));
		sym->setLineWidth(lineWidth);
		// get the linewidth information
		
		//QMessageBox::information(0,"Zorder", mne.text());
		
		
		// add the layer to the maplayer
		
		if(type = "database"){
			
				QgsDatabaseLayer *dbl = new QgsDatabaseLayer(dataSource, layerName);
				
				map->addLayer(dbl);
				dbl->setSymbol(sym);
				dbl->setVisible(visible == "1");
		}else{
			if(type == "vector"){
			}else{
				if(type == "raster"){
				}
				}
			}
			
	
	}
	
	}
}
QString QgsProjectIo::selectFileName(){
if(action == SAVE && fullPath.isEmpty()){
	action = SAVEAS;
	}
switch(action){
	case OPEN:
	fullPath = QFileDialog::getOpenFileName("./", "QGis files (*.qgs)", 0,  0, "Choose a file to open" );
  
	break;
	case SAVEAS:
		fullPath = QFileDialog::getSaveFileName("./", "QGis files (*.qgs)", 0,  0, "Choose a filename  to save" );
	break;
	}
	return fullPath;
}
void QgsProjectIo::writeXML(){
	std::ofstream xml(fullPath);
	if(!xml.fail()){
		xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
		xml << "<!DOCTYPE qgis SYSTEM \"http://mrcc.com/qgis.dtd\">" << std::endl;
		xml << "<qgis projectname=\"default project\">\n";
		xml << "<title>QGis Project File</title>\n";
		xml << "<projectlayers layercount=\"" << map->layerCount() << "\"> \n";
		// write the layers
		for(int i = 0; i < map->layerCount(); i++){
			QgsMapLayer *lyr = map->getZpos(i);
			
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
					break;
			}
			xml << "\" visible=\"";
			if(lyr->visible()){
				xml << "1";
			}else{
				xml << "0";
			}
			xml << "\">\n";
			xml << "\t\t<layername>" + lyr->name() + "</layername>\n";
			xml << "\t\t<datasource>" + lyr->source() + "</datasource>\n";
			xml << "\t\t<zorder>" << i << "</zorder>\n";
			xml << "\t\t<symbol>\n";
			QgsSymbol *sym = lyr->symbol();
			xml << "\t\t\t<linewidth>" << sym->lineWidth() << "</linewidth>\n";
			QColor outlineColor = sym->color();
			xml << "\t\t\t<outlinecolor red=\"" << outlineColor.red() << "\" green=\"" 
				<< outlineColor.green() << "\" blue=\"" << outlineColor.blue() << "\" />\n";
			QColor fillColor = sym->fillColor();
			xml << "\t\t\t<fillcolor red=\"" << fillColor.red() << "\" green=\"" 
				<< fillColor.green() << "\" blue=\"" << fillColor.blue() << "\" />\n";

			xml << "\t\t</symbol>\n";
			xml << "\t</maplayer>\n";
		}
		xml << "</projectlayers>\n";
		xml << "</qgis>\n";
		xml.close();
	}else{
	}
}
