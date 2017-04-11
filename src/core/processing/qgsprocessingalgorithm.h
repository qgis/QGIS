/***************************************************************************
                         qgsprocessingalgorithm.h
                         ------------------------
    begin                : December 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGALGORITHM_H
#define QGSPROCESSINGALGORITHM_H

#include "qgis_core.h"
#include <QString>
#include <QVariant>
#include <QIcon>

class QgsProcessingProvider;

/**
 * \class QgsProcessingAlgorithm
 * \ingroup core
 * Abstract base class for processing algorithms.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingAlgorithm
{
  public:

    //! Flags indicating how and when an algorithm operates and should be exposed to users
    enum Flag
    {
      FlagHideFromToolbox = 1 << 1, //!< Algorithm should be hidden from the toolbox
      FlagHideFromModeler = 1 << 2, //!< Algorithm should be hidden from the modeler
      FlagSupportsBatch = 1 << 3,  //!< Algorithm supports batch mode
      FlagDeprecated = FlagHideFromToolbox | FlagHideFromModeler, //!< Algorithm is deprecated
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsProcessingAlgorithm.
     */
    QgsProcessingAlgorithm() = default;

    virtual ~QgsProcessingAlgorithm() = default;

    //! Algorithms cannot be copied
    QgsProcessingAlgorithm( const QgsProcessingAlgorithm &other ) = delete;
    //! Algorithms cannot be copied
    QgsProcessingAlgorithm &operator=( const QgsProcessingAlgorithm &other ) = delete;

    /**
     * Returns the algorithm name, used for identifying the algorithm. This string
     * should be fixed for the algorithm, and must not be localised. The name should
     * be unique within each provider. Names should contain lowercase alphanumeric characters
     * only and no spaces or other formatting characters.
     * \see displayName()
     * \see group()
     * \see tags()
    */
    virtual QString name() const = 0;

    /**
     * Returns the unique ID for the algorithm, which is a combination of the algorithm
     * provider's ID and the algorithms unique name (e.g. "qgis:mergelayers" ).
     * \see name()
     * \see provider()
     */
    QString id() const;

    /**
     * Returns the translated algorithm name, which should be used for any user-visible display
     * of the algorithm name.
     * \see name()
     */
    virtual QString displayName() const = 0;

    /**
     * Returns a list of tags which relate to the algorithm, and are used to assist users in searching
     * for suitable algorithms. These tags should be localised.
    */
    virtual QStringList tags() const { return QStringList(); }

    /**
     * Returns an icon for the algorithm.
     * \see svgIconPath()
    */
    virtual QIcon icon() const;

    /**
     * Returns a path to an SVG version of the algorithm's icon.
     * \see icon()
     */
    virtual QString svgIconPath() const;

    /**
     * Returns the name of the group this algorithm belongs to. This string
     * should be localised.
     * \see tags()
    */
    virtual QString group() const { return QString(); }

    /**
     * Returns the flags indicating how and when the algorithm operates and should be exposed to users.
     */
    virtual Flags flags() const;

    /**
     * Returns the provider to which this algorithm belongs.
     */
    QgsProcessingProvider *provider() const;

  private:

    /**
     * Associates this algorithm with its provider. No transfer of ownership is involved.
     */
    void setProvider( QgsProcessingProvider *provider );

    QgsProcessingProvider *mProvider = nullptr;

    // friend class to access setProvider() - we do not want this public!
    friend class QgsProcessingProvider;
    friend class TestQgsProcessing;

#ifdef SIP_RUN
    QgsProcessingAlgorithm( const QgsProcessingAlgorithm &other );
#endif

};
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsProcessingAlgorithm::Flags )

#endif // QGSPROCESSINGALGORITHM_H


