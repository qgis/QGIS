# Copyright (c) 2005 Allan Saddi <allan@saddi.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# $Id$

__author__ = 'Allan Saddi <allan@saddi.com>'
__version__ = '$Revision$'

import sys
import thread
import threading

class ThreadPool(object):
    """
    Thread pool that maintains the number of idle threads between
    minSpare and maxSpare inclusive. By default, there is no limit on
    the number of threads that can be started, but this can be controlled
    by maxThreads.
    """
    def __init__(self, minSpare=1, maxSpare=5, maxThreads=sys.maxint):
        self._minSpare = minSpare
        self._maxSpare = maxSpare
        self._maxThreads = max(minSpare, maxThreads)

        self._lock = threading.Condition()
        self._workQueue = []
        self._idleCount = self._workerCount = maxSpare

        self._threads = []
        self._stop = False

        # Start the minimum number of worker threads.
        for i in range(maxSpare):
            self._start_new_thread()

    def _start_new_thread(self):
        t = threading.Thread(target=self._worker)
        self._threads.append(t)
        t.setDaemon(True)
        t.start()
        return t

    def shutdown(self):
        """shutdown all workers."""
        self._lock.acquire()
        self._stop = True
        self._lock.notifyAll()
        self._lock.release()

        # wait for all threads to finish
        for t in self._threads[:]:
            t.join()

    def addJob(self, job, allowQueuing=True):
        """
        Adds a job to the work queue. The job object should have a run()
        method. If allowQueuing is True (the default), the job will be
        added to the work queue regardless if there are any idle threads
        ready. (The only way for there to be no idle threads is if maxThreads
        is some reasonable, finite limit.)

        Otherwise, if allowQueuing is False, and there are no more idle
        threads, the job will not be queued.

        Returns True if the job was queued, False otherwise.
        """
        self._lock.acquire()
        try:
            # Maintain minimum number of spares.
            while self._idleCount < self._minSpare and \
                  self._workerCount < self._maxThreads:
                try:
                    self._start_new_thread()
                except thread.error:
                    return False
                self._workerCount += 1
                self._idleCount += 1

            # Hand off the job.
            if self._idleCount or allowQueuing:
                self._workQueue.append(job)
                self._lock.notify()
                return True
            else:
                return False
        finally:
            self._lock.release()

    def _worker(self):
        """
        Worker thread routine. Waits for a job, executes it, repeat.
        """
        self._lock.acquire()
        try:
            while True:
                while not self._workQueue and not self._stop:
                    self._lock.wait()

                if self._stop:
                    return

                # We have a job to do...
                job = self._workQueue.pop(0)

                assert self._idleCount > 0
                self._idleCount -= 1

                self._lock.release()

                try:
                    job.run()
                except:
                    # FIXME: This should really be reported somewhere.
                    # But we can't simply report it to stderr because of fcgi
                    pass

                self._lock.acquire()

                if self._idleCount == self._maxSpare:
                    break # NB: lock still held
                self._idleCount += 1
                assert self._idleCount <= self._maxSpare

            # Die off...
            assert self._workerCount > self._maxSpare
            self._threads.remove(threading.currentThread())
            self._workerCount -= 1
        finally:
            self._lock.release()
