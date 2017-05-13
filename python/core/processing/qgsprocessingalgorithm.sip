/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingalgorithm.h                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsProcessingAlgorithm
{
%Docstring
 Abstract base class for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingalgorithm.h"
%End
  public:

    enum Flag
    {
      FlagHideFromToolbox,
      FlagHideFromModeler,
      FlagSupportsBatch,
      FlagDeprecated,
    };
    typedef QFlags<QgsProcessingAlgorithm::Flag> Flags;


    QgsProcessingAlgorithm();
%Docstring
 Constructor for QgsProcessingAlgorithm.
%End

    virtual ~QgsProcessingAlgorithm();


    virtual QString name() const = 0;
%Docstring
 Returns the algorithm name, used for identifying the algorithm. This string
 should be fixed for the algorithm, and must not be localised. The name should
 be unique within each provider. Names should contain lowercase alphanumeric characters
 only and no spaces or other formatting characters.
.. seealso:: displayName()
.. seealso:: group()
.. seealso:: tags()
 :rtype: str
%End

    QString id() const;
%Docstring
 Returns the unique ID for the algorithm, which is a combination of the algorithm
 provider's ID and the algorithms unique name (e.g. "qgis:mergelayers" ).
.. seealso:: name()
.. seealso:: provider()
 :rtype: str
%End

    virtual QString displayName() const = 0;
%Docstring
 Returns the translated algorithm name, which should be used for any user-visible display
 of the algorithm name.
.. seealso:: name()
 :rtype: str
%End

    virtual QStringList tags() const;
%Docstring
 Returns a list of tags which relate to the algorithm, and are used to assist users in searching
 for suitable algorithms. These tags should be localised.
 :rtype: list of str
%End

    virtual QIcon icon() const;
%Docstring
 Returns an icon for the algorithm.
.. seealso:: svgIconPath()
 :rtype: QIcon
%End

    virtual QString svgIconPath() const;
%Docstring
 Returns a path to an SVG version of the algorithm's icon.
.. seealso:: icon()
 :rtype: str
%End

    virtual QString group() const;
%Docstring
 Returns the name of the group this algorithm belongs to. This string
 should be localised.
.. seealso:: tags()
 :rtype: str
%End

    virtual Flags flags() const;
%Docstring
 Returns the flags indicating how and when the algorithm operates and should be exposed to users.
 :rtype: Flags
%End

    QgsProcessingProvider *provider() const;
%Docstring
 Returns the provider to which this algorithm belongs.
 :rtype: QgsProcessingProvider
%End

    void setProvider( QgsProcessingProvider *provider );

  private:
    QgsProcessingAlgorithm( const QgsProcessingAlgorithm &other );
};
QFlags<QgsProcessingAlgorithm::Flag> operator|(QgsProcessingAlgorithm::Flag f1, QFlags<QgsProcessingAlgorithm::Flag> f2);




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingalgorithm.h                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
