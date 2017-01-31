# patch.py -- For dealing with packed-style patches.
# Copyright (C) 2009-2013 Jelmer Vernooij <jelmer@samba.org>
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

"""Classes for dealing with git am-style patches.

These patches are basically unified diffs with some extra metadata tacked
on.
"""

from difflib import SequenceMatcher
import email.parser
import time

from dulwich.objects import (
    Blob,
    Commit,
    S_ISGITLINK,
    )

FIRST_FEW_BYTES = 8000


def write_commit_patch(f, commit, contents, progress, version=None, encoding=None):
    """Write a individual file patch.

    :param commit: Commit object
    :param progress: Tuple with current patch number and total.
    :return: tuple with filename and contents
    """
    encoding = encoding or getattr(f, "encoding", "ascii")
    if type(contents) is str:
        contents = contents.encode(encoding)
    (num, total) = progress
    f.write(b"From " + commit.id + b" " + time.ctime(commit.commit_time).encode(encoding) + b"\n")
    f.write(b"From: " + commit.author + b"\n")
    f.write(b"Date: " + time.strftime("%a, %d %b %Y %H:%M:%S %Z").encode(encoding) + b"\n")
    f.write(("Subject: [PATCH %d/%d] " % (num, total)).encode(encoding) + commit.message + b"\n")
    f.write(b"\n")
    f.write(b"---\n")
    try:
        import subprocess
        p = subprocess.Popen(["diffstat"], stdout=subprocess.PIPE,
                             stdin=subprocess.PIPE)
    except (ImportError, OSError):
        pass # diffstat not available?
    else:
        (diffstat, _) = p.communicate(contents)
        f.write(diffstat)
        f.write(b"\n")
    f.write(contents)
    f.write(b"-- \n")
    if version is None:
        from dulwich import __version__ as dulwich_version
        f.write(b"Dulwich %d.%d.%d\n" % dulwich_version)
    else:
        f.write(version.encode(encoding) + b"\n")


def get_summary(commit):
    """Determine the summary line for use in a filename.

    :param commit: Commit
    :return: Summary string
    """
    return commit.message.splitlines()[0].replace(" ", "-")


def unified_diff(a, b, fromfile, tofile, n=3):
    """difflib.unified_diff that doesn't write any dates or trailing spaces.

    Based on the same function in Python2.6.5-rc2's difflib.py
    """
    started = False
    for group in SequenceMatcher(None, a, b).get_grouped_opcodes(n):
        if not started:
            yield b'--- ' + fromfile + b'\n'
            yield b'+++ ' + tofile + b'\n'
            started = True
        i1, i2, j1, j2 = group[0][1], group[-1][2], group[0][3], group[-1][4]
        sizes = "@@ -%d,%d +%d,%d @@\n" % (i1+1, i2-i1, j1+1, j2-j1)
        yield sizes.encode('ascii')
        for tag, i1, i2, j1, j2 in group:
            if tag == 'equal':
                for line in a[i1:i2]:
                    yield b' ' + line
                continue
            if tag == 'replace' or tag == 'delete':
                for line in a[i1:i2]:
                    if not line[-1:] == b'\n':
                        line += b'\n\\ No newline at end of file\n'
                    yield b'-' + line
            if tag == 'replace' or tag == 'insert':
                for line in b[j1:j2]:
                    if not line[-1:] == b'\n':
                        line += b'\n\\ No newline at end of file\n'
                    yield b'+' + line


def is_binary(content):
    """See if the first few bytes contain any null characters.

    :param content: Bytestring to check for binary content
    """
    return b'\0' in content[:FIRST_FEW_BYTES]


def shortid(hexsha):
    if hexsha is None:
        return b"0" * 7
    else:
        return hexsha[:7]


def patch_filename(p, root):
    if p is None:
        return b"/dev/null"
    else:
        return root + b"/" + p


def write_object_diff(f, store, old_file, new_file, diff_binary=False):
    """Write the diff for an object.

    :param f: File-like object to write to
    :param store: Store to retrieve objects from, if necessary
    :param old_file: (path, mode, hexsha) tuple
    :param new_file: (path, mode, hexsha) tuple
    :param diff_binary: Whether to diff files even if they
        are considered binary files by is_binary().

    :note: the tuple elements should be None for nonexistant files
    """
    (old_path, old_mode, old_id) = old_file
    (new_path, new_mode, new_id) = new_file
    old_path = patch_filename(old_path, b"a")
    new_path = patch_filename(new_path, b"b")
    def content(mode, hexsha):
        if hexsha is None:
            return Blob.from_string(b'')
        elif S_ISGITLINK(mode):
            return Blob.from_string(b"Submodule commit " + hexsha + b"\n")
        else:
            return store[hexsha]

    def lines(content):
        if not content:
            return []
        else:
            return content.splitlines()
    f.writelines(gen_diff_header(
        (old_path, new_path), (old_mode, new_mode), (old_id, new_id)))
    old_content = content(old_mode, old_id)
    new_content = content(new_mode, new_id)
    if not diff_binary and (
            is_binary(old_content.data) or is_binary(new_content.data)):
        f.write(b"Binary files " + old_path + b" and " + new_path + b" differ\n")
    else:
        f.writelines(unified_diff(lines(old_content), lines(new_content),
            old_path, new_path))


# TODO(jelmer): Support writing unicode, rather than bytes.
def gen_diff_header(paths, modes, shas):
    """Write a blob diff header.

    :param paths: Tuple with old and new path
    :param modes: Tuple with old and new modes
    :param shas: Tuple with old and new shas
    """
    (old_path, new_path) = paths
    (old_mode, new_mode) = modes
    (old_sha, new_sha) = shas
    yield b"diff --git " + old_path + b" " + new_path + b"\n"
    if old_mode != new_mode:
        if new_mode is not None:
            if old_mode is not None:
                yield ("old mode %o\n" % old_mode).encode('ascii')
            yield ("new mode %o\n" % new_mode).encode('ascii')
        else:
            yield ("deleted mode %o\n" % old_mode).encode('ascii')
    yield b"index " + shortid(old_sha) + b".." + shortid(new_sha)
    if new_mode is not None:
        yield (" %o" % new_mode).encode('ascii')
    yield b"\n"


# TODO(jelmer): Support writing unicode, rather than bytes.
def write_blob_diff(f, old_file, new_file):
    """Write blob diff.

    :param f: File-like object to write to
    :param old_file: (path, mode, hexsha) tuple (None if nonexisting)
    :param new_file: (path, mode, hexsha) tuple (None if nonexisting)

    :note: The use of write_object_diff is recommended over this function.
    """
    (old_path, old_mode, old_blob) = old_file
    (new_path, new_mode, new_blob) = new_file
    old_path = patch_filename(old_path, b"a")
    new_path = patch_filename(new_path, b"b")
    def lines(blob):
        if blob is not None:
            return blob.splitlines()
        else:
            return []
    f.writelines(gen_diff_header(
        (old_path, new_path), (old_mode, new_mode),
        (getattr(old_blob, "id", None), getattr(new_blob, "id", None))))
    old_contents = lines(old_blob)
    new_contents = lines(new_blob)
    f.writelines(unified_diff(old_contents, new_contents,
        old_path, new_path))


# TODO(jelmer): Support writing unicode, rather than bytes.
def write_tree_diff(f, store, old_tree, new_tree, diff_binary=False):
    """Write tree diff.

    :param f: File-like object to write to.
    :param old_tree: Old tree id
    :param new_tree: New tree id
    :param diff_binary: Whether to diff files even if they
        are considered binary files by is_binary().
    """
    changes = store.tree_changes(old_tree, new_tree)
    for (oldpath, newpath), (oldmode, newmode), (oldsha, newsha) in changes:
        write_object_diff(f, store, (oldpath, oldmode, oldsha),
                                    (newpath, newmode, newsha),
                                    diff_binary=diff_binary)


def git_am_patch_split(f, encoding=None):
    """Parse a git-am-style patch and split it up into bits.

    :param f: File-like object to parse
    :param encoding: Encoding to use when creating Git objects
    :return: Tuple with commit object, diff contents and git version
    """
    encoding = encoding or getattr(f, "encoding", "ascii")
    contents = f.read()
    if type(contents) is bytes and getattr(email.parser, "BytesParser", None):
        parser = email.parser.BytesParser()
        msg = parser.parsebytes(contents)
    else:
        parser = email.parser.Parser()
        msg = parser.parsestr(contents)
    return parse_patch_message(msg, encoding)


def parse_patch_message(msg, encoding=None):
    """Extract a Commit object and patch from an e-mail message.

    :param msg: An email message (email.message.Message)
    :param encoding: Encoding to use to encode Git commits
    :return: Tuple with commit object, diff contents and git version
    """
    c = Commit()
    c.author = msg["from"].encode(encoding)
    c.committer = msg["from"].encode(encoding)
    try:
        patch_tag_start = msg["subject"].index("[PATCH")
    except ValueError:
        subject = msg["subject"]
    else:
        close = msg["subject"].index("] ", patch_tag_start)
        subject = msg["subject"][close+2:]
    c.message = (subject.replace("\n", "") + "\n").encode(encoding)
    first = True

    body = msg.get_payload(decode=True)
    lines = body.splitlines(True)
    line_iter = iter(lines)

    for l in line_iter:
        if l == b"---\n":
            break
        if first:
            if l.startswith(b"From: "):
                c.author = l[len(b"From: "):].rstrip()
            else:
                c.message += b"\n" + l
            first = False
        else:
            c.message += l
    diff = b""
    for l in line_iter:
        if l == b"-- \n":
            break
        diff += l
    try:
        version = next(line_iter).rstrip(b"\n")
    except StopIteration:
        version = None
    return c, diff, version
