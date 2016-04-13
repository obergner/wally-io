""" Integration test core classes
"""
import signal
import subprocess
import logging

class ServerUnderTest(object):
    """ Wraps process running WallyIO
    """

    def __init__(self, name):
        self.name = name
        self.process = None

    def start(self):
        """ Start WallyIO subprocess
        """
        logging.info("Starting %s ...", self.name)
        args = ['./target/sanitize/wally-iod',
                '--log-file', './target/itest/itest.log',
                '--log-file-level', 'trace']
        self.process = subprocess.Popen(args)
        logging.info("Started %s", self.name)

    def stop(self):
        """ Stop WallyIO subprocess
        """
        logging.info("Stopping %s ...", self.name)
        self.process.send_signal(signal.SIGTERM)
        logging.info("Stopped %s", self.name)
