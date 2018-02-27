""" Test publishing LWT messages
"""
import warnings
import unittest
import time
import logging

import itest.core

class PublishUnretainedLWTMessagesToExistingSubscribersQoS2Tests(unittest.TestCase):
    """ Test that publishing unretained LWT messages with QoS2 to existing subscribers works.
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

        self.subscriber_qos0 = itest.core.Subscriber("PublishUnretainedLWTMessagesToExistingSubscribersQoS2Tests-Sub0",
                                                     "/test/unretained/lwt/qos2/#", 0)
        self.subscriber_qos0.start()
        self.subscriber_qos1 = itest.core.Subscriber("PublishUnretainedLWTMessagesToExistingSubscribersQoS2Tests-Sub1",
                                                     "/test/unretained/lwt/qos2/#", 1)
        self.subscriber_qos1.start()
        self.subscriber_qos2 = itest.core.Subscriber("PublishUnretainedLWTMessagesToExistingSubscribersQoS2Tests-Sub2",
                                                     "/test/unretained/lwt/qos2/#", 2)
        self.subscriber_qos2.start()
        self.publisher = itest.core.Publisher("PublishUnretainedLWTMessagesQoS2Tests-Pub",
                                              "/test/unretained/lwt/qos2/1", b'LWT', 0, False)
        self.publisher.start()
        time.sleep(1)

    def tearDown(self):
        self.publisher.stop()
        self.subscriber_qos0.stop()
        self.subscriber_qos1.stop()
        self.subscriber_qos2.stop()
        time.sleep(1)

    def test_lwt_msg_published_on_ungraceful_disconnect_qos2(self):
        """ Test that an unretained LWT message is published to subscribers if client disconnects ungracefully using QoS 2 """
        self.publisher.disconnect_ungracefully()

        msg1 = self.subscriber_qos0.wait_for_message(2)
        self.assertIsNotNone(msg1)
        self.assertEqual(msg1.payload, b'LWT')
        self.assertEqual(msg1.qos, 0)
        self.assertEqual(msg1.retain, False)

        msg2 = self.subscriber_qos1.wait_for_message(2)
        self.assertIsNotNone(msg2)
        self.assertEqual(msg2.payload, b'LWT')
        self.assertEqual(msg2.qos, 1)
        self.assertEqual(msg2.retain, False)

        msg3 = self.subscriber_qos2.wait_for_message(2)
        self.assertIsNotNone(msg3)
        self.assertEqual(msg3.payload, b'LWT')
        self.assertEqual(msg3.qos, 2)
        self.assertEqual(msg3.retain, False)


if __name__ == '__main__':
    unittest.main()
