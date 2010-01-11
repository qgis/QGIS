<tagfile>
  <compound kind="class">
    <name>QIODevice</name>
    <filename>qiodevice.html</filename>
    <member kind="function">
      <name>QIODevice</name>
      <anchor>QIODevice</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <name>QIODevice</name>
      <anchor>QIODevice-2</anchor>
      <arglist>( QObject * parent )</arglist>
    </member>
    <member kind="function">
      <name>aboutToClose</name>
      <anchor>aboutToClose</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <name>atEnd</name>
      <anchor>atEnd</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <name>bytesAvailable</name>
      <anchor>bytesAvailable</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <name>bytesToWrite</name>
      <anchor>bytesToWrite</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <name>bytesWritten</name>
      <anchor>bytesWritten</anchor>
      <arglist>( qint64 bytes )</arglist>
    </member>
    <member kind="function">
      <name>canReadLine</name>
      <anchor>canReadLine</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <name>close</name>
      <anchor>close</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <name>errorString</name>
      <anchor>errorString</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <name>getChar</name>
      <anchor>getChar</anchor>
      <arglist>( char * c )</arglist>
    </member>
    <member kind="function">
      <name>isOpen</name>
      <anchor>isOpen</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <name>isReadable</name>
      <anchor>isReadable</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <name>isSequential</name>
      <anchor>isSequential</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <name>isTextModeEnabled</name>
      <anchor>isTextModeEnabled</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <name>isWritable</name>
      <anchor>isWritable</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <name>open</name>
      <anchor>open</anchor>
      <arglist>( OpenMode mode )</arglist>
    </member>
    <member kind="function">
      <name>openMode</name>
      <anchor>openMode</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <name>peek</name>
      <anchor>peek</anchor>
      <arglist>( char * data, qint64 maxSize )</arglist>
    </member>
    <member kind="function">
      <name>peek</name>
      <anchor>peek-2</anchor>
      <arglist>( qint64 maxSize )</arglist>
    </member>
    <member kind="function">
      <name>pos</name>
      <anchor>pos</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <name>putChar</name>
      <anchor>putChar</anchor>
      <arglist>( char c )</arglist>
    </member>
    <member kind="function">
      <name>read</name>
      <anchor>read</anchor>
      <arglist>( char * data, qint64 maxSize )</arglist>
    </member>
    <member kind="function">
      <name>read</name>
      <anchor>read-2</anchor>
      <arglist>( qint64 maxSize )</arglist>
    </member>
    <member kind="function">
      <name>readAll</name>
      <anchor>readAll</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <name>readData</name>
      <anchor>readData</anchor>
      <arglist>( char * data, qint64 maxSize )</arglist>
    </member>
    <member kind="function">
      <name>readLine</name>
      <anchor>readLine</anchor>
      <arglist>( char * data, qint64 maxSize )</arglist>
    </member>
    <member kind="function">
      <name>readLine</name>
      <anchor>readLine-2</anchor>
      <arglist>( qint64 maxSize = 0 )</arglist>
    </member>
    <member kind="function">
      <name>readLineData</name>
      <anchor>readLineData</anchor>
      <arglist>( char * data, qint64 maxSize )</arglist>
    </member>
    <member kind="function">
      <name>readyRead</name>
      <anchor>readyRead</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <name>reset</name>
      <anchor>reset</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <name>seek</name>
      <anchor>seek</anchor>
      <arglist>( qint64 pos )</arglist>
    </member>
    <member kind="function">
      <name>setErrorString</name>
      <anchor>setErrorString</anchor>
      <arglist>( const QString &amp; str )</arglist>
    </member>
    <member kind="function">
      <name>setOpenMode</name>
      <anchor>setOpenMode</anchor>
      <arglist>( OpenMode openMode )</arglist>
    </member>
    <member kind="function">
      <name>setTextModeEnabled</name>
      <anchor>setTextModeEnabled</anchor>
      <arglist>( bool enabled )</arglist>
    </member>
    <member kind="function">
      <name>size</name>
      <anchor>size</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <name>ungetChar</name>
      <anchor>ungetChar</anchor>
      <arglist>( char c )</arglist>
    </member>
    <member kind="function">
      <name>waitForBytesWritten</name>
      <anchor>waitForBytesWritten</anchor>
      <arglist>( int msecs )</arglist>
    </member>
    <member kind="function">
      <name>waitForReadyRead</name>
      <anchor>waitForReadyRead</anchor>
      <arglist>( int msecs )</arglist>
    </member>
    <member kind="function">
      <name>write</name>
      <anchor>write</anchor>
      <arglist>( const char * data, qint64 maxSize )</arglist>
    </member>
    <member kind="function">
      <name>write</name>
      <anchor>write-2</anchor>
      <arglist>( const QByteArray &amp; byteArray )</arglist>
    </member>
    <member kind="function">
      <name>writeData</name>
      <anchor>writeData</anchor>
      <arglist>( const char * data, qint64 maxSize )</arglist>
    </member>
  </compound>
</tagfile>
