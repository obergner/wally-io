""" Test unsubscribing from topics
"""
import unittest
import time
import logging
import threading
import socket
import paho.mqtt.client as mqtt

import itest.core

class MQTTConnector(object):
    """ Test MQTT connector
    """
    def __init__(self, name):
        self.name = name
        self.client = mqtt.Client(self.name)
        self.client.on_connect = self.__on_connect
        self.__connected_cv = threading.Condition()
        self.__connect_rc = None

    def connect(self):
        """ Connect to broker
        """
        self.client.connect("127.0.0.1")
        self.client.loop_start()

    def disconnect(self):
        """ Stop this subscriber
        """
        self.client.disconnect()
        self.client.loop_stop()

    def wait_until_connected(self, timeout_secs):
        """ Wait until connected
        """
        with self.__connected_cv:
            while self.__connect_rc is None:
                self.__connected_cv.wait(timeout_secs)
            connect_rc = self.__connect_rc
            self.__connect_rc = None
        return connect_rc

    def __on_connect(self, client, userdata, flags, return_code):
        """ Called when wrapped MQTT client connects
        """
        logging.info("[%s] CONNECTED: %s - %s - %s - %s",
                     self.name, client, userdata, flags, return_code)
        with self.__connected_cv:
            self.__connect_rc = return_code
            self.__connected_cv.notify_all()

class TCPConnector(object):
    """ TCP connector
    """
    def __init__(self):
        self.__socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def connect(self):
        """ Open tcp connection
        """
        self.__socket.connect(("127.0.0.1", 1883))

    def is_connected(self, timeout=1):
        """ Test if we are connected
        """
        try:
            with itest.core.Timeout(timeout):
                chunk = self.__socket.recv(1)
                if chunk == b'':
                    return False
                else:
                    return True
        except itest.core.TimeoutError:
            return True

    def disconnect(self):
        """ Close connection
        """
        try:
            self.__socket.shutdown(0)
            self.__socket.close()
        except OSError:
            pass

class MQTTConnectTests(unittest.TestCase):
    """ Integration test connecting to MQTT broker
    """

    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.ERROR)

        if not itest.core.SERVER_UNDER_TEST.is_started():
            itest.core.SERVER_UNDER_TEST.start()

    @classmethod
    def tearDownClass(cls):
        pass

    def setUp(self):
        self.mqtt_connector = MQTTConnector("MQTTConnector")

    def tearDown(self):
        self.mqtt_connector.disconnect()
        time.sleep(1)

    def test_connect_successful(self):
        """ Test that we can successfully connect to MQTT broker
        """
        self.mqtt_connector.connect()
        connect_rc = self.mqtt_connector.wait_until_connected(2)
        self.assertEqual(connect_rc, 0)

class TCPConnectTests(unittest.TestCase):
    """ Integration test connecting to MQTT broker
    """

    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.ERROR)

        if not itest.core.SERVER_UNDER_TEST.is_started():
            itest.core.SERVER_UNDER_TEST.start()

    @classmethod
    def tearDownClass(cls):
        pass

    def setUp(self):
        self.tcp_connector = TCPConnector()

    def tearDown(self):
        self.tcp_connector.disconnect()

    def test_connect_timeout(self):
        """ Test that new connection is closed after connect timeout
        """
        self.tcp_connector.connect()
        time.sleep(3) # Connect timeout is 2 s
        is_connected = self.tcp_connector.is_connected()
        self.assertFalse(is_connected)


if __name__ == '__main__':
    unittest.main()

