#include "qgslegendvectorsymbologyitem.h"
#include "qgsrenderer.h"
#include "qgssymbol.h"
//Added by qt3to4:
#include <QPixmap>
#include <QIcon>


QColor QgsRenderer::mSelectionColor=QColor(0,0,0);

QgsRenderer::QgsRenderer()
{

}

void QgsRenderer::refreshLegend(QTreeWidgetItem* legendparent) const
{
    if(legendparent)
    {
	
      //first remove the existing childs
      //legendparent->takeChildren();
      dynamic_cast<QgsLegendItem*>(legendparent)->removeAllChildren();

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
	  
	  QIcon theIcon(pix);
	  item->setIcon(0, theIcon);
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

