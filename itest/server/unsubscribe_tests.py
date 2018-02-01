""" Test unsubscribing from topics
"""
import warnings
import unittest
import time
import logging

import itest.core

class UnsubscribeTests(unittest.TestCase):
    """ Test that unsubscribing from topics works.
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

        self.subscriber = itest.core.Subscriber("UnsubscribeTests-Sub", "/test/unsubscribe", 0)
        self.subscriber.start()
        self.publisher = itest.core.Publisher("UnsubscribeTests-Pub")
        self.publisher.start()
        time.sleep(1)

    def tearDown(self):
        self.publisher.stop()
        self.subscriber.stop()
        time.sleep(1)

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
