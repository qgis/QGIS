# reflog.py -- Parsing and writing reflog files
# Copyright (C) 2015 Jelmer Vernooij and others.
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

"""Utilities for reading and generating reflogs.
"""

import collections

from dulwich.objects import (
    format_timezone,
    parse_timezone,
    ZERO_SHA,
    )

Entry = collections.namedtuple('Entry', ['old_sha', 'new_sha', 'committer',
    'timestamp', 'timezone', 'message'])


def format_reflog_line(old_sha, new_sha, committer, timestamp, timezone, message):
    """Generate a single reflog line.

    :param old_sha: Old Commit SHA
    :param new_sha: New Commit SHA
    :param committer: Committer name and e-mail
    :param timestamp: Timestamp
    :param timezone: Timezone
    :param message: Message
    """
    if old_sha is None:
        old_sha = ZERO_SHA
    return (old_sha + b' ' + new_sha + b' ' + committer + b' ' +
            str(timestamp).encode('ascii') + b' ' +
            format_timezone(timezone) + b'\t' + message)


def parse_reflog_line(line):
    """Parse a reflog line.

    :param line: Line to parse
    :return: Tuple of (old_sha, new_sha, committer, timestamp, timezone,
        message)
    """
    (begin, message) = line.split(b'\t', 1)
    (old_sha, new_sha, rest) = begin.split(b' ', 2)
    (committer, timestamp_str, timezone_str) = rest.rsplit(b' ', 2)
    return Entry(old_sha, new_sha, committer, int(timestamp_str),
                 parse_timezone(timezone_str)[0], message)


def read_reflog(f):
    """Read reflog.

    :param f: File-like object
    :returns: Iterator over Entry objects
    """
    for l in f:
        yield parse_reflog_line(l)
