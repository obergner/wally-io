""" Integration test core classes
"""
import atexit
import logging
import os
import signal
import subprocess
import time

class TimeoutError(Exception):
    """ Timeout exception
    """
    pass

class Timeout(object):
    """ Timeout
    """
    def __init__(self, seconds=1):
        self.secs = seconds

    def handle_timeout(self, signum, frame):
        """ Callback
        """
        raise TimeoutError("Timeout after [%s] seconds [signum: %s|frame: %s]" %
                           (self.secs, signum, frame))

    def seconds(self):
        """ Returns seconds
        """
        return self.seconds

    def __enter__(self):
        signal.signal(signal.SIGALRM, self.handle_timeout)
        signal.alarm(self.secs)

    def __exit__(self, kind, value, traceback):
        signal.alarm(0)

class ServerUnderTest(object):
    """ Wraps process running WallyIO
    """
    def __init__(self, name):
        self.__name = name
        self.__process = None

    def start(self):
        """ Start WallyIO subprocess
        """
        config = os.environ['CONFIG']
        executable = './target/%s/main/wally-iod' % (config)
        log_file = './target/%s/itest/itest_server.log' % (config)
        logging.info("Starting %s ...", self.__name)
        args = [executable,
                '--log-file', log_file,
                '--log-level', 'trace',
                '--conn-timeout', '2000']
        self.__process = subprocess.Popen(args)
        time.sleep(2)
        if self.__process.poll() is not None:
            raise RuntimeError("Failed to start %s" % (self.__name))
        logging.info("Started %s", self.__name)

    def is_started(self):
        """ Test whether this server is started
        """
        return self.__process and self.__process.poll() is None

    def stop(self):
        """ Stop WallyIO subprocess
        """
        logging.info("Stopping %s ...", self.__name)
        self.__process.send_signal(signal.SIGTERM)
        time.sleep(1)
        if self.__process.poll() is None:
            self.__process.send_signal(signal.SIGKILL)
        self.__process.wait()
        logging.info("Stopped %s", self.__name)

SERVER_UNDER_TEST = ServerUnderTest("ServerUnderTest")

def shutdown_server():
    """ Shutdown server
    """
    SERVER_UNDER_TEST.stop()

atexit.register(shutdown_server)
