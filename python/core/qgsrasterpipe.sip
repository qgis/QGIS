
/**  Raster pipe */

class QgsRasterPipe
{
%TypeHeaderCode
#include <qgsrasterpipe.h>
#include <qgsrasterresamplefilter.h>
#include <qgsrasterprojector.h>
%End
  public:
    enum Role
    {
      UnknownRole   = 0,
      ProviderRole  = 1,
      RendererRole  = 2,
      ResamplerRole = 3,
      ProjectorRole = 4
    };

    QgsRasterPipe();
    QgsRasterPipe( const QgsRasterPipe& thePipe );

    ~QgsRasterPipe();

    bool insert( int idx, QgsRasterInterface* theInterface /Transfer/ );

    bool replace( int idx, QgsRasterInterface* theInterface /Transfer/ );

    bool set( QgsRasterInterface * theInterface /Transfer/ );

    bool remove( int idx );

    bool remove( QgsRasterInterface * theInterface );

    int size() const;
    QgsRasterInterface * at( int idx ) const;
    QgsRasterInterface * last() const;

    bool setOn( int idx, bool on );

    bool canSetOn( int idx, bool on );

    QgsRasterDataProvider * provider() const;
    QgsRasterRenderer * renderer() const;
    QgsRasterResampleFilter * resampleFilter() const;
    QgsRasterProjector * projector() const;

    void setStatsOn( bool on );
};

