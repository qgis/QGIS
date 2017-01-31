# paramiko_vendor.py -- paramiko implementation of the SSHVendor interface
# Copyright (C) 2013 Aaron O'Mullan <aaron.omullan@friendco.de>
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

"""Paramiko SSH support for Dulwich.

To use this implementation as the SSH implementation in Dulwich, override
the dulwich.client.get_ssh_vendor attribute:

  >>> from dulwich import client as _mod_client
  >>> from dulwich.contrib.paramiko_vendor import ParamikoSSHVendor
  >>> _mod_client.get_ssh_vendor = ParamikoSSHVendor

This implementation is experimental and does not have any tests.
"""

import paramiko
import paramiko.client
import threading

class _ParamikoWrapper(object):
    STDERR_READ_N = 2048  # 2k

    def __init__(self, client, channel, progress_stderr=None):
        self.client = client
        self.channel = channel
        self.progress_stderr = progress_stderr
        self.should_monitor = bool(progress_stderr) or True
        self.monitor_thread = None
        self.stderr = b''

        # Channel must block
        self.channel.setblocking(True)

        # Start
        if self.should_monitor:
            self.monitor_thread = threading.Thread(
                target=self.monitor_stderr)
            self.monitor_thread.start()

    def monitor_stderr(self):
        while self.should_monitor:
            # Block and read
            data = self.read_stderr(self.STDERR_READ_N)

            # Socket closed
            if not data:
                self.should_monitor = False
                break

            # Emit data
            if self.progress_stderr:
                self.progress_stderr(data)

            # Append to buffer
            self.stderr += data

    def stop_monitoring(self):
        # Stop StdErr thread
        if self.should_monitor:
            self.should_monitor = False
            self.monitor_thread.join()

            # Get left over data
            data = self.channel.in_stderr_buffer.empty()
            self.stderr += data

    def can_read(self):
        return self.channel.recv_ready()

    def write(self, data):
        return self.channel.sendall(data)

    def read_stderr(self, n):
        return self.channel.recv_stderr(n)

    def read(self, n=None):
        data = self.channel.recv(n)
        data_len = len(data)

        # Closed socket
        if not data:
            return

        # Read more if needed
        if n and data_len < n:
            diff_len = n - data_len
            return data + self.read(diff_len)
        return data

    def close(self):
        self.channel.close()
        self.stop_monitoring()


class ParamikoSSHVendor(object):

    def __init__(self):
        self.ssh_kwargs = {}

    def run_command(self, host, command, username=None, port=None,
                    progress_stderr=None):
        if not isinstance(command, bytes):
            raise TypeError(command)
        # Paramiko needs an explicit port. None is not valid
        if port is None:
            port = 22

        client = paramiko.SSHClient()

        policy = paramiko.client.MissingHostKeyPolicy()
        client.set_missing_host_key_policy(policy)
        client.connect(host, username=username, port=port,
                       **self.ssh_kwargs)

        # Open SSH session
        channel = client.get_transport().open_session()

        # Run commands
        channel.exec_command(command)

        return _ParamikoWrapper(
            client, channel, progress_stderr=progress_stderr)
