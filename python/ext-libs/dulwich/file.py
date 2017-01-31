# file.py -- Safe access to git files
# Copyright (C) 2010 Google, Inc.
#
# Dulwich is dual-licensed under the Apache License, Version 2.0 and the GNU
# General Public License as public by the Free Software Foundation; version 2.0
# or (at your option) any later version. You can redistribute it and/or
# modify it under the terms of either of these two licenses.
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# You should have received a copy of the licenses; if not, see
# <http://www.gnu.org/licenses/> for a copy of the GNU General Public License
# and <http://www.apache.org/licenses/LICENSE-2.0> for a copy of the Apache
# License, Version 2.0.
#

"""Safe access to git files."""

import errno
import io
import os
import sys
import tempfile

def ensure_dir_exists(dirname):
    """Ensure a directory exists, creating if necessary."""
    try:
        os.makedirs(dirname)
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise


def _fancy_rename(oldname, newname):
    """Rename file with temporary backup file to rollback if rename fails"""
    if not os.path.exists(newname):
        try:
            os.rename(oldname, newname)
        except OSError:
            raise
        return

    # destination file exists
    try:
        (fd, tmpfile) = tempfile.mkstemp(".tmp", prefix=oldname+".", dir=".")
        os.close(fd)
        os.remove(tmpfile)
    except OSError:
        # either file could not be created (e.g. permission problem)
        # or could not be deleted (e.g. rude virus scanner)
        raise
    try:
        os.rename(newname, tmpfile)
    except OSError:
        raise   # no rename occurred
    try:
        os.rename(oldname, newname)
    except OSError:
        os.rename(tmpfile, newname)
        raise
    os.remove(tmpfile)


def GitFile(filename, mode='rb', bufsize=-1):
    """Create a file object that obeys the git file locking protocol.

    :return: a builtin file object or a _GitFile object

    :note: See _GitFile for a description of the file locking protocol.

    Only read-only and write-only (binary) modes are supported; r+, w+, and a
    are not.  To read and write from the same file, you can take advantage of
    the fact that opening a file for write does not actually open the file you
    request.
    """
    if 'a' in mode:
        raise IOError('append mode not supported for Git files')
    if '+' in mode:
        raise IOError('read/write mode not supported for Git files')
    if 'b' not in mode:
        raise IOError('text mode not supported for Git files')
    if 'w' in mode:
        return _GitFile(filename, mode, bufsize)
    else:
        return io.open(filename, mode, bufsize)


class _GitFile(object):
    """File that follows the git locking protocol for writes.

    All writes to a file foo will be written into foo.lock in the same
    directory, and the lockfile will be renamed to overwrite the original file
    on close.

    :note: You *must* call close() or abort() on a _GitFile for the lock to be
        released. Typically this will happen in a finally block.
    """

    PROXY_PROPERTIES = set(['closed', 'encoding', 'errors', 'mode', 'name',
                            'newlines', 'softspace'])
    PROXY_METHODS = ('__iter__', 'flush', 'fileno', 'isatty', 'read',
                     'readline', 'readlines', 'seek', 'tell',
                     'truncate', 'write', 'writelines')
    def __init__(self, filename, mode, bufsize):
        self._filename = filename
        self._lockfilename = '%s.lock' % self._filename
        fd = os.open(self._lockfilename,
            os.O_RDWR | os.O_CREAT | os.O_EXCL | getattr(os, "O_BINARY", 0))
        self._file = os.fdopen(fd, mode, bufsize)
        self._closed = False

        for method in self.PROXY_METHODS:
            setattr(self, method, getattr(self._file, method))

    def abort(self):
        """Close and discard the lockfile without overwriting the target.

        If the file is already closed, this is a no-op.
        """
        if self._closed:
            return
        self._file.close()
        try:
            os.remove(self._lockfilename)
            self._closed = True
        except OSError as e:
            # The file may have been removed already, which is ok.
            if e.errno != errno.ENOENT:
                raise
            self._closed = True

    def close(self):
        """Close this file, saving the lockfile over the original.

        :note: If this method fails, it will attempt to delete the lockfile.
            However, it is not guaranteed to do so (e.g. if a filesystem becomes
            suddenly read-only), which will prevent future writes to this file
            until the lockfile is removed manually.
        :raises OSError: if the original file could not be overwritten. The lock
            file is still closed, so further attempts to write to the same file
            object will raise ValueError.
        """
        if self._closed:
            return
        self._file.close()
        try:
            try:
                os.rename(self._lockfilename, self._filename)
            except OSError as e:
                if sys.platform == 'win32' and e.errno == errno.EEXIST:
                    # Windows versions prior to Vista don't support atomic renames
                    _fancy_rename(self._lockfilename, self._filename)
                else:
                    raise
        finally:
            self.abort()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def __getattr__(self, name):
        """Proxy property calls to the underlying file."""
        if name in self.PROXY_PROPERTIES:
            return getattr(self._file, name)
        raise AttributeError(name)
