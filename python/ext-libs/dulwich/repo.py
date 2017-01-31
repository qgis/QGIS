# repo.py -- For dealing with git repositories.
# Copyright (C) 2007 James Westby <jw+debian@jameswestby.net>
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


"""Repository access.

This module contains the base class for git repositories
(BaseRepo) and an implementation which uses a repository on
local disk (Repo).

"""

from io import BytesIO
import errno
import os
import sys
import stat

from dulwich.errors import (
    NoIndexPresent,
    NotBlobError,
    NotCommitError,
    NotGitRepository,
    NotTreeError,
    NotTagError,
    CommitError,
    RefFormatError,
    HookError,
    )
from dulwich.file import (
    GitFile,
    )
from dulwich.object_store import (
    DiskObjectStore,
    MemoryObjectStore,
    ObjectStoreGraphWalker,
    )
from dulwich.objects import (
    check_hexsha,
    Blob,
    Commit,
    ShaFile,
    Tag,
    Tree,
    )

from dulwich.hooks import (
    PreCommitShellHook,
    PostCommitShellHook,
    CommitMsgShellHook,
    )

from dulwich.refs import (
    check_ref_format,
    RefsContainer,
    DictRefsContainer,
    InfoRefsContainer,
    DiskRefsContainer,
    read_packed_refs,
    read_packed_refs_with_peeled,
    write_packed_refs,
    SYMREF,
    )


import warnings


CONTROLDIR = '.git'
OBJECTDIR = 'objects'
REFSDIR = 'refs'
REFSDIR_TAGS = 'tags'
REFSDIR_HEADS = 'heads'
INDEX_FILENAME = "index"
COMMONDIR = 'commondir'
GITDIR = 'gitdir'
WORKTREES = 'worktrees'

BASE_DIRECTORIES = [
    ["branches"],
    [REFSDIR],
    [REFSDIR, REFSDIR_TAGS],
    [REFSDIR, REFSDIR_HEADS],
    ["hooks"],
    ["info"]
    ]

DEFAULT_REF = b'refs/heads/master'


def parse_graftpoints(graftpoints):
    """Convert a list of graftpoints into a dict

    :param graftpoints: Iterator of graftpoint lines

    Each line is formatted as:
        <commit sha1> <parent sha1> [<parent sha1>]*

    Resulting dictionary is:
        <commit sha1>: [<parent sha1>*]

    https://git.wiki.kernel.org/index.php/GraftPoint
    """
    grafts = {}
    for l in graftpoints:
        raw_graft = l.split(None, 1)

        commit = raw_graft[0]
        if len(raw_graft) == 2:
            parents = raw_graft[1].split()
        else:
            parents = []

        for sha in [commit] + parents:
            check_hexsha(sha, 'Invalid graftpoint')

        grafts[commit] = parents
    return grafts


def serialize_graftpoints(graftpoints):
    """Convert a dictionary of grafts into string

    The graft dictionary is:
        <commit sha1>: [<parent sha1>*]

    Each line is formatted as:
        <commit sha1> <parent sha1> [<parent sha1>]*

    https://git.wiki.kernel.org/index.php/GraftPoint

    """
    graft_lines = []
    for commit, parents in graftpoints.items():
        if parents:
            graft_lines.append(commit + b' ' + b' '.join(parents))
        else:
            graft_lines.append(commit)
    return b'\n'.join(graft_lines)


class BaseRepo(object):
    """Base class for a git repository.

    :ivar object_store: Dictionary-like object for accessing
        the objects
    :ivar refs: Dictionary-like object with the refs in this
        repository
    """

    def __init__(self, object_store, refs):
        """Open a repository.

        This shouldn't be called directly, but rather through one of the
        base classes, such as MemoryRepo or Repo.

        :param object_store: Object store to use
        :param refs: Refs container to use
        """
        self.object_store = object_store
        self.refs = refs

        self._graftpoints = {}
        self.hooks = {}

    def _determine_file_mode(self):
        """Probe the file-system to determine whether permissions can be trusted.

        :return: True if permissions can be trusted, False otherwise.
        """
        raise NotImplementedError(self._determine_file_mode)

    def _init_files(self, bare):
        """Initialize a default set of named files."""
        from dulwich.config import ConfigFile
        self._put_named_file('description', b"Unnamed repository")
        f = BytesIO()
        cf = ConfigFile()
        cf.set(b"core", b"repositoryformatversion", b"0")
        if self._determine_file_mode():
            cf.set(b"core", b"filemode", True)
        else:
            cf.set(b"core", b"filemode", False)

        cf.set(b"core", b"bare", bare)
        cf.set(b"core", b"logallrefupdates", True)
        cf.write_to_file(f)
        self._put_named_file('config', f.getvalue())
        self._put_named_file(os.path.join('info', 'exclude'), b'')

    def get_named_file(self, path):
        """Get a file from the control dir with a specific name.

        Although the filename should be interpreted as a filename relative to
        the control dir in a disk-based Repo, the object returned need not be
        pointing to a file in that location.

        :param path: The path to the file, relative to the control dir.
        :return: An open file object, or None if the file does not exist.
        """
        raise NotImplementedError(self.get_named_file)

    def _put_named_file(self, path, contents):
        """Write a file to the control dir with the given name and contents.

        :param path: The path to the file, relative to the control dir.
        :param contents: A string to write to the file.
        """
        raise NotImplementedError(self._put_named_file)

    def open_index(self):
        """Open the index for this repository.

        :raise NoIndexPresent: If no index is present
        :return: The matching `Index`
        """
        raise NotImplementedError(self.open_index)

    def fetch(self, target, determine_wants=None, progress=None):
        """Fetch objects into another repository.

        :param target: The target repository
        :param determine_wants: Optional function to determine what refs to
            fetch.
        :param progress: Optional progress function
        :return: The local refs
        """
        if determine_wants is None:
            determine_wants = target.object_store.determine_wants_all
        target.object_store.add_objects(
            self.fetch_objects(determine_wants, target.get_graph_walker(),
                               progress))
        return self.get_refs()

    def fetch_objects(self, determine_wants, graph_walker, progress,
                      get_tagged=None):
        """Fetch the missing objects required for a set of revisions.

        :param determine_wants: Function that takes a dictionary with heads
            and returns the list of heads to fetch.
        :param graph_walker: Object that can iterate over the list of revisions
            to fetch and has an "ack" method that will be called to acknowledge
            that a revision is present.
        :param progress: Simple progress function that will be called with
            updated progress strings.
        :param get_tagged: Function that returns a dict of pointed-to sha -> tag
            sha for including tags.
        :return: iterator over objects, with __len__ implemented
        """
        wants = determine_wants(self.get_refs())
        if not isinstance(wants, list):
            raise TypeError("determine_wants() did not return a list")

        shallows = getattr(graph_walker, 'shallow', frozenset())
        unshallows = getattr(graph_walker, 'unshallow', frozenset())

        if wants == []:
            # TODO(dborowitz): find a way to short-circuit that doesn't change
            # this interface.

            if shallows or unshallows:
                # Do not send a pack in shallow short-circuit path
                return None

            return []

        # If the graph walker is set up with an implementation that can
        # ACK/NAK to the wire, it will write data to the client through
        # this call as a side-effect.
        haves = self.object_store.find_common_revisions(graph_walker)

        # Deal with shallow requests separately because the haves do
        # not reflect what objects are missing
        if shallows or unshallows:
            haves = []  # TODO: filter the haves commits from iter_shas.
                        # the specific commits aren't missing.

        def get_parents(commit):
            if commit.id in shallows:
                return []
            return self.get_parents(commit.id, commit)

        return self.object_store.iter_shas(
          self.object_store.find_missing_objects(
              haves, wants, progress,
              get_tagged,
              get_parents=get_parents))

    def get_graph_walker(self, heads=None):
        """Retrieve a graph walker.

        A graph walker is used by a remote repository (or proxy)
        to find out which objects are present in this repository.

        :param heads: Repository heads to use (optional)
        :return: A graph walker object
        """
        if heads is None:
            heads = self.refs.as_dict(b'refs/heads').values()
        return ObjectStoreGraphWalker(heads, self.get_parents)

    def get_refs(self):
        """Get dictionary with all refs.

        :return: A ``dict`` mapping ref names to SHA1s
        """
        return self.refs.as_dict()

    def head(self):
        """Return the SHA1 pointed at by HEAD."""
        return self.refs[b'HEAD']

    def _get_object(self, sha, cls):
        assert len(sha) in (20, 40)
        ret = self.get_object(sha)
        if not isinstance(ret, cls):
            if cls is Commit:
                raise NotCommitError(ret)
            elif cls is Blob:
                raise NotBlobError(ret)
            elif cls is Tree:
                raise NotTreeError(ret)
            elif cls is Tag:
                raise NotTagError(ret)
            else:
                raise Exception("Type invalid: %r != %r" % (
                  ret.type_name, cls.type_name))
        return ret

    def get_object(self, sha):
        """Retrieve the object with the specified SHA.

        :param sha: SHA to retrieve
        :return: A ShaFile object
        :raise KeyError: when the object can not be found
        """
        return self.object_store[sha]

    def get_parents(self, sha, commit=None):
        """Retrieve the parents of a specific commit.

        If the specific commit is a graftpoint, the graft parents
        will be returned instead.

        :param sha: SHA of the commit for which to retrieve the parents
        :param commit: Optional commit matching the sha
        :return: List of parents
        """

        try:
            return self._graftpoints[sha]
        except KeyError:
            if commit is None:
                commit = self[sha]
            return commit.parents

    def get_config(self):
        """Retrieve the config object.

        :return: `ConfigFile` object for the ``.git/config`` file.
        """
        raise NotImplementedError(self.get_config)

    def get_description(self):
        """Retrieve the description for this repository.

        :return: String with the description of the repository
            as set by the user.
        """
        raise NotImplementedError(self.get_description)

    def set_description(self, description):
        """Set the description for this repository.

        :param description: Text to set as description for this repository.
        """
        raise NotImplementedError(self.set_description)

    def get_config_stack(self):
        """Return a config stack for this repository.

        This stack accesses the configuration for both this repository
        itself (.git/config) and the global configuration, which usually
        lives in ~/.gitconfig.

        :return: `Config` instance for this repository
        """
        from dulwich.config import StackedConfig
        backends = [self.get_config()] + StackedConfig.default_backends()
        return StackedConfig(backends, writable=backends[0])

    def get_peeled(self, ref):
        """Get the peeled value of a ref.

        :param ref: The refname to peel.
        :return: The fully-peeled SHA1 of a tag object, after peeling all
            intermediate tags; if the original ref does not point to a tag, this
            will equal the original SHA1.
        """
        cached = self.refs.get_peeled(ref)
        if cached is not None:
            return cached
        return self.object_store.peel_sha(self.refs[ref]).id

    def get_walker(self, include=None, *args, **kwargs):
        """Obtain a walker for this repository.

        :param include: Iterable of SHAs of commits to include along with their
            ancestors. Defaults to [HEAD]
        :param exclude: Iterable of SHAs of commits to exclude along with their
            ancestors, overriding includes.
        :param order: ORDER_* constant specifying the order of results. Anything
            other than ORDER_DATE may result in O(n) memory usage.
        :param reverse: If True, reverse the order of output, requiring O(n)
            memory.
        :param max_entries: The maximum number of entries to yield, or None for
            no limit.
        :param paths: Iterable of file or subtree paths to show entries for.
        :param rename_detector: diff.RenameDetector object for detecting
            renames.
        :param follow: If True, follow path across renames/copies. Forces a
            default rename_detector.
        :param since: Timestamp to list commits after.
        :param until: Timestamp to list commits before.
        :param queue_cls: A class to use for a queue of commits, supporting the
            iterator protocol. The constructor takes a single argument, the
            Walker.
        :return: A `Walker` object
        """
        from dulwich.walk import Walker
        if include is None:
            include = [self.head()]
        if isinstance(include, str):
            include = [include]

        kwargs['get_parents'] = lambda commit: self.get_parents(commit.id, commit)

        return Walker(self.object_store, include, *args, **kwargs)

    def __getitem__(self, name):
        """Retrieve a Git object by SHA1 or ref.

        :param name: A Git object SHA1 or a ref name
        :return: A `ShaFile` object, such as a Commit or Blob
        :raise KeyError: when the specified ref or object does not exist
        """
        if not isinstance(name, bytes):
            raise TypeError("'name' must be bytestring, not %.80s" %
                    type(name).__name__)
        if len(name) in (20, 40):
            try:
                return self.object_store[name]
            except (KeyError, ValueError):
                pass
        try:
            return self.object_store[self.refs[name]]
        except RefFormatError:
            raise KeyError(name)

    def __contains__(self, name):
        """Check if a specific Git object or ref is present.

        :param name: Git object SHA1 or ref name
        """
        if len(name) in (20, 40):
            return name in self.object_store or name in self.refs
        else:
            return name in self.refs

    def __setitem__(self, name, value):
        """Set a ref.

        :param name: ref name
        :param value: Ref value - either a ShaFile object, or a hex sha
        """
        if name.startswith(b"refs/") or name == b'HEAD':
            if isinstance(value, ShaFile):
                self.refs[name] = value.id
            elif isinstance(value, bytes):
                self.refs[name] = value
            else:
                raise TypeError(value)
        else:
            raise ValueError(name)

    def __delitem__(self, name):
        """Remove a ref.

        :param name: Name of the ref to remove
        """
        if name.startswith(b"refs/") or name == b"HEAD":
            del self.refs[name]
        else:
            raise ValueError(name)

    def _get_user_identity(self):
        """Determine the identity to use for new commits.
        """
        config = self.get_config_stack()
        return (config.get((b"user", ), b"name") + b" <" +
                config.get((b"user", ), b"email") + b">")

    def _add_graftpoints(self, updated_graftpoints):
        """Add or modify graftpoints

        :param updated_graftpoints: Dict of commit shas to list of parent shas
        """

        # Simple validation
        for commit, parents in updated_graftpoints.items():
            for sha in [commit] + parents:
                check_hexsha(sha, 'Invalid graftpoint')

        self._graftpoints.update(updated_graftpoints)

    def _remove_graftpoints(self, to_remove=[]):
        """Remove graftpoints

        :param to_remove: List of commit shas
        """
        for sha in to_remove:
            del self._graftpoints[sha]

    def do_commit(self, message=None, committer=None,
                  author=None, commit_timestamp=None,
                  commit_timezone=None, author_timestamp=None,
                  author_timezone=None, tree=None, encoding=None,
                  ref=b'HEAD', merge_heads=None):
        """Create a new commit.

        :param message: Commit message
        :param committer: Committer fullname
        :param author: Author fullname (defaults to committer)
        :param commit_timestamp: Commit timestamp (defaults to now)
        :param commit_timezone: Commit timestamp timezone (defaults to GMT)
        :param author_timestamp: Author timestamp (defaults to commit timestamp)
        :param author_timezone: Author timestamp timezone
            (defaults to commit timestamp timezone)
        :param tree: SHA1 of the tree root to use (if not specified the
            current index will be committed).
        :param encoding: Encoding
        :param ref: Optional ref to commit to (defaults to current branch)
        :param merge_heads: Merge heads (defaults to .git/MERGE_HEADS)
        :return: New commit SHA1
        """
        import time
        c = Commit()
        if tree is None:
            index = self.open_index()
            c.tree = index.commit(self.object_store)
        else:
            if len(tree) != 40:
                raise ValueError("tree must be a 40-byte hex sha string")
            c.tree = tree

        try:
            self.hooks['pre-commit'].execute()
        except HookError as e:
            raise CommitError(e)
        except KeyError:  # no hook defined, silent fallthrough
            pass

        if merge_heads is None:
            # FIXME: Read merge heads from .git/MERGE_HEADS
            merge_heads = []
        if committer is None:
            # FIXME: Support GIT_COMMITTER_NAME/GIT_COMMITTER_EMAIL environment
            # variables
            committer = self._get_user_identity()
        c.committer = committer
        if commit_timestamp is None:
            # FIXME: Support GIT_COMMITTER_DATE environment variable
            commit_timestamp = time.time()
        c.commit_time = int(commit_timestamp)
        if commit_timezone is None:
            # FIXME: Use current user timezone rather than UTC
            commit_timezone = 0
        c.commit_timezone = commit_timezone
        if author is None:
            # FIXME: Support GIT_AUTHOR_NAME/GIT_AUTHOR_EMAIL environment
            # variables
            author = committer
        c.author = author
        if author_timestamp is None:
            # FIXME: Support GIT_AUTHOR_DATE environment variable
            author_timestamp = commit_timestamp
        c.author_time = int(author_timestamp)
        if author_timezone is None:
            author_timezone = commit_timezone
        c.author_timezone = author_timezone
        if encoding is not None:
            c.encoding = encoding
        if message is None:
            # FIXME: Try to read commit message from .git/MERGE_MSG
            raise ValueError("No commit message specified")

        try:
            c.message = self.hooks['commit-msg'].execute(message)
            if c.message is None:
                c.message = message
        except HookError as e:
            raise CommitError(e)
        except KeyError:  # no hook defined, message not modified
            c.message = message

        if ref is None:
            # Create a dangling commit
            c.parents = merge_heads
            self.object_store.add_object(c)
        else:
            try:
                old_head = self.refs[ref]
                c.parents = [old_head] + merge_heads
                self.object_store.add_object(c)
                ok = self.refs.set_if_equals(ref, old_head, c.id)
            except KeyError:
                c.parents = merge_heads
                self.object_store.add_object(c)
                ok = self.refs.add_if_new(ref, c.id)
            if not ok:
                # Fail if the atomic compare-and-swap failed, leaving the commit and
                # all its objects as garbage.
                raise CommitError("%s changed during commit" % (ref,))

        try:
            self.hooks['post-commit'].execute()
        except HookError as e:  # silent failure
            warnings.warn("post-commit hook failed: %s" % e, UserWarning)
        except KeyError:  # no hook defined, silent fallthrough
            pass

        return c.id



def read_gitfile(f):
    """Read a ``.git`` file.

    The first line of the file should start with "gitdir: "

    :param f: File-like object to read from
    :return: A path
    """
    cs = f.read()
    if not cs.startswith("gitdir: "):
        raise ValueError("Expected file to start with 'gitdir: '")
    return cs[len("gitdir: "):].rstrip("\n")


class Repo(BaseRepo):
    """A git repository backed by local disk.

    To open an existing repository, call the contructor with
    the path of the repository.

    To create a new repository, use the Repo.init class method.
    """

    def __init__(self, root):
        hidden_path = os.path.join(root, CONTROLDIR)
        if os.path.isdir(os.path.join(hidden_path, OBJECTDIR)):
            self.bare = False
            self._controldir = hidden_path
        elif (os.path.isdir(os.path.join(root, OBJECTDIR)) and
              os.path.isdir(os.path.join(root, REFSDIR))):
            self.bare = True
            self._controldir = root
        elif os.path.isfile(hidden_path):
            self.bare = False
            with open(hidden_path, 'r') as f:
                path = read_gitfile(f)
            self.bare = False
            self._controldir = os.path.join(root, path)
        else:
            raise NotGitRepository(
                "No git repository was found at %(path)s" % dict(path=root)
            )
        commondir = self.get_named_file(COMMONDIR)
        if commondir is not None:
            with commondir:
                self._commondir = os.path.join(
                    self.controldir(),
                    commondir.read().rstrip(b"\r\n").decode(sys.getfilesystemencoding()))
        else:
            self._commondir = self._controldir
        self.path = root
        object_store = DiskObjectStore(
            os.path.join(self.commondir(), OBJECTDIR))
        refs = DiskRefsContainer(self.commondir(), self._controldir)
        BaseRepo.__init__(self, object_store, refs)

        self._graftpoints = {}
        graft_file = self.get_named_file(os.path.join("info", "grafts"),
                                         basedir=self.commondir())
        if graft_file:
            with graft_file:
                self._graftpoints.update(parse_graftpoints(graft_file))
        graft_file = self.get_named_file("shallow",
                                         basedir=self.commondir())
        if graft_file:
            with graft_file:
                self._graftpoints.update(parse_graftpoints(graft_file))

        self.hooks['pre-commit'] = PreCommitShellHook(self.controldir())
        self.hooks['commit-msg'] = CommitMsgShellHook(self.controldir())
        self.hooks['post-commit'] = PostCommitShellHook(self.controldir())

    @classmethod
    def discover(cls, start='.'):
        """Iterate parent directories to discover a repository

        Return a Repo object for the first parent directory that looks like a
        Git repository.

        :param start: The directory to start discovery from (defaults to '.')
        """
        remaining = True
        path = os.path.abspath(start)
        while remaining:
            try:
                return cls(path)
            except NotGitRepository:
                path, remaining = os.path.split(path)
        raise NotGitRepository(
            "No git repository was found at %(path)s" % dict(path=start)
        )

    def controldir(self):
        """Return the path of the control directory."""
        return self._controldir

    def commondir(self):
        """Return the path of the common directory.

        For a main working tree, it is identical to `controldir()`.

        For a linked working tree, it is the control directory of the
        main working tree."""

        return self._commondir

    def _determine_file_mode(self):
        """Probe the file-system to determine whether permissions can be trusted.

        :return: True if permissions can be trusted, False otherwise.
        """
        fname = os.path.join(self.path, '.probe-permissions')
        with open(fname, 'w') as f:
            f.write('')

        st1 = os.lstat(fname)
        os.chmod(fname, st1.st_mode ^ stat.S_IXUSR)
        st2 = os.lstat(fname)

        os.unlink(fname)

        mode_differs = st1.st_mode != st2.st_mode
        st2_has_exec = (st2.st_mode & stat.S_IXUSR) != 0

        return mode_differs and st2_has_exec

    def _put_named_file(self, path, contents):
        """Write a file to the control dir with the given name and contents.

        :param path: The path to the file, relative to the control dir.
        :param contents: A string to write to the file.
        """
        path = path.lstrip(os.path.sep)
        with GitFile(os.path.join(self.controldir(), path), 'wb') as f:
            f.write(contents)

    def get_named_file(self, path, basedir=None):
        """Get a file from the control dir with a specific name.

        Although the filename should be interpreted as a filename relative to
        the control dir in a disk-based Repo, the object returned need not be
        pointing to a file in that location.

        :param path: The path to the file, relative to the control dir.
        :param basedir: Optional argument that specifies an alternative to the control dir.
        :return: An open file object, or None if the file does not exist.
        """
        # TODO(dborowitz): sanitize filenames, since this is used directly by
        # the dumb web serving code.
        if basedir is None:
            basedir = self.controldir()
        path = path.lstrip(os.path.sep)
        try:
            return open(os.path.join(basedir, path), 'rb')
        except (IOError, OSError) as e:
            if e.errno == errno.ENOENT:
                return None
            raise

    def index_path(self):
        """Return path to the index file."""
        return os.path.join(self.controldir(), INDEX_FILENAME)

    def open_index(self):
        """Open the index for this repository.

        :raise NoIndexPresent: If no index is present
        :return: The matching `Index`
        """
        from dulwich.index import Index
        if not self.has_index():
            raise NoIndexPresent()
        return Index(self.index_path())

    def has_index(self):
        """Check if an index is present."""
        # Bare repos must never have index files; non-bare repos may have a
        # missing index file, which is treated as empty.
        return not self.bare

    def stage(self, fs_paths):
        """Stage a set of paths.

        :param fs_paths: List of paths, relative to the repository path
        """

        root_path_bytes = self.path.encode(sys.getfilesystemencoding())

        if not isinstance(fs_paths, list):
            fs_paths = [fs_paths]
        from dulwich.index import (
            blob_from_path_and_stat,
            index_entry_from_stat,
            _fs_to_tree_path,
            )
        index = self.open_index()
        for fs_path in fs_paths:
            if not isinstance(fs_path, bytes):
                fs_path = fs_path.encode(sys.getfilesystemencoding())
            tree_path = _fs_to_tree_path(fs_path)
            full_path = os.path.join(root_path_bytes, fs_path)
            try:
                st = os.lstat(full_path)
            except OSError:
                # File no longer exists
                try:
                    del index[tree_path]
                except KeyError:
                    pass  # already removed
            else:
                blob = blob_from_path_and_stat(full_path, st)
                self.object_store.add_object(blob)
                index[tree_path] = index_entry_from_stat(st, blob.id, 0)
        index.write()

    def clone(self, target_path, mkdir=True, bare=False,
            origin=b"origin"):
        """Clone this repository.

        :param target_path: Target path
        :param mkdir: Create the target directory
        :param bare: Whether to create a bare repository
        :param origin: Base name for refs in target repository
            cloned from this repository
        :return: Created repository as `Repo`
        """
        if not bare:
            target = self.init(target_path, mkdir=mkdir)
        else:
            target = self.init_bare(target_path)
        self.fetch(target)
        target.refs.import_refs(
            b'refs/remotes/' + origin, self.refs.as_dict(b'refs/heads'))
        target.refs.import_refs(
            b'refs/tags', self.refs.as_dict(b'refs/tags'))
        try:
            target.refs.add_if_new(DEFAULT_REF, self.refs[DEFAULT_REF])
        except KeyError:
            pass
        target_config = target.get_config()
        encoded_path = self.path
        if not isinstance(encoded_path, bytes):
            encoded_path = encoded_path.encode(sys.getfilesystemencoding())
        target_config.set((b'remote', b'origin'), b'url', encoded_path)
        target_config.set((b'remote', b'origin'), b'fetch',
            b'+refs/heads/*:refs/remotes/origin/*')
        target_config.write_to_path()

        # Update target head
        head_chain, head_sha = self.refs.follow(b'HEAD')
        if head_chain and head_sha is not None:
            target.refs.set_symbolic_ref(b'HEAD', head_chain[-1])
            target[b'HEAD'] = head_sha

            if not bare:
                # Checkout HEAD to target dir
                target.reset_index()

        return target

    def reset_index(self, tree=None):
        """Reset the index back to a specific tree.

        :param tree: Tree SHA to reset to, None for current HEAD tree.
        """
        from dulwich.index import (
            build_index_from_tree,
            validate_path_element_default,
            validate_path_element_ntfs,
            )
        if tree is None:
            tree = self[b'HEAD'].tree
        config = self.get_config()
        honor_filemode = config.get_boolean('core', 'filemode', os.name != "nt")
        if config.get_boolean('core', 'core.protectNTFS', os.name == "nt"):
            validate_path_element = validate_path_element_ntfs
        else:
            validate_path_element = validate_path_element_default
        return build_index_from_tree(self.path, self.index_path(),
                self.object_store, tree, honor_filemode=honor_filemode,
                validate_path_element=validate_path_element)

    def get_config(self):
        """Retrieve the config object.

        :return: `ConfigFile` object for the ``.git/config`` file.
        """
        from dulwich.config import ConfigFile
        path = os.path.join(self._controldir, 'config')
        try:
            return ConfigFile.from_path(path)
        except (IOError, OSError) as e:
            if e.errno != errno.ENOENT:
                raise
            ret = ConfigFile()
            ret.path = path
            return ret

    def get_description(self):
        """Retrieve the description of this repository.

        :return: A string describing the repository or None.
        """
        path = os.path.join(self._controldir, 'description')
        try:
            with GitFile(path, 'rb') as f:
                return f.read()
        except (IOError, OSError) as e:
            if e.errno != errno.ENOENT:
                raise
            return None

    def __repr__(self):
        return "<Repo at %r>" % self.path

    def set_description(self, description):
        """Set the description for this repository.

        :param description: Text to set as description for this repository.
        """

        self._put_named_file('description', description)

    @classmethod
    def _init_maybe_bare(cls, path, bare):
        for d in BASE_DIRECTORIES:
            os.mkdir(os.path.join(path, *d))
        DiskObjectStore.init(os.path.join(path, OBJECTDIR))
        ret = cls(path)
        ret.refs.set_symbolic_ref(b'HEAD', DEFAULT_REF)
        ret._init_files(bare)
        return ret

    @classmethod
    def init(cls, path, mkdir=False):
        """Create a new repository.

        :param path: Path in which to create the repository
        :param mkdir: Whether to create the directory
        :return: `Repo` instance
        """
        if mkdir:
            os.mkdir(path)
        controldir = os.path.join(path, CONTROLDIR)
        os.mkdir(controldir)
        cls._init_maybe_bare(controldir, False)
        return cls(path)

    @classmethod
    def _init_new_working_directory(cls, path, main_repo, identifier=None, mkdir=False):
        """Create a new working directory linked to a repository.

        :param path: Path in which to create the working tree.
        :param main_repo: Main repository to reference
        :param identifier: Worktree identifier
        :param mkdir: Whether to create the directory
        :return: `Repo` instance
        """
        if mkdir:
            os.mkdir(path)
        if identifier is None:
            identifier = os.path.basename(path)
        main_worktreesdir = os.path.join(main_repo.controldir(), WORKTREES)
        worktree_controldir = os.path.join(main_worktreesdir, identifier)
        gitdirfile = os.path.join(path, CONTROLDIR)
        with open(gitdirfile, 'wb') as f:
            f.write(b'gitdir: ' +
                    worktree_controldir.encode(sys.getfilesystemencoding()) +
                    b'\n')
        try:
            os.mkdir(main_worktreesdir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise
        try:
            os.mkdir(worktree_controldir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise
        with open(os.path.join(worktree_controldir, GITDIR), 'wb') as f:
            f.write(gitdirfile.encode(sys.getfilesystemencoding()) + b'\n')
        with open(os.path.join(worktree_controldir, COMMONDIR), 'wb') as f:
            f.write(b'../..\n')
        with open(os.path.join(worktree_controldir, 'HEAD'), 'wb') as f:
            f.write(main_repo.head() + b'\n')
        r = cls(path)
        r.reset_index()
        return r

    @classmethod
    def init_bare(cls, path):
        """Create a new bare repository.

        ``path`` should already exist and be an emty directory.

        :param path: Path to create bare repository in
        :return: a `Repo` instance
        """
        return cls._init_maybe_bare(path, True)

    create = init_bare

    def close(self):
        """Close any files opened by this repository."""
        self.object_store.close()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()


class MemoryRepo(BaseRepo):
    """Repo that stores refs, objects, and named files in memory.

    MemoryRepos are always bare: they have no working tree and no index, since
    those have a stronger dependency on the filesystem.
    """

    def __init__(self):
        from dulwich.config import ConfigFile
        BaseRepo.__init__(self, MemoryObjectStore(), DictRefsContainer({}))
        self._named_files = {}
        self.bare = True
        self._config = ConfigFile()

    def _determine_file_mode(self):
        """Probe the file-system to determine whether permissions can be trusted.

        :return: True if permissions can be trusted, False otherwise.
        """
        return sys.platform != 'win32'

    def _put_named_file(self, path, contents):
        """Write a file to the control dir with the given name and contents.

        :param path: The path to the file, relative to the control dir.
        :param contents: A string to write to the file.
        """
        self._named_files[path] = contents

    def get_named_file(self, path):
        """Get a file from the control dir with a specific name.

        Although the filename should be interpreted as a filename relative to
        the control dir in a disk-baked Repo, the object returned need not be
        pointing to a file in that location.

        :param path: The path to the file, relative to the control dir.
        :return: An open file object, or None if the file does not exist.
        """
        contents = self._named_files.get(path, None)
        if contents is None:
            return None
        return BytesIO(contents)

    def open_index(self):
        """Fail to open index for this repo, since it is bare.

        :raise NoIndexPresent: Raised when no index is present
        """
        raise NoIndexPresent()

    def get_config(self):
        """Retrieve the config object.

        :return: `ConfigFile` object.
        """
        return self._config

    def get_description(self):
        """Retrieve the repository description.

        This defaults to None, for no description.
        """
        return None

    @classmethod
    def init_bare(cls, objects, refs):
        """Create a new bare repository in memory.

        :param objects: Objects for the new repository,
            as iterable
        :param refs: Refs as dictionary, mapping names
            to object SHA1s
        """
        ret = cls()
        for obj in objects:
            ret.object_store.add_object(obj)
        for refname, sha in refs.items():
            ret.refs[refname] = sha
        ret._init_files(bare=True)
        return ret
