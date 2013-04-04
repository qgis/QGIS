%ModuleHeaderCode
#include <qgslinevectorlayerdirector.h>
%End

/**
 * \ingroup networkanalysis
 * \class QgsGraphDirector
 * \brief Determine making the graph. QgsGraphBuilder and QgsGraphDirector is a builder patter.
 */
class QgsGraphDirector : QObject
{
%TypeHeaderCode
#include <qgsgraphbuilder.h>
%End

%ConvertToSubClassCode
  if ( dynamic_cast< QgsLineVectorLayerDirector* > ( sipCpp ) != NULL )
    sipClass = sipClass_QgsLineVectorLayerDirector;
  else
    sipClass = NULL;
%End


  signals:
    void buildProgress( int, int ) const;
    void buildMessage( QString ) const;

  public:
    //! Destructor
    virtual ~QgsGraphDirector();

    /**
     * Make a graph using RgGraphBuilder
     *
     * @param builder   The graph builder
     *
     * @param additionalPoints  Vector of points that must be tied to the graph
     *
     * @param tiedPoints  Vector of tied points
     *
     * @note if tiedPoints[i]==QgsPoint(0.0,0.0) then tied failed.
     */
    virtual void makeGraph( QgsGraphBuilderInterface *builder,
                            const QVector< QgsPoint > &additionalPoints,
                            QVector< QgsPoint> &tiedPoints /Out/ );

    void addProperter( QgsArcProperter* prop /Transfer/ ) ;

    /**
     * return Director name
     */
    virtual QString name() const = 0;
};

