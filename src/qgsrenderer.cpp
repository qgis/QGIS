#include "qgslegendvectorsymbologyitem.h"
#include "qgsrenderer.h"
#include "qgssymbol.h"


QColor QgsRenderer::mSelectionColor=QColor(0,0,0);

QgsRenderer::QgsRenderer()
{

}

void QgsRenderer::refreshLegend(QListViewItem* legendparent) const
{
    if(legendparent)
    {
	//first remove the existing child items
	QListViewItem* tmp;
	QListViewItem* myChild = legendparent->firstChild();

        while( myChild ) 
	{
            tmp=myChild;
            myChild = myChild->nextSibling();
	    delete tmp;
        }

	//add the new items
	QString lw, uv, label;
	const std::list<QgsSymbol*> sym = symbols();
	
	for(std::list<QgsSymbol*>::const_reverse_iterator it=sym.rbegin(); it!=sym.rend(); ++it)
	{
	    QgsLegendVectorSymbologyItem* item = new QgsLegendVectorSymbologyItem(legendparent, "");
	    item->addSymbol(*it);

	    QPixmap pix;
	    if((*it)->type() == QGis::Point)
	    {
		pix = (*it)->getPointSymbolAsPixmap();
	    }
	    else if((*it)->type() == QGis::Line)
	    {
		pix = (*it)->getLineSymbolAsPixmap();
	    }
	    else //polygon
	    {
		pix = (*it)->getPolygonSymbolAsPixmap();
	    }

	    item->setPixmap(0, pix);
	    QString values;
	    lw = (*it)->lowerValue();
	    if(!lw.isEmpty())
	    {
		values += lw;
	    }
	    uv = (*it)->upperValue();
	    if(!uv.isEmpty())
	    {
		values += " - ";
		values += uv;
	    }
	    label = (*it)->label();
	    if(!label.isEmpty())
	    {
		values += " ";
		values += label;
	    }
	    item->setText(0, values);
	}
    }
}

