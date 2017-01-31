# client.py -- Implementation of the client side git protocols
# Copyright (C) 2008-2013 Jelmer Vernooij <jelmer@samba.org>
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

"""Client side support for the Git protocol.

The Dulwich client supports the following capabilities:

 * thin-pack
 * multi_ack_detailed
 * multi_ack
 * side-band-64k
 * ofs-delta
 * quiet
 * report-status
 * delete-refs

Known capabilities that are not supported:

 * shallow
 * no-progress
 * include-tag
"""

__docformat__ = 'restructuredText'

from contextlib import closing
from io import BytesIO, BufferedReader
import dulwich
import select
import socket
import subprocess
import sys

try:
    from urllib import quote as urlquote
    from urllib import unquote as urlunquote
except ImportError:
    from urllib.parse import quote as urlquote
    from urllib.parse import unquote as urlunquote

try:
    import urllib2
    import urlparse
except ImportError:
    import urllib.request as urllib2
    import urllib.parse as urlparse

from dulwich.errors import (
    GitProtocolError,
    NotGitRepository,
    SendPackError,
    UpdateRefsError,
    )
from dulwich.protocol import (
    _RBUFSIZE,
    capability_agent,
    CAPABILITY_DELETE_REFS,
    CAPABILITY_MULTI_ACK,
    CAPABILITY_MULTI_ACK_DETAILED,
    CAPABILITY_OFS_DELTA,
    CAPABILITY_QUIET,
    CAPABILITY_REPORT_STATUS,
    CAPABILITY_SIDE_BAND_64K,
    CAPABILITY_THIN_PACK,
    CAPABILITIES_REF,
    COMMAND_DONE,
    COMMAND_HAVE,
    COMMAND_WANT,
    SIDE_BAND_CHANNEL_DATA,
    SIDE_BAND_CHANNEL_PROGRESS,
    SIDE_BAND_CHANNEL_FATAL,
    PktLineParser,
    Protocol,
    ProtocolFile,
    TCP_GIT_PORT,
    ZERO_SHA,
    extract_capabilities,
    )
from dulwich.pack import (
    write_pack_objects,
    )
from dulwich.refs import (
    read_info_refs,
    )


def _fileno_can_read(fileno):
    """Check if a file descriptor is readable."""
    return len(select.select([fileno], [], [], 0)[0]) > 0


COMMON_CAPABILITIES = [CAPABILITY_OFS_DELTA, CAPABILITY_SIDE_BAND_64K]
FETCH_CAPABILITIES = ([CAPABILITY_THIN_PACK, CAPABILITY_MULTI_ACK,
                       CAPABILITY_MULTI_ACK_DETAILED] +
                      COMMON_CAPABILITIES)
SEND_CAPABILITIES = [CAPABILITY_REPORT_STATUS] + COMMON_CAPABILITIES


class ReportStatusParser(object):
    """Handle status as reported by servers with 'report-status' capability.
    """

    def __init__(self):
        self._done = False
        self._pack_status = None
        self._ref_status_ok = True
        self._ref_statuses = []

    def check(self):
        """Check if there were any errors and, if so, raise exceptions.

        :raise SendPackError: Raised when the server could not unpack
        :raise UpdateRefsError: Raised when refs could not be updated
        """
        if self._pack_status not in (b'unpack ok', None):
            raise SendPackError(self._pack_status)
        if not self._ref_status_ok:
            ref_status = {}
            ok = set()
            for status in self._ref_statuses:
                if b' ' not in status:
                    # malformed response, move on to the next one
                    continue
                status, ref = status.split(b' ', 1)

                if status == b'ng':
                    if b' ' in ref:
                        ref, status = ref.split(b' ', 1)
                else:
                    ok.add(ref)
                ref_status[ref] = status
            # TODO(jelmer): don't assume encoding of refs is ascii.
            raise UpdateRefsError(', '.join([
                ref.decode('ascii') for ref in ref_status if ref not in ok]) +
                ' failed to update', ref_status=ref_status)

    def handle_packet(self, pkt):
        """Handle a packet.

        :raise GitProtocolError: Raised when packets are received after a
            flush packet.
        """
        if self._done:
            raise GitProtocolError("received more data after status report")
        if pkt is None:
            self._done = True
            return
        if self._pack_status is None:
            self._pack_status = pkt.strip()
        else:
            ref_status = pkt.strip()
            self._ref_statuses.append(ref_status)
            if not ref_status.startswith(b'ok '):
                self._ref_status_ok = False


def read_pkt_refs(proto):
    server_capabilities = None
    refs = {}
    # Receive refs from server
    for pkt in proto.read_pkt_seq():
        (sha, ref) = pkt.rstrip(b'\n').split(None, 1)
        if sha == b'ERR':
            raise GitProtocolError(ref)
        if server_capabilities is None:
            (ref, server_capabilities) = extract_capabilities(ref)
        refs[ref] = sha

    if len(refs) == 0:
        return None, set([])
    if refs == {CAPABILITIES_REF: ZERO_SHA}:
        refs = {}
    return refs, set(server_capabilities)


# TODO(durin42): this doesn't correctly degrade if the server doesn't
# support some capabilities. This should work properly with servers
# that don't support multi_ack.
class GitClient(object):
    """Git smart server client.

    """

    def __init__(self, thin_packs=True, report_activity=None, quiet=False):
        """Create a new GitClient instance.

        :param thin_packs: Whether or not thin packs should be retrieved
        :param report_activity: Optional callback for reporting transport
            activity.
        """
        self._report_activity = report_activity
        self._report_status_parser = None
        self._fetch_capabilities = set(FETCH_CAPABILITIES)
        self._fetch_capabilities.add(capability_agent())
        self._send_capabilities = set(SEND_CAPABILITIES)
        self._send_capabilities.add(capability_agent())
        if quiet:
            self._send_capabilities.add(CAPABILITY_QUIET)
        if not thin_packs:
            self._fetch_capabilities.remove(CAPABILITY_THIN_PACK)

    def get_url(self, path):
        """Retrieves full url to given path.

        :param path: Repository path (as string)
        :return: Url to path (as string)
        """
        raise NotImplementedError(self.get_url)

    @classmethod
    def from_parsedurl(cls, parsedurl, **kwargs):
        """Create an instance of this client from a urlparse.parsed object.

        :param parsedurl: Result of urlparse.urlparse()
        :return: A `GitClient` object
        """
        raise NotImplementedError(cls.from_parsedurl)

    def send_pack(self, path, determine_wants, generate_pack_contents,
                  progress=None, write_pack=write_pack_objects):
        """Upload a pack to a remote repository.

        :param path: Repository path (as bytestring)
        :param generate_pack_contents: Function that can return a sequence of
            the shas of the objects to upload.
        :param progress: Optional progress function
        :param write_pack: Function called with (file, iterable of objects) to
            write the objects returned by generate_pack_contents to the server.

        :raises SendPackError: if server rejects the pack data
        :raises UpdateRefsError: if the server supports report-status
                                 and rejects ref updates
        :return: new_refs dictionary containing the changes that were made
            {refname: new_ref}, including deleted refs.
        """
        raise NotImplementedError(self.send_pack)

    def fetch(self, path, target, determine_wants=None, progress=None):
        """Fetch into a target repository.

        :param path: Path to fetch from (as bytestring)
        :param target: Target repository to fetch into
        :param determine_wants: Optional function to determine what refs
            to fetch
        :param progress: Optional progress function
        :return: Dictionary with all remote refs (not just those fetched)
        """
        if determine_wants is None:
            determine_wants = target.object_store.determine_wants_all
        if CAPABILITY_THIN_PACK in self._fetch_capabilities:
            # TODO(jelmer): Avoid reading entire file into memory and
            # only processing it after the whole file has been fetched.
            f = BytesIO()
            def commit():
                if f.tell():
                    f.seek(0)
                    target.object_store.add_thin_pack(f.read, None)
            def abort():
                pass
        else:
            f, commit, abort = target.object_store.add_pack()
        try:
            result = self.fetch_pack(
                path, determine_wants, target.get_graph_walker(), f.write,
                progress)
        except:
            abort()
            raise
        else:
            commit()
        return result

    def fetch_pack(self, path, determine_wants, graph_walker, pack_data,
                   progress=None):
        """Retrieve a pack from a git smart server.

        :param determine_wants: Callback that returns list of commits to fetch
        :param graph_walker: Object with next() and ack().
        :param pack_data: Callback called for each bit of data in the pack
        :param progress: Callback for progress reports (strings)
        :return: Dictionary with all remote refs (not just those fetched)
        """
        raise NotImplementedError(self.fetch_pack)

    def get_refs(self, path):
        """Retrieve the current refs from a git smart server.

        :param path: Path to the repo to fetch from. (as bytestring)
        """
        raise NotImplementedError(self.get_refs)

    def _parse_status_report(self, proto):
        unpack = proto.read_pkt_line().strip()
        if unpack != b'unpack ok':
            st = True
            # flush remaining error data
            while st is not None:
                st = proto.read_pkt_line()
            raise SendPackError(unpack)
        statuses = []
        errs = False
        ref_status = proto.read_pkt_line()
        while ref_status:
            ref_status = ref_status.strip()
            statuses.append(ref_status)
            if not ref_status.startswith(b'ok '):
                errs = True
            ref_status = proto.read_pkt_line()

        if errs:
            ref_status = {}
            ok = set()
            for status in statuses:
                if b' ' not in status:
                    # malformed response, move on to the next one
                    continue
                status, ref = status.split(b' ', 1)

                if status == b'ng':
                    if b' ' in ref:
                        ref, status = ref.split(b' ', 1)
                else:
                    ok.add(ref)
                ref_status[ref] = status
            raise UpdateRefsError(', '.join([ref for ref in ref_status
                                             if ref not in ok]) +
                                             b' failed to update',
                                  ref_status=ref_status)

    def _read_side_band64k_data(self, proto, channel_callbacks):
        """Read per-channel data.

        This requires the side-band-64k capability.

        :param proto: Protocol object to read from
        :param channel_callbacks: Dictionary mapping channels to packet
            handlers to use. None for a callback discards channel data.
        """
        for pkt in proto.read_pkt_seq():
            channel = ord(pkt[:1])
            pkt = pkt[1:]
            try:
                cb = channel_callbacks[channel]
            except KeyError:
                raise AssertionError('Invalid sideband channel %d' % channel)
            else:
                if cb is not None:
                    cb(pkt)

    def _handle_receive_pack_head(self, proto, capabilities, old_refs,
                                  new_refs):
        """Handle the head of a 'git-receive-pack' request.

        :param proto: Protocol object to read from
        :param capabilities: List of negotiated capabilities
        :param old_refs: Old refs, as received from the server
        :param new_refs: Refs to change
        :return: (have, want) tuple
        """
        want = []
        have = [x for x in old_refs.values() if not x == ZERO_SHA]
        sent_capabilities = False

        for refname in new_refs:
            if not isinstance(refname, bytes):
                raise TypeError('refname is not a bytestring: %r' % refname)
            old_sha1 = old_refs.get(refname, ZERO_SHA)
            if not isinstance(old_sha1, bytes):
                raise TypeError('old sha1 for %s is not a bytestring: %r' %
                        (refname, old_sha1))
            new_sha1 = new_refs.get(refname, ZERO_SHA)
            if not isinstance(new_sha1, bytes):
                raise TypeError('old sha1 for %s is not a bytestring %r' %
                        (refname, new_sha1))

            if old_sha1 != new_sha1:
                if sent_capabilities:
                    proto.write_pkt_line(old_sha1 + b' ' + new_sha1 + b' ' + refname)
                else:
                    proto.write_pkt_line(
                        old_sha1 + b' ' + new_sha1 + b' ' + refname + b'\0' +
                        b' '.join(capabilities))
                    sent_capabilities = True
            if new_sha1 not in have and new_sha1 != ZERO_SHA:
                want.append(new_sha1)
        proto.write_pkt_line(None)
        return (have, want)

    def _handle_receive_pack_tail(self, proto, capabilities, progress=None):
        """Handle the tail of a 'git-receive-pack' request.

        :param proto: Protocol object to read from
        :param capabilities: List of negotiated capabilities
        :param progress: Optional progress reporting function
        """
        if b"side-band-64k" in capabilities:
            if progress is None:
                progress = lambda x: None
            channel_callbacks = {2: progress}
            if CAPABILITY_REPORT_STATUS in capabilities:
                channel_callbacks[1] = PktLineParser(
                    self._report_status_parser.handle_packet).parse
            self._read_side_band64k_data(proto, channel_callbacks)
        else:
            if CAPABILITY_REPORT_STATUS in capabilities:
                for pkt in proto.read_pkt_seq():
                    self._report_status_parser.handle_packet(pkt)
        if self._report_status_parser is not None:
            self._report_status_parser.check()

    def _handle_upload_pack_head(self, proto, capabilities, graph_walker,
                                 wants, can_read):
        """Handle the head of a 'git-upload-pack' request.

        :param proto: Protocol object to read from
        :param capabilities: List of negotiated capabilities
        :param graph_walker: GraphWalker instance to call .ack() on
        :param wants: List of commits to fetch
        :param can_read: function that returns a boolean that indicates
            whether there is extra graph data to read on proto
        """
        assert isinstance(wants, list) and isinstance(wants[0], bytes)
        proto.write_pkt_line(COMMAND_WANT + b' ' + wants[0] + b' ' + b' '.join(capabilities) + b'\n')
        for want in wants[1:]:
            proto.write_pkt_line(COMMAND_WANT + b' ' + want + b'\n')
        proto.write_pkt_line(None)
        have = next(graph_walker)
        while have:
            proto.write_pkt_line(COMMAND_HAVE + b' ' + have + b'\n')
            if can_read():
                pkt = proto.read_pkt_line()
                parts = pkt.rstrip(b'\n').split(b' ')
                if parts[0] == b'ACK':
                    graph_walker.ack(parts[1])
                    if parts[2] in (b'continue', b'common'):
                        pass
                    elif parts[2] == b'ready':
                        break
                    else:
                        raise AssertionError(
                            "%s not in ('continue', 'ready', 'common)" %
                            parts[2])
            have = next(graph_walker)
        proto.write_pkt_line(COMMAND_DONE + b'\n')

    def _handle_upload_pack_tail(self, proto, capabilities, graph_walker,
                                 pack_data, progress=None, rbufsize=_RBUFSIZE):
        """Handle the tail of a 'git-upload-pack' request.

        :param proto: Protocol object to read from
        :param capabilities: List of negotiated capabilities
        :param graph_walker: GraphWalker instance to call .ack() on
        :param pack_data: Function to call with pack data
        :param progress: Optional progress reporting function
        :param rbufsize: Read buffer size
        """
        pkt = proto.read_pkt_line()
        while pkt:
            parts = pkt.rstrip(b'\n').split(b' ')
            if parts[0] == b'ACK':
                graph_walker.ack(parts[1])
            if len(parts) < 3 or parts[2] not in (
                    b'ready', b'continue', b'common'):
                break
            pkt = proto.read_pkt_line()
        if CAPABILITY_SIDE_BAND_64K in capabilities:
            if progress is None:
                # Just ignore progress data
                progress = lambda x: None
            self._read_side_band64k_data(proto, {
                SIDE_BAND_CHANNEL_DATA: pack_data,
                SIDE_BAND_CHANNEL_PROGRESS: progress}
            )
        else:
            while True:
                data = proto.read(rbufsize)
                if data == b"":
                    break
                pack_data(data)


class TraditionalGitClient(GitClient):
    """Traditional Git client."""

    DEFAULT_ENCODING = 'utf-8'

    def __init__(self, path_encoding=DEFAULT_ENCODING, **kwargs):
        self._remote_path_encoding = path_encoding
        super(TraditionalGitClient, self).__init__(**kwargs)

    def _connect(self, cmd, path):
        """Create a connection to the server.

        This method is abstract - concrete implementations should
        implement their own variant which connects to the server and
        returns an initialized Protocol object with the service ready
        for use and a can_read function which may be used to see if
        reads would block.

        :param cmd: The git service name to which we should connect.
        :param path: The path we should pass to the service. (as bytestirng)
        """
        raise NotImplementedError()

    def send_pack(self, path, determine_wants, generate_pack_contents,
                  progress=None, write_pack=write_pack_objects):
        """Upload a pack to a remote repository.

        :param path: Repository path (as bytestring)
        :param generate_pack_contents: Function that can return a sequence of
            the shas of the objects to upload.
        :param progress: Optional callback called with progress updates
        :param write_pack: Function called with (file, iterable of objects) to
            write the objects returned by generate_pack_contents to the server.

        :raises SendPackError: if server rejects the pack data
        :raises UpdateRefsError: if the server supports report-status
                                 and rejects ref updates
        :return: new_refs dictionary containing the changes that were made
            {refname: new_ref}, including deleted refs.
        """
        proto, unused_can_read = self._connect(b'receive-pack', path)
        with proto:
            old_refs, server_capabilities = read_pkt_refs(proto)
            negotiated_capabilities = self._send_capabilities & server_capabilities

            if CAPABILITY_REPORT_STATUS in negotiated_capabilities:
                self._report_status_parser = ReportStatusParser()
            report_status_parser = self._report_status_parser

            try:
                new_refs = orig_new_refs = determine_wants(dict(old_refs))
            except:
                proto.write_pkt_line(None)
                raise

            if not CAPABILITY_DELETE_REFS in server_capabilities:
                # Server does not support deletions. Fail later.
                new_refs = dict(orig_new_refs)
                for ref, sha in orig_new_refs.items():
                    if sha == ZERO_SHA:
                        if CAPABILITY_REPORT_STATUS in negotiated_capabilities:
                            report_status_parser._ref_statuses.append(
                                b'ng ' + sha + b' remote does not support deleting refs')
                            report_status_parser._ref_status_ok = False
                        del new_refs[ref]

            if new_refs is None:
                proto.write_pkt_line(None)
                return old_refs

            if len(new_refs) == 0 and len(orig_new_refs):
                # NOOP - Original new refs filtered out by policy
                proto.write_pkt_line(None)
                if report_status_parser is not None:
                    report_status_parser.check()
                return old_refs

            (have, want) = self._handle_receive_pack_head(
                proto, negotiated_capabilities, old_refs, new_refs)
            if not want and set(new_refs.items()).issubset(set(old_refs.items())):
                return new_refs
            objects = generate_pack_contents(have, want)

            dowrite = len(objects) > 0
            dowrite = dowrite or any(old_refs.get(ref) != sha
                                     for (ref, sha) in new_refs.items()
                                     if sha != ZERO_SHA)
            if dowrite:
                write_pack(proto.write_file(), objects)

            self._handle_receive_pack_tail(
                proto, negotiated_capabilities, progress)
            return new_refs

    def fetch_pack(self, path, determine_wants, graph_walker, pack_data,
                   progress=None):
        """Retrieve a pack from a git smart server.

        :param determine_wants: Callback that returns list of commits to fetch
        :param graph_walker: Object with next() and ack().
        :param pack_data: Callback called for each bit of data in the pack
        :param progress: Callback for progress reports (strings)
        :return: Dictionary with all remote refs (not just those fetched)
        """
        proto, can_read = self._connect(b'upload-pack', path)
        with proto:
            refs, server_capabilities = read_pkt_refs(proto)
            negotiated_capabilities = (
                self._fetch_capabilities & server_capabilities)

            if refs is None:
                proto.write_pkt_line(None)
                return refs

            try:
                wants = determine_wants(refs)
            except:
                proto.write_pkt_line(None)
                raise
            if wants is not None:
                wants = [cid for cid in wants if cid != ZERO_SHA]
            if not wants:
                proto.write_pkt_line(None)
                return refs
            self._handle_upload_pack_head(
                proto, negotiated_capabilities, graph_walker, wants, can_read)
            self._handle_upload_pack_tail(
                proto, negotiated_capabilities, graph_walker, pack_data, progress)
            return refs

    def get_refs(self, path):
        """Retrieve the current refs from a git smart server."""
        # stock `git ls-remote` uses upload-pack
        proto, _ = self._connect(b'upload-pack', path)
        with proto:
            refs, _ = read_pkt_refs(proto)
            return refs

    def archive(self, path, committish, write_data, progress=None,
                write_error=None):
        proto, can_read = self._connect(b'upload-archive', path)
        with proto:
            proto.write_pkt_line(b"argument " + committish)
            proto.write_pkt_line(None)
            pkt = proto.read_pkt_line()
            if pkt == b"NACK\n":
                return
            elif pkt == b"ACK\n":
                pass
            elif pkt.startswith(b"ERR "):
                raise GitProtocolError(pkt[4:].rstrip(b"\n"))
            else:
                raise AssertionError("invalid response %r" % pkt)
            ret = proto.read_pkt_line()
            if ret is not None:
                raise AssertionError("expected pkt tail")
            self._read_side_band64k_data(proto, {
                SIDE_BAND_CHANNEL_DATA: write_data,
                SIDE_BAND_CHANNEL_PROGRESS: progress,
                SIDE_BAND_CHANNEL_FATAL: write_error})


class TCPGitClient(TraditionalGitClient):
    """A Git Client that works over TCP directly (i.e. git://)."""

    def __init__(self, host, port=None, **kwargs):
        if port is None:
            port = TCP_GIT_PORT
        self._host = host
        self._port = port
        super(TCPGitClient, self).__init__(**kwargs)

    @classmethod
    def from_parsedurl(cls, parsedurl, **kwargs):
        return cls(parsedurl.hostname, port=parsedurl.port, **kwargs)

    def get_url(self, path):
        netloc = self._host
        if self._port is not None and self._port != TCP_GIT_PORT:
            netloc += ":%d" % self._port
        return urlparse.urlunsplit(("git", netloc, path, '', ''))

    def _connect(self, cmd, path):
        if type(cmd) is not bytes:
            raise TypeError(cmd)
        if type(path) is not bytes:
            path = path.encode(self._remote_path_encoding)
        sockaddrs = socket.getaddrinfo(
            self._host, self._port, socket.AF_UNSPEC, socket.SOCK_STREAM)
        s = None
        err = socket.error("no address found for %s" % self._host)
        for (family, socktype, proto, canonname, sockaddr) in sockaddrs:
            s = socket.socket(family, socktype, proto)
            s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            try:
                s.connect(sockaddr)
                break
            except socket.error as err:
                if s is not None:
                    s.close()
                s = None
        if s is None:
            raise err
        # -1 means system default buffering
        rfile = s.makefile('rb', -1)
        # 0 means unbuffered
        wfile = s.makefile('wb', 0)
        def close():
            rfile.close()
            wfile.close()
            s.close()

        proto = Protocol(rfile.read, wfile.write, close,
                         report_activity=self._report_activity)
        if path.startswith(b"/~"):
            path = path[1:]
        # TODO(jelmer): Alternative to ascii?
        proto.send_cmd(b'git-' + cmd, path, b'host=' + self._host.encode('ascii'))
        return proto, lambda: _fileno_can_read(s)


class SubprocessWrapper(object):
    """A socket-like object that talks to a subprocess via pipes."""

    def __init__(self, proc):
        self.proc = proc
        if sys.version_info[0] == 2:
            self.read = proc.stdout.read
        else:
            self.read = BufferedReader(proc.stdout).read
        self.write = proc.stdin.write

    def can_read(self):
        if sys.platform == 'win32':
            from msvcrt import get_osfhandle
            from win32pipe import PeekNamedPipe
            handle = get_osfhandle(self.proc.stdout.fileno())
            data, total_bytes_avail, msg_bytes_left = PeekNamedPipe(handle, 0)
            return total_bytes_avail != 0
        else:
            return _fileno_can_read(self.proc.stdout.fileno())

    def close(self):
        self.proc.stdin.close()
        self.proc.stdout.close()
        if self.proc.stderr:
            self.proc.stderr.close()
        self.proc.wait()


def find_git_command():
    """Find command to run for system Git (usually C Git).
    """
    if sys.platform == 'win32': # support .exe, .bat and .cmd
        try: # to avoid overhead
            import win32api
        except ImportError: # run through cmd.exe with some overhead
            return ['cmd', '/c', 'git']
        else:
            status, git = win32api.FindExecutable('git')
            return [git]
    else:
        return ['git']


class SubprocessGitClient(TraditionalGitClient):
    """Git client that talks to a server using a subprocess."""

    def __init__(self, **kwargs):
        self._connection = None
        self._stderr = None
        self._stderr = kwargs.get('stderr')
        if 'stderr' in kwargs:
            del kwargs['stderr']
        super(SubprocessGitClient, self).__init__(**kwargs)

    @classmethod
    def from_parsedurl(cls, parsedurl, **kwargs):
        return cls(**kwargs)

    git_command = None

    def _connect(self, service, path):
        if type(service) is not bytes:
            raise TypeError(service)
        if type(path) is not bytes:
            path = path.encode(self._remote_path_encoding)
        if self.git_command is None:
            git_command = find_git_command()
        argv = git_command + [service.decode('ascii'), path]
        p = SubprocessWrapper(
            subprocess.Popen(argv, bufsize=0, stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             stderr=self._stderr))
        return Protocol(p.read, p.write, p.close,
                        report_activity=self._report_activity), p.can_read


class LocalGitClient(GitClient):
    """Git Client that just uses a local Repo."""

    def __init__(self, thin_packs=True, report_activity=None):
        """Create a new LocalGitClient instance.

        :param thin_packs: Whether or not thin packs should be retrieved
        :param report_activity: Optional callback for reporting transport
            activity.
        """
        self._report_activity = report_activity
        # Ignore the thin_packs argument

    def get_url(self, path):
        return urlparse.urlunsplit(('file', '', path, '', ''))

    @classmethod
    def from_parsedurl(cls, parsedurl, **kwargs):
        return cls(**kwargs)

    @classmethod
    def _open_repo(cls, path):
        from dulwich.repo import Repo
        if not isinstance(path, str):
            path = path.decode(sys.getfilesystemencoding())
        return closing(Repo(path))

    def send_pack(self, path, determine_wants, generate_pack_contents,
                  progress=None, write_pack=write_pack_objects):
        """Upload a pack to a remote repository.

        :param path: Repository path (as bytestring)
        :param generate_pack_contents: Function that can return a sequence of
            the shas of the objects to upload.
        :param progress: Optional progress function
        :param write_pack: Function called with (file, iterable of objects) to
            write the objects returned by generate_pack_contents to the server.

        :raises SendPackError: if server rejects the pack data
        :raises UpdateRefsError: if the server supports report-status
                                 and rejects ref updates
        :return: new_refs dictionary containing the changes that were made
            {refname: new_ref}, including deleted refs.
        """
        if not progress:
            progress = lambda x: None

        with self._open_repo(path)  as target:
            old_refs = target.get_refs()
            new_refs = determine_wants(dict(old_refs))

            have = [sha1 for sha1 in old_refs.values() if sha1 != ZERO_SHA]
            want = []
            for refname, new_sha1 in new_refs.items():
                if new_sha1 not in have and not new_sha1 in want and new_sha1 != ZERO_SHA:
                    want.append(new_sha1)

            if not want and set(new_refs.items()).issubset(set(old_refs.items())):
                return new_refs

            target.object_store.add_objects(generate_pack_contents(have, want))

            for refname, new_sha1 in new_refs.items():
                old_sha1 = old_refs.get(refname, ZERO_SHA)
                if new_sha1 != ZERO_SHA:
                    if not target.refs.set_if_equals(refname, old_sha1, new_sha1):
                        progress('unable to set %s to %s' % (refname, new_sha1))
                else:
                    if not target.refs.remove_if_equals(refname, old_sha1):
                        progress('unable to remove %s' % refname)

        return new_refs

    def fetch(self, path, target, determine_wants=None, progress=None):
        """Fetch into a target repository.

        :param path: Path to fetch from (as bytestring)
        :param target: Target repository to fetch into
        :param determine_wants: Optional function to determine what refs
            to fetch
        :param progress: Optional progress function
        :return: Dictionary with all remote refs (not just those fetched)
        """
        with self._open_repo(path) as r:
            return r.fetch(target, determine_wants=determine_wants,
                           progress=progress)

    def fetch_pack(self, path, determine_wants, graph_walker, pack_data,
                   progress=None):
        """Retrieve a pack from a git smart server.

        :param determine_wants: Callback that returns list of commits to fetch
        :param graph_walker: Object with next() and ack().
        :param pack_data: Callback called for each bit of data in the pack
        :param progress: Callback for progress reports (strings)
        :return: Dictionary with all remote refs (not just those fetched)
        """
        with self._open_repo(path) as r:
            objects_iter = r.fetch_objects(determine_wants, graph_walker, progress)

            # Did the process short-circuit (e.g. in a stateless RPC call)? Note
            # that the client still expects a 0-object pack in most cases.
            if objects_iter is None:
                return
            write_pack_objects(ProtocolFile(None, pack_data), objects_iter)
            return r.get_refs()

    def get_refs(self, path):
        """Retrieve the current refs from a git smart server."""

        with self._open_repo(path) as target:
            return target.get_refs()


# What Git client to use for local access
default_local_git_client_cls = LocalGitClient


class SSHVendor(object):
    """A client side SSH implementation."""

    def connect_ssh(self, host, command, username=None, port=None):
        # This function was deprecated in 0.9.1
        import warnings
        warnings.warn(
            "SSHVendor.connect_ssh has been renamed to SSHVendor.run_command",
            DeprecationWarning)
        return self.run_command(host, command, username=username, port=port)

    def run_command(self, host, command, username=None, port=None):
        """Connect to an SSH server.

        Run a command remotely and return a file-like object for interaction
        with the remote command.

        :param host: Host name
        :param command: Command to run (as argv array)
        :param username: Optional ame of user to log in as
        :param port: Optional SSH port to use
        """
        raise NotImplementedError(self.run_command)


class SubprocessSSHVendor(SSHVendor):
    """SSH vendor that shells out to the local 'ssh' command."""

    def run_command(self, host, command, username=None, port=None):
        if not isinstance(command, bytes):
            raise TypeError(command)

        #FIXME: This has no way to deal with passwords..
        args = ['ssh', '-x']
        if port is not None:
            args.extend(['-p', str(port)])
        if username is not None:
            host = '%s@%s' % (username, host)
        args.append(host)
        proc = subprocess.Popen(args + [command],
                                stdin=subprocess.PIPE,
                                stdout=subprocess.PIPE)
        return SubprocessWrapper(proc)


def ParamikoSSHVendor(**kwargs):
    import warnings
    warnings.warn(
        "ParamikoSSHVendor has been moved to dulwich.contrib.paramiko_vendor.",
        DeprecationWarning)
    from dulwich.contrib.paramiko_vendor import ParamikoSSHVendor
    return ParamikoSSHVendor(**kwargs)


# Can be overridden by users
get_ssh_vendor = SubprocessSSHVendor


class SSHGitClient(TraditionalGitClient):

    def __init__(self, host, port=None, username=None, vendor=None, **kwargs):
        self.host = host
        self.port = port
        self.username = username
        super(SSHGitClient, self).__init__(**kwargs)
        self.alternative_paths = {}
        if vendor is not None:
            self.ssh_vendor = vendor
        else:
            self.ssh_vendor = get_ssh_vendor()

    def get_url(self, path):
        netloc = self.host
        if self.port is not None:
            netloc += ":%d" % self.port

        if self.username is not None:
            netloc = urlquote(self.username, '@/:') + "@" + netloc

        return urlparse.urlunsplit(('ssh', netloc, path, '', ''))

    @classmethod
    def from_parsedurl(cls, parsedurl, **kwargs):
        return cls(host=parsedurl.hostname, port=parsedurl.port,
                   username=parsedurl.username, **kwargs)

    def _get_cmd_path(self, cmd):
        cmd = self.alternative_paths.get(cmd, b'git-' + cmd)
        assert isinstance(cmd, bytes)
        return cmd

    def _connect(self, cmd, path):
        if type(cmd) is not bytes:
            raise TypeError(cmd)
        if type(path) is not bytes:
            path = path.encode(self._remote_path_encoding)
        if path.startswith(b"/~"):
            path = path[1:]
        argv = self._get_cmd_path(cmd) + b" '" + path + b"'"
        con = self.ssh_vendor.run_command(
            self.host, argv, port=self.port, username=self.username)
        return (Protocol(con.read, con.write, con.close,
                         report_activity=self._report_activity),
                con.can_read)


def default_user_agent_string():
    return "dulwich/%s" % ".".join([str(x) for x in dulwich.__version__])


def default_urllib2_opener(config):
    if config is not None:
        proxy_server = config.get("http", "proxy")
    else:
        proxy_server = None
    handlers = []
    if proxy_server is not None:
        handlers.append(urllib2.ProxyHandler({"http": proxy_server}))
    opener = urllib2.build_opener(*handlers)
    if config is not None:
        user_agent = config.get("http", "useragent")
    else:
        user_agent = None
    if user_agent is None:
        user_agent = default_user_agent_string()
    opener.addheaders = [('User-agent', user_agent)]
    return opener


class HttpGitClient(GitClient):

    def __init__(self, base_url, dumb=None, opener=None, config=None,
                 username=None, password=None, **kwargs):
        self._base_url = base_url.rstrip("/") + "/"
        self._username = username
        self._password = password
        self.dumb = dumb
        if opener is None:
            self.opener = default_urllib2_opener(config)
        else:
            self.opener = opener
        if username is not None:
            pass_man = urllib2.HTTPPasswordMgrWithDefaultRealm()
            pass_man.add_password(None, base_url, username, password)
            self.opener.add_handler(urllib2.HTTPBasicAuthHandler(pass_man))
        GitClient.__init__(self, **kwargs)

    def get_url(self, path):
        return self._get_url(path).rstrip("/")

    @classmethod
    def from_parsedurl(cls, parsedurl, **kwargs):
        auth, host = urllib2.splituser(parsedurl.netloc)
        password = parsedurl.password
        if password is not None:
            password = urlunquote(password)
        username = parsedurl.username
        if username is not None:
            username = urlunquote(username)
        # TODO(jelmer): This also strips the username
        parsedurl = parsedurl._replace(netloc=host)
        return cls(urlparse.urlunparse(parsedurl),
                   password=password, username=username, **kwargs)

    def __repr__(self):
        return "%s(%r, dumb=%r)" % (type(self).__name__, self._base_url, self.dumb)

    def _get_url(self, path):
        return urlparse.urljoin(self._base_url, path).rstrip("/") + "/"

    def _http_request(self, url, headers={}, data=None):
        req = urllib2.Request(url, headers=headers, data=data)
        try:
            resp = self.opener.open(req)
        except urllib2.HTTPError as e:
            if e.code == 404:
                raise NotGitRepository()
            if e.code != 200:
                raise GitProtocolError("unexpected http response %d" % e.code)
        return resp

    def _discover_references(self, service, url):
        assert url[-1] == "/"
        url = urlparse.urljoin(url, "info/refs")
        headers = {}
        if self.dumb is not False:
            url += "?service=%s" % service.decode('ascii')
            headers["Content-Type"] = "application/x-%s-request" % (
                service.decode('ascii'))
        resp = self._http_request(url, headers)
        try:
            content_type = resp.info().gettype()
        except AttributeError:
            content_type = resp.info().get_content_type()
        try:
            self.dumb = (not content_type.startswith("application/x-git-"))
            if not self.dumb:
                proto = Protocol(resp.read, None)
                # The first line should mention the service
                try:
                    [pkt] = list(proto.read_pkt_seq())
                except ValueError:
                    raise GitProtocolError(
                        "unexpected number of packets received")
                if pkt.rstrip(b'\n') != (b'# service=' + service):
                    raise GitProtocolError(
                        "unexpected first line %r from smart server" % pkt)
                return read_pkt_refs(proto)
            else:
                return read_info_refs(resp), set()
        finally:
            resp.close()

    def _smart_request(self, service, url, data):
        assert url[-1] == "/"
        url = urlparse.urljoin(url, service)
        headers = {
            "Content-Type": "application/x-%s-request" % service
        }
        resp = self._http_request(url, headers, data)
        try:
            content_type = resp.info().gettype()
        except AttributeError:
            content_type = resp.info().get_content_type()
        if content_type != (
                "application/x-%s-result" % service):
            raise GitProtocolError("Invalid content-type from server: %s"
                % content_type)
        return resp

    def send_pack(self, path, determine_wants, generate_pack_contents,
                  progress=None, write_pack=write_pack_objects):
        """Upload a pack to a remote repository.

        :param path: Repository path (as bytestring)
        :param generate_pack_contents: Function that can return a sequence of
            the shas of the objects to upload.
        :param progress: Optional progress function
        :param write_pack: Function called with (file, iterable of objects) to
            write the objects returned by generate_pack_contents to the server.

        :raises SendPackError: if server rejects the pack data
        :raises UpdateRefsError: if the server supports report-status
                                 and rejects ref updates
        :return: new_refs dictionary containing the changes that were made
            {refname: new_ref}, including deleted refs.
        """
        url = self._get_url(path)
        old_refs, server_capabilities = self._discover_references(
            b"git-receive-pack", url)
        negotiated_capabilities = self._send_capabilities & server_capabilities

        if CAPABILITY_REPORT_STATUS in negotiated_capabilities:
            self._report_status_parser = ReportStatusParser()

        new_refs = determine_wants(dict(old_refs))
        if new_refs is None:
            # Determine wants function is aborting the push.
            return old_refs
        if self.dumb:
            raise NotImplementedError(self.fetch_pack)
        req_data = BytesIO()
        req_proto = Protocol(None, req_data.write)
        (have, want) = self._handle_receive_pack_head(
            req_proto, negotiated_capabilities, old_refs, new_refs)
        if not want and set(new_refs.items()).issubset(set(old_refs.items())):
            return new_refs
        objects = generate_pack_contents(have, want)
        if len(objects) > 0:
            write_pack(req_proto.write_file(), objects)
        resp = self._smart_request("git-receive-pack", url,
                                   data=req_data.getvalue())
        try:
            resp_proto = Protocol(resp.read, None)
            self._handle_receive_pack_tail(resp_proto, negotiated_capabilities,
                progress)
            return new_refs
        finally:
            resp.close()


    def fetch_pack(self, path, determine_wants, graph_walker, pack_data,
                   progress=None):
        """Retrieve a pack from a git smart server.

        :param determine_wants: Callback that returns list of commits to fetch
        :param graph_walker: Object with next() and ack().
        :param pack_data: Callback called for each bit of data in the pack
        :param progress: Callback for progress reports (strings)
        :return: Dictionary with all remote refs (not just those fetched)
        """
        url = self._get_url(path)
        refs, server_capabilities = self._discover_references(
            b"git-upload-pack", url)
        negotiated_capabilities = self._fetch_capabilities & server_capabilities
        wants = determine_wants(refs)
        if wants is not None:
            wants = [cid for cid in wants if cid != ZERO_SHA]
        if not wants:
            return refs
        if self.dumb:
            raise NotImplementedError(self.send_pack)
        req_data = BytesIO()
        req_proto = Protocol(None, req_data.write)
        self._handle_upload_pack_head(
            req_proto, negotiated_capabilities, graph_walker, wants,
            lambda: False)
        resp = self._smart_request(
            "git-upload-pack", url, data=req_data.getvalue())
        try:
            resp_proto = Protocol(resp.read, None)
            self._handle_upload_pack_tail(resp_proto, negotiated_capabilities,
                graph_walker, pack_data, progress)
            return refs
        finally:
            resp.close()

    def get_refs(self, path):
        """Retrieve the current refs from a git smart server."""
        url = self._get_url(path)
        refs, _ = self._discover_references(
            b"git-upload-pack", url)
        return refs


def get_transport_and_path_from_url(url, config=None, **kwargs):
    """Obtain a git client from a URL.

    :param url: URL to open (a unicode string)
    :param config: Optional config object
    :param thin_packs: Whether or not thin packs should be retrieved
    :param report_activity: Optional callback for reporting transport
        activity.
    :return: Tuple with client instance and relative path.
    """
    parsed = urlparse.urlparse(url)
    if parsed.scheme == 'git':
        return (TCPGitClient.from_parsedurl(parsed, **kwargs),
                parsed.path)
    elif parsed.scheme in ('git+ssh', 'ssh'):
        path = parsed.path
        if path.startswith('/'):
            path = parsed.path[1:]
        return SSHGitClient.from_parsedurl(parsed, **kwargs), path
    elif parsed.scheme in ('http', 'https'):
        return HttpGitClient.from_parsedurl(
            parsed, config=config, **kwargs), parsed.path
    elif parsed.scheme == 'file':
        return default_local_git_client_cls.from_parsedurl(
            parsed, **kwargs), parsed.path

    raise ValueError("unknown scheme '%s'" % parsed.scheme)


def get_transport_and_path(location, **kwargs):
    """Obtain a git client from a URL.

    :param location: URL or path (a string)
    :param config: Optional config object
    :param thin_packs: Whether or not thin packs should be retrieved
    :param report_activity: Optional callback for reporting transport
        activity.
    :return: Tuple with client instance and relative path.
    """
    # First, try to parse it as a URL
    try:
        return get_transport_and_path_from_url(location, **kwargs)
    except ValueError:
        pass

    if (sys.platform == 'win32' and
            location[0].isalpha() and location[1:3] == ':\\'):
        # Windows local path
        return default_local_git_client_cls(**kwargs), location

    if ':' in location and not '@' in location:
        # SSH with no user@, zero or one leading slash.
        (hostname, path) = location.split(':', 1)
        return SSHGitClient(hostname, **kwargs), path
    elif ':' in location:
        # SSH with user@host:foo.
        user_host, path = location.split(':', 1)
        if '@' in user_host:
            user, host = user_host.rsplit('@', 1)
        else:
            user = None
            host = user_host
        return SSHGitClient(host, username=user, **kwargs), path

    # Otherwise, assume it's a local path.
    return default_local_git_client_cls(**kwargs), location
