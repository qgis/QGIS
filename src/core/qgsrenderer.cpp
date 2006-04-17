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

void QgsRenderer::refreshLegend(std::list< std::pair<QString, QPixmap> >* symbologyList) const
{
    if(symbologyList)
    {
      //add the new items
      QString lw, uv, label;
      const std::list<QgsSymbol*> sym = symbols();
	
      for(std::list<QgsSymbol*>::const_iterator it=sym.begin(); it!=sym.end(); ++it)
	{
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
	  symbologyList->push_back(std::make_pair(values, pix));
	}
    }
}

