
/**  Base class for processing modules.
 *
 */

class QgsRasterInterface
{
%TypeHeaderCode
#include <qgsrasterinterface.h>
#include <qgsrasterresamplefilter.h>
//#include <qgsrasternuller.h>
#include <qgsrasterrenderer.h>
#include <qgsrasterprojector.h>
#include <qgsrasterdataprovider.h>

// QgsRasterRenderer subclass headers must be here because ConvertToSubClassCode
// from QgsRasterRenderer is probably included
#include <qgspalettedrasterrenderer.h>
#include <qgsmultibandcolorrenderer.h>
#include <qgssinglebandpseudocolorrenderer.h>
#include <qgssinglebandgrayrenderer.h>
#include <qgssinglebandcolordatarenderer.h>
%End

%ConvertToSubClassCode
  if (dynamic_cast<QgsRasterResampleFilter*>(sipCpp) != NULL)
    sipClass = sipClass_QgsRasterResampleFilter;
//  if (dynamic_cast<QgsRasterNuller*>(sipCpp) != NULL)
//    sipClass = sipClass_QgsRasterNuller;
  if (dynamic_cast<QgsRasterRenderer*>(sipCpp) != NULL)
    sipClass = sipClass_QgsRasterRenderer;
  if (dynamic_cast<QgsRasterProjector*>(sipCpp) != NULL)
    sipClass = sipClass_QgsRasterProjector;
  if (dynamic_cast<QgsRasterDataProvider*>(sipCpp) != NULL)
  {
    sipClass = sipClass_QgsRasterDataProvider;
    *sipCppRet = static_cast<QgsRasterDataProvider*>(sipCpp);
  }
  else
    sipClass = 0;
%End

  public:

    QgsRasterInterface( QgsRasterInterface * input = 0 );

    virtual ~QgsRasterInterface();

    /** Clone itself, create deep copy */
    virtual QgsRasterInterface *clone() const = 0 /Factory/;

    /** Returns data type for the band specified by number */
    virtual QgsRasterBlock::DataType dataType( int bandNo ) const = 0;

    int dataTypeSize ( int bandNo );

    /** Get number of bands */
    virtual int bandCount() const = 0;

    /** Return no data value for specific band. Each band/provider must have
     * no data value, if there is no one set in original data, provider decides one
     * possibly using wider data type.
     * @param bandNo band number
     * @return No data value */
    virtual double noDataValue( int bandNo ) const;

    /** Test if value is nodata for specific band
     * @param bandNo band number
     * @param value tested value
     * @return true if value is nodata */
    virtual bool isNoDataValue( int bandNo, double value ) const;

    virtual QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height ) = 0 / Factory /;

    /** Set input.
      * Returns true if set correctly, false if cannot use that input */
    virtual bool setInput( QgsRasterInterface* input );

    /** Current input */
    virtual QgsRasterInterface * input() const;

    /** Is on/off */
    virtual bool on();

    /** Set on/off */
    virtual void setOn( bool on );

    /** Get source / raw input, the first in pipe, usually provider.
     *  It may be used to get info about original data, e.g. resolution to decide
     *  resampling etc.
     */
    virtual QgsRasterInterface *srcInput();
};

