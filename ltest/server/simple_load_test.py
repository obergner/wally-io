""" Very simple and incomplete load test for WallyIO MQTT Server
"""
import time
import logging
import atexit
import loadtest.core

SUBSCRIBERS_COUNT = 3
PUBLISHERS_COUNT = 3
MSGS_PER_PUBLISHER = 5

logging.basicConfig(level=logging.DEBUG)

SERVER_UNDER_TEST = loadtest.core.ServerUnderTest("ServerUnderTest")

def shutdown_server():
    """ Shutdown server at exit
    """
    SERVER_UNDER_TEST.stop()

atexit.register(shutdown_server)

SERVER_UNDER_TEST.start()

time.sleep(2)

logging.info("Starting %s subscribers ...", SUBSCRIBERS_COUNT)
SUBSCRIBERS = []
for i in range(SUBSCRIBERS_COUNT):
    subscr = loadtest.core.Subscriber(i, i % 3)
    SUBSCRIBERS.append(subscr)
    subscr.start()
logging.info("All subscribers started")

logging.info("Starting %s publishers with %s messages to publish each...",
             PUBLISHERS_COUNT, MSGS_PER_PUBLISHER)
PUBLISHERS = []
for i in range(PUBLISHERS_COUNT):
    pub = loadtest.core.Publisher(i, MSGS_PER_PUBLISHER, i % 3)
    PUBLISHERS.append(pub)
    pub.start()
logging.info("All publishers started")

for pub in PUBLISHERS:
    pub.join()

time.sleep(5)

for subscr in SUBSCRIBERS:
    subscr.stop()

for subscr in SUBSCRIBERS:
    subscr.join()

