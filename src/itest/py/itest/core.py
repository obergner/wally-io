""" Integration test core classes
"""
import signal
import subprocess
import logging
import time
import atexit

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
        self.__started = False

    def start(self):
        """ Start WallyIO subprocess
        """
        logging.info("Starting %s ...", self.__name)
        args = ['./target/sanitize/wally-iod',
                '--log-file', './target/itest/itest_server.log',
                '--log-file-level', 'trace',
                '--conn-timeout', '2000']
        self.__process = subprocess.Popen(args)
        self.__started = True
        time.sleep(1)
        logging.info("Started %s", self.__name)

    def is_started(self):
        """ Test whether this server is started
        """
        return self.__started

    def stop(self):
        """ Stop WallyIO subprocess
        """
        logging.info("Stopping %s ...", self.__name)
        self.__process.send_signal(signal.SIGTERM)
        self.__started = False
        logging.info("Stopped %s", self.__name)

SERVER_UNDER_TEST = ServerUnderTest("ServerUnderTest")

def shutdown_server():
    """ Shutdown server
    """
    SERVER_UNDER_TEST.stop()

atexit.register(shutdown_server)
