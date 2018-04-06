/***************************************************************************
                         qgsmeshlayer.h
                         --------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHLAYER_H
#define QGSMESHLAYER_H

#include "qgis_core.h"
#include <QMap>
#include <QSet>
#include <QSharedPointer>
#include <QList>
#include <QStringList>
#include <QFont>
#include <QMutex>

#include "qgis.h"
#include "qgsmaplayer.h"
#include "qgsrendercontext.h"
#include "qgsmeshdataprovider.h"

class QgsMapLayerRenderer;
class QgsSymbol;

class QgsMeshDataProvider;
class QgsNativeMesh;
class QgsTriangularMesh;
struct QgsMesh;

/**
 * \ingroup core
 *
 * Represents a mesh layer supporting display of data on structured or unstructured meshes
 *
 * The QgsMeshLayer is instantiated by specifying the name of a data provider,
 * such as mdal, and url defining the specific data set to connect to.
 * The vector layer constructor in turn instantiates a QgsMeshDataProvider subclass
 * corresponding to the provider type, and passes it the url. The data provider
 * connects to the data source.
 *
 * The QgsMeshLayer provides a common interface to the different data types. It does not
 * yet support editing transactions.
 *
 * The main data providers supported by QGIS are listed below.
 *
 * \section providers Mesh data providers
 *
 * \subsection memory Memory data providerType (mesh_memory)
 *
 * The memory data provider is used to construct in memory data, for example scratch
 * data. There is no inherent persistent storage of the data. The data source uri is constructed.
 * Data can be populated by setMesh(const QString &vertices, const QString &faces), where
 * vertices and faces is comma separated coordinates and connections for mesh.
 * E.g. to create mesh with one quad and one triangle
 *
 * \code
 *  QString uri(
 *      "1.0, 2.0 \n" \
 *      "2.0, 2.0 \n" \
 *      "3.0, 2.0 \n" \
 *      "2.0, 3.0 \n" \
 *      "1.0, 3.0 \n" \
 *      "---"
 *      "0, 1, 3, 4 \n" \
 *      "1, 2, 3 \n"
 *    );
 *    QgsMeshLayer *scratchLayer = new QgsMeshLayer(uri, "My Scratch layer", "memory_mesh");
 * \endcode
 *
 * \subsection mdal MDAL data provider (mdal)
 *
 * Accesses data using the MDAL drivers (https://github.com/lutraconsulting/MDAL). The url
 * is the MDAL connection string. QGIS must be built with MDAL support to allow this provider.

 * \code
 *     QString uri = "test/land.2dm";
 *     QgsMeshLayer *scratchLayer = new QgsMeshLayer(uri, "My Scratch Layer",  "mdal");
 * \endcode
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshLayer : public QgsMapLayer
{
    Q_OBJECT
  public:

    /**
     * Constructor - creates a mesh layer
     *
     * The QgsMeshLayer is constructed by instantiating a data provider.  The provider
     * interprets the supplied path (url) of the data source to connect to and access the
     * data.
     *
     * \param path  The path or url of the parameter.  Typically this encodes
     *               parameters used by the data provider as url query items.
     * \param baseName The name used to represent the layer in the legend
     * \param providerLib  The name of the data provider, e.g., "mesh_memory", "mdal"
     */
    explicit QgsMeshLayer( const QString &path = QString(), const QString &baseName = QString(), const QString &providerLib = "mesh_memory" );
    //! Dtor
    ~QgsMeshLayer() override;

    //! QgsMeshLayer cannot be copied.
    QgsMeshLayer( const QgsMeshLayer &rhs ) = delete;
    //! QgsMeshLayer cannot be copied.
    QgsMeshLayer &operator=( QgsMeshLayer const &rhs ) = delete;

    //! Return data provider
    QgsMeshDataProvider *dataProvider() override;

    //! Return const data provider
    const QgsMeshDataProvider *dataProvider() const override SIP_SKIP;

    /**
     * Returns a new instance equivalent to this one. A new provider is
     *  created for the same data source and renderers are cloned too.
     * \returns a new layer instance
     */
    QgsMeshLayer *clone() const override SIP_FACTORY;

    //! Returns the extent of the layer.
    QgsRectangle extent() const override;

    /**
     * Return new instance of QgsMapLayerRenderer that will be used for rendering of given context
     */
    virtual QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) SIP_FACTORY;

    //! Return the provider type for this layer
    QString providerType() const;

    //! return native mesh (nullprt before rendering)
    QgsMesh *nativeMesh() SIP_SKIP {return mNativeMesh;}

    //! return triangular mesh (nullprt before rendering)
    QgsTriangularMesh *triangularMesh() SIP_SKIP {return mTriangularMesh;}

    //! Returns a line symbol used for rendering native mesh.
    QgsSymbol *nativeMeshSymbol() {return mNativeMeshSymbol;}

    /**
     * Returns a line symbol used for rendering of triangular (derived) mesh.
     * \see toggleTriangularMeshRendering
     */
    QgsSymbol *triangularMeshSymbol() {return mTriangularMeshSymbol;}

    //! Toggle rendering of triangular (derived) mesh. Off by default
    void toggleTriangularMeshRendering( bool toggle );

    /**
     * Read the symbology for the current layer from the Dom node supplied.
     * \param node node that will contain the symbology definition for this layer.
     * \param errorMessage reference to string that will be updated with any error messages
     * \param context reading context (used for transform from relative to absolute paths)
     * \returns true in case of success.
     */
    bool readSymbology( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context );

    /**
     * Write the symbology for the layer into the docment provided.
     *  \param node the node that will have the style element added to it.
     *  \param doc the document that will have the QDomNode added.
     *  \param errorMessage reference to string that will be updated with any error messages
     *  \param context writing context (used for transform from absolute to relative paths)
     *  \returns true in case of success.
     */
    bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context ) const;

  private:                       // Private methods

    /**
     * Returns true if the provider is in read-only mode
     */
    bool isReadOnly() const override {return true;}

    /**
     * Bind layer to a specific data provider
     * \param provider provider key string, must match a valid QgsMeshDataProvider key. E.g. "mesh_memory", etc.
     */
    bool setDataProvider( QString const &provider );

#ifdef SIP_RUN
    QgsMeshLayer( const QgsMeshLayer &rhs );
#endif

  private:
    //! Clear native and triangular mesh
    void clearMeshes();
    void fillNativeMesh();

  private:
    //! Pointer to native mesh structure, used as cache for rendering
    QgsMesh *mNativeMesh = nullptr;

    //! Pointer to derived mesh structure
    QgsTriangularMesh *mTriangularMesh = nullptr;

    //! Pointer to data provider derived from the abastract base class QgsMeshDataProvider
    QgsMeshDataProvider *mDataProvider = nullptr;

    //! Data provider key
    QString mProviderKey;

    //! rendering native mesh
    QgsSymbol *mNativeMeshSymbol = nullptr;

    //! rendering triangular mesh
    QgsSymbol *mTriangularMeshSymbol = nullptr;
};

#endif //QGSMESHLAYER_H
