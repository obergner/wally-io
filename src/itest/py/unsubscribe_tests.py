""" Test unsubscribing from topics
"""
import unittest
import time
import logging
import threading
import paho.mqtt.client as mqtt

import itest.core

class Subscriber(threading.Thread):
    """ Test MQTT subscriber
    """
    def __init__(self, name, topic_pattern="#", qos=0):
        threading.Thread.__init__(self)
        self.name = name
        self.topic_pattern = topic_pattern
        self.qos = qos
        self.client = mqtt.Client(self.name)
        self.client.on_connect = self.__on_connect
        self.client.on_message = self.__on_message
        self.__message_received_cv = threading.Condition()
        self.__received_message = None

    def run(self):
        self.__start()

    def __start(self):
        """ Start this subscriber
        """
        self.client.connect("127.0.0.1")
        self.client.loop_forever()

    def stop(self):
        """ Stop this subscriber
        """
        self.client.disconnect()

    def wait_for_message(self, timeout_secs):
        """ Wait for an incoming message
        """
        with self.__message_received_cv:
            self.__message_received_cv.wait(timeout_secs)
            msg = self.__received_message
            self.__received_message = None
        return msg

    def unsubscribe(self):
        """ Unsubscribe
        """
        self.client.unsubscribe(self.topic_pattern)

    def __on_connect(self, client, userdata, flags, return_code):
        """ Called when wrapped MQTT client connects
        """
        if return_code != 0:
            logging.error("[%s] CONNECTING FAILED: rc = %s", self.name, return_code)
            raise RuntimeError("Connecting failed")

        logging.info("[%s] CONNECTED: %s - %s - %s - %s",
                     self.name, client, userdata, flags, return_code)
        self.client.subscribe(self.topic_pattern, self.qos)

    def __on_message(self, client, userdata, msg):
        """ Called whenever a message is received
        """
        logging.info("[%s] RCVD: (%s) %s/%s (client:%s)",
                     self.name, userdata, msg.topic, str(msg.payload), client)
        with self.__message_received_cv:
            self.__received_message = msg
            self.__message_received_cv.notify_all()

class Publisher(threading.Thread):
    """ Test MQTT publisher
    """
    def __init__(self, name):
        threading.Thread.__init__(self)
        self.name = name
        self.client = mqtt.Client(self.name)
        self.client.on_connect = self.__on_connect
        self.client.on_publish = self.__on_publish

    def run(self):
        self.__start()

    def __start(self):
        """ Start this publisher
        """
        self.client.connect("127.0.0.1")
        self.client.loop_forever()

    def stop(self):
        """ Stop this publisher
        """
        self.client.disconnect()

    def __on_connect(self, client, userdata, flags, return_code):
        """ Called when client receives CONNACK
        """
        if return_code != 0:
            logging.error("[%s] CONNECTING FAILED: rc = %s", self.name, return_code)
            raise RuntimeError("Connecting failed")

        logging.info("[%s] CONNECTED: %s - %s - %s - %s",
                     self.name, client, userdata, flags, return_code)

    def publish(self, topic, msg, qos=0):
        """ Publish a message
        """
        (result, mid) = self.client.publish(topic, msg, qos)
        if result != mqtt.MQTT_ERR_SUCCESS:
            logging.error("Failed to publish message: result = %s", result)
            raise RuntimeError("Failed to publish message: result = %s" % result)
        logging.info("[%s] Sent message '%s': mid = %s", self.name, msg, mid)

    def __on_publish(self, client, userdata, mid):
        """ Called when publish completed successfully
        """
        logging.info("[%s] Successfully published message: mid = %s (client:%s|userdata:%s)",
                     self.name, mid, client, userdata)


class UnsubscribeTests(unittest.TestCase):
    """ Integration test unsubscribing from topics.
    """

    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.ERROR)

        cls.__under_test = itest.core.ServerUnderTest("UnsubscribeTests")
        cls.__under_test.start()
        time.sleep(1)

    @classmethod
    def tearDownClass(cls):
        cls.__under_test.stop()

    def setUp(self):
        self.subscriber = Subscriber("subscriber", "/test/unsubscribe", 0)
        self.subscriber.start()
        self.publisher = Publisher("publisher")
        self.publisher.start()
        time.sleep(1)

    def tearDown(self):
        self.publisher.stop()
        self.subscriber.stop()
        self.publisher.join()
        self.subscriber.join()

    def test_unsubscribe(self):
        """ Test unsubscribe
        """
        self.publisher.publish("/test/unsubscribe", "Unsubscribe test", 0)
        msg = self.subscriber.wait_for_message(2)
        self.assertIsNotNone(msg)

        self.subscriber.unsubscribe()
        time.sleep(1)
        self.publisher.publish("/test/unsubscribe", "Unsubscribe test 2", 0)
        msg = self.subscriber.wait_for_message(2)
        self.assertIsNone(msg)


if __name__ == '__main__':
    unittest.main()

