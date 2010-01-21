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

    virtual QStringList groups() =0;

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

    //! show layer propeties
    //! @note added in 1.5
    virtual void legendLayerShowProperties() =0;
};

