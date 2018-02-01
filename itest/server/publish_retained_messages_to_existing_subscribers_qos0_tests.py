""" Test publishing retained messages
"""
import warnings
import unittest
import time
import logging

import itest.core

class PublishRetainedMessagesToExistingSubscribersQoS0Tests(unittest.TestCase):
    """ Test that publishing retained messages with QoS0 to existing subscribers works.
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

        self.subscriber_qos0 = itest.core.Subscriber("PublishRetainedMessagesQoS0Tests-Sub0",
                                                     "/test/retained/qos0/#", 0)
        self.subscriber_qos0.start()
        self.subscriber_qos1 = itest.core.Subscriber("PublishRetainedMessagesQoS0Tests-Sub1",
                                                     "/test/retained/qos0/#", 1)
        self.subscriber_qos1.start()
        self.subscriber_qos2 = itest.core.Subscriber("PublishRetainedMessagesQoS0Tests-Sub2",
                                                     "/test/retained/qos0/#", 2)
        self.subscriber_qos2.start()
        self.publisher = itest.core.Publisher("PublishRetainedMessagesQoS0Tests-Pub")
        self.publisher.start()
        time.sleep(1)

    def tearDown(self):
        self.publisher.stop()
        self.subscriber_qos0.stop()
        self.subscriber_qos1.stop()
        self.subscriber_qos2.stop()
        time.sleep(1)

    def test_retained_publish_forwarded_without_retained_flag_qos0(self):
        """ Test that a PUBLISH packet WITH retained flag set is forwarded WITHOUT retained flag to ALREADY EXISTING subscribers when that PUBLISH packet is published using QoS 0 """
        self.publisher.publish("/test/retained/qos0/test-messages", "test_retained_qos0", 0, True)
        msg = self.subscriber_qos0.wait_for_message(2)
        self.assertIsNotNone(msg)
        self.assertEqual(msg.payload, b'test_retained_qos0')
        self.assertEqual(msg.qos, 0)
        self.assertEqual(msg.retain, False)


if __name__ == '__main__':
    unittest.main()
