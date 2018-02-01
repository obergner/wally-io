""" Test publishing retained messages
"""
import warnings
import unittest
import time
import logging

import itest.core

class DeleteRetainedMessagesQoS0Tests(unittest.TestCase):
    """ Test that deleting retained message by sending retained message of size 0 works
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

        self.subscriber_qos0 = itest.core.Subscriber("DeleteRetainedMessagesQoS0Tests-Sub0",
                                                     "/test/retained/delete/qos0/#", 0)
        self.subscriber_qos0.start()

        self.subscriber_qos1 = itest.core.Subscriber("DeleteRetainedMessagesQoS0Tests-Sub1",
                                                     "/test/retained/delete/qos0/#", 1)

        self.publisher = itest.core.Publisher("DeleteRetainedMessagesQoS0Tests-Pub")
        self.publisher.start()
        time.sleep(1)

    def tearDown(self):
        self.publisher.stop()
        self.subscriber_qos0.stop()
        self.subscriber_qos1.stop()
        time.sleep(1)

    def test_retained_publish_forwarded_without_retained_flag_qos0(self):
        """ Test that a PUBLISH packet WITH retained flag set and messages size 0 is forwarded WITHOUT retained flag to ALREADY EXISTING subscribers BUT deletes previously stored retained message """
        self.publisher.publish("/test/retained/delete/qos0/test-messages", "test_delete_retained_qos0", 0, True)

        msg1 = self.subscriber_qos0.wait_for_message(2)
        self.assertIsNotNone(msg1)
        self.assertEqual(msg1.payload, b'test_delete_retained_qos0')
        self.assertEqual(msg1.qos, 0)
        self.assertEqual(msg1.retain, False)

        self.publisher.publish("/test/retained/delete/qos0/test-messages", "", 0, True)

        msg2 = self.subscriber_qos0.wait_for_message(2)
        self.assertIsNotNone(msg2)
        self.assertEqual(msg2.payload, b'')
        self.assertEqual(msg2.qos, 0)
        self.assertEqual(msg2.retain, False)

        self.subscriber_qos1.start()
        msg3 = self.subscriber_qos1.wait_for_message(2)
        self.assertIsNone(msg3)


if __name__ == '__main__':
    unittest.main()
