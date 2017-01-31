# objectspec.py -- Object specification
# Copyright (C) 2014 Jelmer Vernooij <jelmer@samba.org>
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

"""Object specification."""


def to_bytes(text):
    if getattr(text, "encode", None) is not None:
        text = text.encode('ascii')
    return text


def parse_object(repo, objectish):
    """Parse a string referring to an object.

    :param repo: A `Repo` object
    :param objectish: A string referring to an object
    :return: A git object
    :raise KeyError: If the object can not be found
    """
    objectish = to_bytes(objectish)
    return repo[objectish]


def parse_ref(container, refspec):
    """Parse a string referring to a reference.

    :param container: A RefsContainer object
    :param refspec: A string referring to a ref
    :return: A ref
    :raise KeyError: If the ref can not be found
    """
    refspec = to_bytes(refspec)
    possible_refs = [
        refspec,
        b"refs/" + refspec,
        b"refs/tags/" + refspec,
        b"refs/heads/" + refspec,
        b"refs/remotes/" + refspec,
        b"refs/remotes/" + refspec + b"/HEAD"
    ]
    for ref in possible_refs:
        if ref in container:
            return ref
    else:
        raise KeyError(refspec)


def parse_reftuple(lh_container, rh_container, refspec):
    """Parse a reftuple spec.

    :param lh_container: A RefsContainer object
    :param hh_container: A RefsContainer object
    :param refspec: A string
    :return: A tuple with left and right ref
    :raise KeyError: If one of the refs can not be found
    """
    if refspec.startswith(b"+"):
        force = True
        refspec = refspec[1:]
    else:
        force = False
    refspec = to_bytes(refspec)
    if b":" in refspec:
        (lh, rh) = refspec.split(b":")
    else:
        lh = rh = refspec
    if lh == b"":
        lh = None
    else:
        lh = parse_ref(lh_container, lh)
    if rh == b"":
        rh = None
    else:
        try:
            rh = parse_ref(rh_container, rh)
        except KeyError:
            # TODO: check force?
            if not b"/" in rh:
                rh = b"refs/heads/" + rh
    return (lh, rh, force)


def parse_reftuples(lh_container, rh_container, refspecs):
    """Parse a list of reftuple specs to a list of reftuples.

    :param lh_container: A RefsContainer object
    :param hh_container: A RefsContainer object
    :param refspecs: A list of refspecs or a string
    :return: A list of refs
    :raise KeyError: If one of the refs can not be found
    """
    if not isinstance(refspecs, list):
        refspecs = [refspecs]
    ret = []
    # TODO: Support * in refspecs
    for refspec in refspecs:
        ret.append(parse_reftuple(lh_container, rh_container, refspec))
    return ret


def parse_refs(container, refspecs):
    """Parse a list of refspecs to a list of refs.

    :param container: A RefsContainer object
    :param refspecs: A list of refspecs or a string
    :return: A list of refs
    :raise KeyError: If one of the refs can not be found
    """
    # TODO: Support * in refspecs
    if not isinstance(refspecs, list):
        refspecs = [refspecs]
    ret = []
    for refspec in refspecs:
        ret.append(parse_ref(container, refspec))
    return ret


def parse_commit_range(repo, committishs):
    """Parse a string referring to a range of commits.

    :param repo: A `Repo` object
    :param committishs: A string referring to a range of commits.
    :return: An iterator over `Commit` objects
    :raise KeyError: When the reference commits can not be found
    :raise ValueError: If the range can not be parsed
    """
    committishs = to_bytes(committishs)
    # TODO(jelmer): Support more than a single commit..
    return iter([parse_commit(repo, committishs)])


def parse_commit(repo, committish):
    """Parse a string referring to a single commit.

    :param repo: A` Repo` object
    :param commitish: A string referring to a single commit.
    :return: A Commit object
    :raise KeyError: When the reference commits can not be found
    :raise ValueError: If the range can not be parsed
    """
    committish = to_bytes(committish)
    return repo[committish] # For now..


# TODO: parse_path_in_tree(), which handles e.g. v1.0:Documentation
