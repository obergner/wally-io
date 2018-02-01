""" Test unsubscribing from topics
"""
import warnings
import unittest
import time
import logging

import itest.core

class BasicPublishSubscribeTests(unittest.TestCase):
    """ Test that publishing and subscribing to a topic basically works.
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
        warnings.filterwarnings("ignore", category=ResourceWarning)

        self.subscriber_qos0 = itest.core.Subscriber("BasicPublishSubscribeTests-Sub0", "/test/publish/#", 0)
        self.subscriber_qos0.start()
        self.subscriber_qos1 = itest.core.Subscriber("BasicPublishSubscribeTests-Sub1", "/test/publish/#", 1)
        self.subscriber_qos1.start()
        self.subscriber_qos2 = itest.core.Subscriber("BasicPublishSubscribeTests-Sub2", "/test/publish/#", 2)
        self.subscriber_qos2.start()
        self.publisher = itest.core.Publisher("BasicPublishSubscribeTests-Pub")
        self.publisher.start()
        time.sleep(1)

    def tearDown(self):
        self.publisher.stop()
        self.subscriber_qos0.stop()
        self.subscriber_qos1.stop()
        self.subscriber_qos2.stop()
        time.sleep(1)

    def test_publish_qos0_sub_qos0(self):
        """ Test publishing with QoS 0 / subscribing with QoS 0
        """
        self.publisher.publish("/test/publish/qos0", "test_publish_qos0", 0)
        msg = self.subscriber_qos0.wait_for_message(2)
        self.assertIsNotNone(msg)
        self.assertEqual(msg.payload, b'test_publish_qos0')
        self.assertEqual(msg.qos, 0)

    def test_publish_qos1_sub_qos0(self):
        """ Test publishing with QoS 1 / subscribing with QoS 0
        """
        self.publisher.publish("/test/publish/qos1", "test_publish_qos1", 1)
        msg = self.subscriber_qos0.wait_for_message(2)
        self.assertIsNotNone(msg)
        self.assertEqual(msg.payload, b'test_publish_qos1')
        self.assertEqual(msg.qos, 0)

    def test_publish_qos2_sub_qos0(self):
        """ Test publishing with QoS 2 / subscribing with QoS 0
        """
        self.publisher.publish("/test/publish/qos2", "test_publish_qos2", 2)
        msg = self.subscriber_qos0.wait_for_message(2)
        self.assertIsNotNone(msg)
        self.assertEqual(msg.payload, b'test_publish_qos2')
        self.assertEqual(msg.qos, 0)

    def test_publish_qos0_sub_qos1(self):
        """ Test publishing with QoS 0 / subscribing with QoS 1
        """
        self.publisher.publish("/test/publish/qos0", "test_publish_qos0", 0)
        msg = self.subscriber_qos1.wait_for_message(2)
        self.assertIsNotNone(msg)
        self.assertEqual(msg.payload, b'test_publish_qos0')
        self.assertEqual(msg.qos, 1)

    def test_publish_qos1_sub_qos1(self):
        """ Test publishing with QoS 1 / subscribing with QoS 1
        """
        self.publisher.publish("/test/publish/qos1", "test_publish_qos1", 1)
        msg = self.subscriber_qos1.wait_for_message(2)
        self.assertIsNotNone(msg)
        self.assertEqual(msg.payload, b'test_publish_qos1')
        self.assertEqual(msg.qos, 1)

    def test_publish_qos2_sub_qos1(self):
        """ Test publishing with QoS 2 / subscribing with QoS 1
        """
        self.publisher.publish("/test/publish/qos2", "test_publish_qos2", 2)
        msg = self.subscriber_qos1.wait_for_message(2)
        self.assertIsNotNone(msg)
        self.assertEqual(msg.payload, b'test_publish_qos2')
        self.assertEqual(msg.qos, 1)

    def test_publish_qos0_sub_qos2(self):
        """ Test publishing with QoS 0 / subscribing with QoS 2
        """
        self.publisher.publish("/test/publish/qos0", "test_publish_qos0", 0)
        msg = self.subscriber_qos2.wait_for_message(2)
        self.assertIsNotNone(msg)
        self.assertEqual(msg.payload, b'test_publish_qos0')
        self.assertEqual(msg.qos, 2)

    def test_publish_qos1_sub_qos2(self):
        """ Test publishing with QoS 1 / subscribing with QoS 2
        """
        self.publisher.publish("/test/publish/qos1", "test_publish_qos1", 1)
        msg = self.subscriber_qos2.wait_for_message(2)
        self.assertIsNotNone(msg)
        self.assertEqual(msg.payload, b'test_publish_qos1')
        self.assertEqual(msg.qos, 2)

    def test_publish_qos2_sub_qos2(self):
        """ Test publishing with QoS 2 / subscribing with QoS 2
        """
        self.publisher.publish("/test/publish/qos2", "test_publish_qos2", 2)
        msg = self.subscriber_qos2.wait_for_message(2)
        self.assertIsNotNone(msg)
        self.assertEqual(msg.payload, b'test_publish_qos2')
        self.assertEqual(msg.qos, 2)


if __name__ == '__main__':
    unittest.main()
