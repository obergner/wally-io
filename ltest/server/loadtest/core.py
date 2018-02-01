""" Loadtest core classes
"""
import signal
import os
import subprocess
import threading
import time
import logging
import paho.mqtt.client as mqtt

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
        config = os.environ['CONFIG']
        executable = "./target/%s/main/wally-iod" % (config)
        log_file = './target/%s/ltest/ltest_server.log' % (config)
        # See: https://stackoverflow.com/questions/42851670/how-to-generate-core-dump-on-addresssanitizer-error
        process_env = {'ASAN_OPTIONS': 'abort_on_error=1:disable_coredump=0:unmap_shadow_on_exit=1'}
        args = [executable,
                '--log-file', log_file,
                '--log-level', 'trace',
                '--log-console',
                '--conn-timeout', '2000']
        self.process = subprocess.Popen(args, env=process_env)
        time.sleep(2)
        if self.process.poll() is not None:
            raise RuntimeError("Failed to start %s" % (self.name))
        logging.info("Started %s", self.name)

    def stop(self):
        """ Stop WallyIO subprocess
        """
        logging.info("Stopping %s ...", self.name)
        self.process.send_signal(signal.SIGTERM)
        time.sleep(1)
        if self.process.poll() is None:
            self.process.send_signal(signal.SIGKILL)
        self.process.wait()
        logging.info("Stopped %s", self.name)

class Subscriber(threading.Thread):
    """ Test MQTT subscriber
    """
    def __init__(self, idx, qos=0, topic_pattern="#"):
        threading.Thread.__init__(self)
        self.idx = idx
        self.qos = qos
        self.topic_pattern = topic_pattern
        self.name = "Subscriber-%s(qos:%s)" % (idx, qos)
        self.client = mqtt.Client(self.name)
        self.client.on_connect = self.__on_connect
        self.client.on_message = self.__on_message

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


class Publisher(threading.Thread):
    """ Test MQTT publisher
    """
    def __init__(self, idx, publish_count=100, qos=0, topic="/test"):
        threading.Thread.__init__(self)
        self.idx = idx
        self.qos = qos
        self.topic = topic
        self.name = "Publisher-%s(qos:%s)" % (idx, qos)
        self.client = mqtt.Client(self.name)
        self.client.on_connect = self.__on_connect
        self.client.on_publish = self.__on_publish
        self.__expected = publish_count
        self.__messages = ["[%s] Message %s" % (self.name, mi) for mi in range(publish_count)]

    def run(self):
        self.__start()

    def __start(self):
        """ Start this publisher
        """
        self.client.connect("127.0.0.1")
        self.client.loop_forever()

    def __on_connect(self, client, userdata, flags, return_code):
        """ Called when client receives CONNACK
        """
        if return_code != 0:
            logging.error("[%s] CONNECTING FAILED: rc = %s", self.name, return_code)
            raise RuntimeError("Connecting failed")

        logging.info("[%s] CONNECTED: %s - %s - %s - %s",
                     self.name, client, userdata, flags, return_code)
        for msg in self.__messages:
            self.__publish(msg)

    def __publish(self, msg):
        """ Publish a message
        """
        (result, mid) = self.client.publish(self.topic, msg, self.qos)
        if result != mqtt.MQTT_ERR_SUCCESS:
            logging.error("Failed to publish message: result = %s", result)
            raise RuntimeError("Failed to publish message: result = %s" % result)
        logging.info("[%s] Sent message '%s': mid = %s", self.name, msg, mid)

    def __on_publish(self, client, userdata, mid):
        """ Called when publish completed successfully
        """
        logging.info("[%s] Successfully published message: mid = %s (client:%s|userdata:%s)",
                     self.name, mid, client, userdata)
        self.__expected -= 1
        if self.__expected == 0:
            logging.info("[%s] All messages have been published - shutting down", self.name)
            self.client.disconnect()

