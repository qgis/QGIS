/**
 * \class QgsLegendInterface
 * \brief Abstract base class to make QgsLegend available to plugins.
 */
class QgsLegendInterface : QObject
{
%TypeHeaderCode
#include <qgslegendinterface.h>
%End

  public:

    /** Constructor */
    QgsLegendInterface();

    /** Virtual destructor */
    ~QgsLegendInterface();

    //! Return a string list of groups
    virtual QStringList groups() =0;

    //! Return all layers in the project in legend order
    //! @note added in 1.5
    virtual QList< QgsMapLayer * > layers() const =0;

    //! Check if a group exists
    //! @note added in 1.5
    virtual bool groupExists( int groupIndex ) =0;

    //! Check if a group is expanded
    //! @note added in 1.5
    virtual bool isGroupExpanded( int groupIndex ) =0;

    //! Check if a group is visible
    //! @note added in 1.5
    virtual bool isGroupVisible( int groupIndex ) =0;

    //! Check if a layer is visible
    //! @note added in 1.5
    virtual bool isLayerVisible( QgsMapLayer * ml ) =0;

  signals:

    //! emitted when a group index has changed
    void groupIndexChanged( int oldIndex, int newIndex );

  public slots:

    //! Add a new group
    virtual int addGroup( QString name, bool expand = true ) =0;

    //! Remove group on index
    virtual void removeGroup( int groupIndex ) =0;

    //! Move a layer to a group
    virtual void moveLayer( QgsMapLayer * layer, int groupIndex ) =0;

    //! Collapse or expand a group
    //! @note added in 1.5
    virtual void setGroupExpanded( int groupIndex, bool expand ) =0;

    //! Set the visibility of a group
    //! @note added in 1.5
    virtual void setGroupVisible( int groupIndex, bool visible ) =0;

    //! Set the visibility of a layer
    //! @note added in 1.5
    virtual void setLayerVisible( QgsMapLayer * ml, bool visible ) =0;

    //! refresh layer symbology
    //! @note added in 1.5
    virtual void refreshLayerSymbology( QgsMapLayer *layer ) =0;
};

