""" Test publishing retained messages
"""
import warnings
import unittest
import time
import logging

import itest.core

class PublishRetainedMessagesToLaterSubscribersQoS1Tests(unittest.TestCase):
    """ Test that publishing retained messages with QoS1 to later subscribers works.
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

        self.subscriber_qos0 = itest.core.Subscriber("PublishRetainedMessagesToLaterSubscribersQoS1Tests-Sub0",
                                                     "/test/retained/later/qos1/#", 0)
        self.subscriber_qos1 = itest.core.Subscriber("PublishRetainedMessagesToLaterSubscribersQoS1Tests-Sub1",
                                                     "/test/retained/later/qos1/#", 1)
        self.subscriber_qos2 = itest.core.Subscriber("PublishRetainedMessagesToLaterSubscribersQoS1Tests-Sub2",
                                                     "/test/retained/later/qos1/#", 2)
        self.publisher = itest.core.Publisher("PublishRetainedMessagesQoS1Tests-Pub")
        self.publisher.start()
        time.sleep(1)

    def tearDown(self):
        self.publisher.stop()
        self.subscriber_qos0.stop()
        self.subscriber_qos1.stop()
        self.subscriber_qos2.stop()
        time.sleep(1)

    def test_retained_publish_forwarded_without_retained_flag_qos1(self):
        """ Test that a PUBLISH packet WITH retained flag set is forwarded WITH retained flag to LATER/NEW subscribers when that PUBLISH packet is published using QoS 1 """
        self.publisher.publish("/test/retained/later/qos1/test-messages", "test_retained_qos1", 0, True)

        self.subscriber_qos0.start()
        self.subscriber_qos1.start()
        self.subscriber_qos2.start()

        msg1 = self.subscriber_qos0.wait_for_message(2)
        self.assertIsNotNone(msg1)
        self.assertEqual(msg1.payload, b'test_retained_qos1')
        self.assertEqual(msg1.qos, 0)
        self.assertEqual(msg1.retain, True)

        msg2 = self.subscriber_qos1.wait_for_message(2)
        self.assertIsNotNone(msg2)
        self.assertEqual(msg2.payload, b'test_retained_qos1')
        self.assertEqual(msg2.qos, 1)
        self.assertEqual(msg2.retain, True)

        msg3 = self.subscriber_qos2.wait_for_message(2)
        self.assertIsNotNone(msg3)
        self.assertEqual(msg3.payload, b'test_retained_qos1')
        self.assertEqual(msg3.qos, 2)
        self.assertEqual(msg3.retain, True)


if __name__ == '__main__':
    unittest.main()
