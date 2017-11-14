/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterpipe.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsRasterPipe
{
%Docstring
 Base class for processing modules.
%End

%TypeHeaderCode
#include "qgsrasterpipe.h"
%End
  public:
    enum Role
    {
      UnknownRole,
      ProviderRole,
      RendererRole,
      BrightnessRole,
      ResamplerRole,
      ProjectorRole,
      NullerRole,
      HueSaturationRole
    };

    QgsRasterPipe();
%Docstring
 Constructor for QgsRasterPipe.
%End


    ~QgsRasterPipe();


    bool insert( int idx, QgsRasterInterface *interface /Transfer/ );
%Docstring
 Try to insert interface at specified index and connect
 if connection would fail, the interface is not inserted and false is returned *
 :rtype: bool
%End
%MethodCode
    sipRes = sipCpp->insert( a0, a1 );
    if ( !sipRes )
    {
      // if insertion failed transfer ownership back to python
      PyObject *o = sipGetPyObject( a1, sipType_QgsRasterInterface );
      if ( o )
        sipTransferBreak( o );
    }
%End

    bool replace( int idx, QgsRasterInterface *interface /Transfer/ );
%Docstring
 Try to replace interface at specified index and connect
 if connection would fail, the interface is not inserted and false is returned *
 :rtype: bool
%End

    bool set( QgsRasterInterface *interface /Transfer/ );
%Docstring
 Insert a new known interface in default place or replace interface of the same
 role if it already exists. Known interfaces are: QgsRasterDataProvider,
 QgsRasterRenderer, QgsRasterResampleFilter, QgsRasterProjector and their
 subclasses. For unknown interfaces it mus be explicitly specified position
 where it should be inserted using insert() method.
 :rtype: bool
%End

    bool remove( int idx );
%Docstring
Remove and delete interface at given index if possible
 :rtype: bool
%End

    bool remove( QgsRasterInterface *interface );
%Docstring
Remove and delete interface from pipe if possible
 :rtype: bool
%End

    int size() const;
%Docstring
 :rtype: int
%End
    QgsRasterInterface *at( int idx ) const;
%Docstring
 :rtype: QgsRasterInterface
%End
    QgsRasterInterface *last() const;
%Docstring
 :rtype: QgsRasterInterface
%End

    bool setOn( int idx, bool on );
%Docstring
 Set interface at index on/off
  Returns true on success *
 :rtype: bool
%End

    bool canSetOn( int idx, bool on );
%Docstring
Test if interface at index may be swithed on/off
 :rtype: bool
%End

    QgsRasterDataProvider *provider() const;
%Docstring
 :rtype: QgsRasterDataProvider
%End
    QgsRasterRenderer *renderer() const;
%Docstring
 :rtype: QgsRasterRenderer
%End
    QgsRasterResampleFilter *resampleFilter() const;
%Docstring
 :rtype: QgsRasterResampleFilter
%End
    QgsBrightnessContrastFilter *brightnessFilter() const;
%Docstring
 :rtype: QgsBrightnessContrastFilter
%End
    QgsHueSaturationFilter *hueSaturationFilter() const;
%Docstring
 :rtype: QgsHueSaturationFilter
%End
    QgsRasterProjector *projector() const;
%Docstring
 :rtype: QgsRasterProjector
%End
    QgsRasterNuller *nuller() const;
%Docstring
 :rtype: QgsRasterNuller
%End

  private:
    QgsRasterPipe( const QgsRasterPipe &pipe );
};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterpipe.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
