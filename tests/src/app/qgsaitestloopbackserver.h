/***************************************************************************
  qgsaitestloopbackserver.h
  -------------------------
  begin                : June 2026
***************************************************************************/

#ifndef QGSAITESTLOOPBACKSERVER_H
#define QGSAITESTLOOPBACKSERVER_H

#include <QByteArray>
#include <QList>
#include <QPair>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

/**
 * Loopback HTTP server driven by a queue of scripted responses, for exercising
 * the AI networking stack (model router, embedding client) without a live API.
 *
 * Each incoming request consumes the next queued response (the last one repeats
 * once the queue is exhausted). Responses are either a plain HTTP body or an
 * SSE script whose chunks are written sequentially, optionally delayed, before
 * the connection is closed. Received request bodies are recorded so tests can
 * assert on the exact payload that was sent.
 */
class QgsAiTestLoopbackServer : public QTcpServer
{
  public:
    struct ScriptedResponse
    {
        QByteArray statusLine = QByteArrayLiteral( "HTTP/1.1 200 OK" );
        QList<QPair<QByteArray, QByteArray>> headers;
        QByteArray body;             //!< Plain body; Content-Length is added automatically
        QList<QByteArray> sseChunks; //!< When non-empty, streamed as text/event-stream instead of \a body
        int interChunkDelayMs = 0;
    };

    static ScriptedResponse jsonResponse( int statusCode, const QByteArray &reasonPhrase, const QByteArray &body, const QList<QPair<QByteArray, QByteArray>> &extraHeaders = {} )
    {
      ScriptedResponse response;
      response.statusLine = QByteArrayLiteral( "HTTP/1.1 " ) + QByteArray::number( statusCode ) + ' ' + reasonPhrase;
      response.headers = extraHeaders;
      response.headers.append( { QByteArrayLiteral( "Content-Type" ), QByteArrayLiteral( "application/json" ) } );
      response.body = body;
      return response;
    }

    static ScriptedResponse sseResponse( const QList<QByteArray> &chunks, int interChunkDelayMs = 0 )
    {
      ScriptedResponse response;
      response.headers.append( { QByteArrayLiteral( "Content-Type" ), QByteArrayLiteral( "text/event-stream" ) } );
      response.sseChunks = chunks;
      response.interChunkDelayMs = interChunkDelayMs;
      return response;
    }

    QList<ScriptedResponse> responses;
    int requestCount = 0;
    QList<QByteArray> requestBodies;
    QList<QByteArray> rawRequests; //!< Full request including headers, for asserting on sent headers

    QByteArray lastRequestBody() const { return requestBodies.isEmpty() ? QByteArray() : requestBodies.last(); }
    QByteArray lastRawRequest() const { return rawRequests.isEmpty() ? QByteArray() : rawRequests.last(); }

  protected:
    void incomingConnection( qintptr handle ) override
    {
      QTcpSocket *socket = new QTcpSocket( this );
      socket->setSocketDescriptor( handle );
      connect( socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater );
      connect( socket, &QTcpSocket::readyRead, this, [this, socket]() { onSocketReadyRead( socket ); } );
    }

  private:
    void onSocketReadyRead( QTcpSocket *socket )
    {
      if ( socket->property( "answered" ).toBool() )
      {
        socket->readAll();
        return;
      }

      QByteArray buffered = socket->property( "buffered" ).toByteArray();
      buffered += socket->readAll();
      socket->setProperty( "buffered", buffered );

      const int headersEnd = buffered.indexOf( "\r\n\r\n" );
      if ( headersEnd < 0 )
        return;

      // Wait for the full body before answering, so tests can assert on it.
      int contentLength = 0;
      const QByteArray headerBlock = buffered.left( headersEnd ).toLower();
      const int contentLengthPos = headerBlock.indexOf( "content-length:" );
      if ( contentLengthPos >= 0 )
      {
        const int lineEnd = headerBlock.indexOf( "\r\n", contentLengthPos );
        contentLength = headerBlock.mid( contentLengthPos + 15, ( lineEnd < 0 ? headerBlock.size() : lineEnd ) - contentLengthPos - 15 ).trimmed().toInt();
      }
      const QByteArray bodySoFar = buffered.mid( headersEnd + 4 );
      if ( bodySoFar.size() < contentLength )
        return;

      socket->setProperty( "answered", true );
      ++requestCount;
      requestBodies.append( bodySoFar.left( contentLength ) );
      rawRequests.append( buffered );

      const int responseIndex = qMin( requestCount - 1, static_cast<int>( responses.size() ) - 1 );
      const ScriptedResponse response = responseIndex >= 0 ? responses.at( responseIndex ) : ScriptedResponse();
      writeResponse( socket, response );
    }

    void writeResponse( QTcpSocket *socket, const ScriptedResponse &response )
    {
      QByteArray head = response.statusLine + "\r\n";
      for ( const QPair<QByteArray, QByteArray> &header : response.headers )
        head += header.first + ": " + header.second + "\r\n";

      if ( response.sseChunks.isEmpty() )
      {
        head += "Content-Length: " + QByteArray::number( response.body.size() ) + "\r\n";
        head += "Connection: close\r\n\r\n";
        socket->write( head + response.body );
        socket->flush();
        socket->disconnectFromHost();
        return;
      }

      head += "Connection: close\r\n\r\n";
      socket->write( head );
      socket->flush();
      writeNextChunk( socket, response.sseChunks, 0, response.interChunkDelayMs );
    }

    void writeNextChunk( QTcpSocket *socket, const QList<QByteArray> &chunks, int index, int delayMs )
    {
      QTimer::singleShot( delayMs, socket, [this, socket, chunks, index, delayMs]() {
        if ( socket->state() != QAbstractSocket::ConnectedState )
          return;
        if ( index >= chunks.size() )
        {
          socket->disconnectFromHost();
          return;
        }
        socket->write( chunks.at( index ) );
        socket->flush();
        writeNextChunk( socket, chunks, index + 1, delayMs );
      } );
    }
};

#endif // QGSAITESTLOOPBACKSERVER_H
