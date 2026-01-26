"""
***************************************************************************
    process_wrapper.py
    ---------------------
    Date                 : February 2023
    Copyright            : (C) 2023 by Yoann Quenach de Quivillic
    Email                : yoann dot quenach at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

import locale
import os
import subprocess
import signal
import sys
import time
from queue import Queue, Empty
from threading import Thread

from qgis.PyQt.QtCore import QObject, pyqtSignal


class ProcessWrapper(QObject):

    finished = pyqtSignal(int)

    def __init__(self, command, interactive=True, parent=None):
        super().__init__(parent)

        self.stdout = ""
        self.stderr = ""
        self.returncode = None

        options = {
            "stdout": subprocess.PIPE,
            "stdin": subprocess.PIPE,
            "stderr": subprocess.PIPE,
            "shell": True,
        }

        # On Unix, we can use os.setsid
        # This will allow killing the process and its children when pressing Ctrl+C if psutil is not available
        if hasattr(os, "setsid"):
            options["preexec_fn"] = os.setsid

        # Create and start subprocess
        self.p = subprocess.Popen(command, **options)

        # Start in non-interactive mode, wait for the process to finish
        if not interactive:
            out, err = self.p.communicate()
            self.stdout = self.decode(out)
            self.stderr = self.decode(err)
            self.returncode = self.p.returncode
            return

        # Read process stdout and push to out queue
        self.q_out = Queue()
        self.t_out = Thread(
            daemon=True, target=self.enqueue_output, args=[self.p.stdout, self.q_out]
        )
        self.t_out.start()

        # Read process stderr and push to err queue
        self.q_err = Queue()
        self.t_err = Thread(
            daemon=True, target=self.enqueue_output, args=[self.p.stderr, self.q_err]
        )
        self.t_err.start()

        # Polls process and output both queues content to sys.stdout and sys.stderr
        self.t_queue = Thread(daemon=True, target=self.dequeue_output)
        self.t_queue.start()

    def enqueue_output(self, stream, queue):
        while True:
            # We have to read the character one by one to ensure to
            # forward every available character to the queue
            # self.p.stdout.readline would block on a unfinished line
            char = stream.read(1)
            if not char:
                # Process terminated
                break
            queue.put(char)
        stream.close()

    def __repr__(self):
        """Helpful representation of the maanaged process"""
        status = (
            "Running" if self.returncode is None else f"Completed ({self.returncode})"
        )
        repr = f"ProcessWrapper object at {hex(id(self))}"
        repr += f"\n - Status: {status}"
        repr += f"\n - stdout: {self.stdout}"
        repr += f"\n - stderr: {self.stderr}"
        return repr

    def decode(self, bytes):
        try:
            # Try to decode the content as utf-8 first
            text = bytes.decode("utf8")
        except UnicodeDecodeError:
            try:
                # If it fails, fallback to the default locale encoding
                text = bytes.decode(locale.getdefaultlocale()[1])
            except UnicodeDecodeError:
                # If everything fails, use representation
                text = str(bytes)[2:-1]
        return text

    def read_content(self, queue, stream, is_stderr):
        """Write queue content to the standard stream and append it to the internal buffer"""
        content = b""
        while True:
            try:
                # While queue contains data, append it to content
                content += queue.get_nowait()
            except Empty:
                text = self.decode(content)
                if text:
                    # Append to the internal buffer
                    if is_stderr:
                        self.stderr += text
                    else:
                        self.stdout += text

                    stream.write(text)
                return

    def dequeue_output(self):
        """Check process every 0.1s and forward its outputs to stdout and stderr"""

        # Poll process and forward its outputs to stdout and stderr
        while self.p.poll() is None:
            time.sleep(0.1)
            self.read_content(self.q_out, sys.stdout, is_stderr=False)
            self.read_content(self.q_err, sys.stderr, is_stderr=True)

        # At this point, the process has terminated, so we wait for the threads to finish
        self.t_out.join()
        self.t_err.join()

        # Reaf the remaining content of the queues
        self.read_content(self.q_out, sys.stdout, is_stderr=False)
        self.read_content(self.q_err, sys.stderr, is_stderr=True)

        # Set returncode and emit finished signal
        self.returncode = self.p.returncode
        self.finished.emit(self.returncode)

    def wait(self, timeout=1):
        """Wait for the managed process to finish. If timeout=-1, waits indefinitely (and freeze the GUI)"""
        self.p.wait(timeout)

    def write(self, data):
        """Send data to the managed process"""
        try:
            self.p.stdin.write((data + "\n").encode("utf8"))
            self.p.stdin.flush()
        except BrokenPipeError as exc:
            self.p.stdout.close()
            self.p.stderr.close()
            self.finished.emit(self.p.poll())

    def kill(self):
        """Kill the managed process"""

        # Process in run with shell=True, so calling self.p.kill() would only kill the shell
        # (i.e a text editor launched with !gedit would not close) so we need to iterate
        # over the child processes to kill them all

        try:
            import psutil

            if self.p.returncode is None:
                process = psutil.Process(self.p.pid)
                for child_process in process.children(recursive=True):
                    child_process.kill()
                process.kill()
        except ImportError:
            # If psutil is not available, we try to use os.killpg to kill the process group (Unix only)
            try:
                os.killpg(os.getpgid(self.p.pid), signal.SIGTERM)
            except AttributeError:
                # If everything fails, simply kill the process. Children will not be killed
                self.p.kill()

    def __del__(self):
        """Ensure streams are closed when the process is destroyed"""
        self.p.stdout.close()
        self.p.stderr.close()
        self.p.stdin.close()
        try:
            self.kill()
        except ProcessLookupError:
            pass
