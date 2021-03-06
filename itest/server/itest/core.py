""" Integration test core classes
"""
import atexit
import logging
import os
import signal
import subprocess
import time
import threading
import paho.mqtt.client as mqtt

class OperationTimeoutError(Exception):
    """ Timeout exception
    """
    pass

class Timeout(object):
    """ Timeout
    """
    def __init__(self, seconds=1):
        self.secs = seconds

    def handle_timeout(self, signum, frame):
        """ Callback
        """
        raise OperationTimeoutError("Timeout after [%s] seconds [signum: %s|frame: %s]" % (self.secs, signum, frame))

    def seconds(self):
        """ Returns seconds
        """
        return self.seconds

    def __enter__(self):
        signal.signal(signal.SIGALRM, self.handle_timeout)
        signal.alarm(self.secs)

    def __exit__(self, kind, value, traceback):
        signal.alarm(0)

class ServerUnderTest(object):
    """ Wraps process running WallyIO
    """
    def __init__(self, name):
        self.__name = name
        self.__process = None

    def start(self):
        """ Start WallyIO subprocess
        """
        config = os.environ['CONFIG']
        executable = './target/%s/main/wally-iod' % (config)
        log_file = './target/%s/itest/itest_server.log' % (config)
        # See: https://stackoverflow.com/questions/42851670/how-to-generate-core-dump-on-addresssanitizer-error
        process_env = {'ASAN_OPTIONS': 'abort_on_error=1:disable_coredump=0:unmap_shadow_on_exit=1'}
        logging.info("Starting %s ...", self.__name)
        args = [executable,
                '--log-file', log_file,
                '--log-level', 'trace',
                '--conn-timeout', '2000']
        self.__process = subprocess.Popen(args, env=process_env)
        time.sleep(2)
        if self.__process.poll() is not None:
            raise RuntimeError("Failed to start %s" % (self.__name))
        logging.info("Started %s", self.__name)

    def is_started(self):
        """ Test whether this server is started
        """
        return self.__process and self.__process.poll() is None

    def stop(self):
        """ Stop WallyIO subprocess
        """
        logging.info("Stopping %s ...", self.__name)
        self.__process.send_signal(signal.SIGTERM)
        time.sleep(1)
        if self.__process.poll() is None:
            self.__process.send_signal(signal.SIGKILL)
        self.__process.wait()
        logging.info("Stopped %s", self.__name)

class Subscriber(object):
    """ Test MQTT subscriber
    """
    def __init__(self, name, topic_pattern="#", qos=0):
        self.name = name
        self.topic_pattern = topic_pattern
        self.qos = qos
        self.client = mqtt.Client(self.name)
        self.client.on_connect = self.__on_connect
        self.client.on_message = self.__on_message
        self.__message_received_cv = threading.Condition()
        self.__received_message = None

    def start(self):
        """ Start this subscriber
        """
        self.client.connect("127.0.0.1")
        self.client.loop_start()

    def stop(self):
        """ Stop this subscriber
        """
        self.client.disconnect()
        self.client.loop_stop()

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

class Publisher(object):
    """ Test MQTT publisher
    """
    def __init__(self, name, will_topic=None, will_message=None, will_qos=0, will_retain=False):
        self.name = name
        self.client = mqtt.Client(self.name)
        if will_topic is not None:
            self.client.will_set(will_topic, will_message, will_qos, will_retain)
        self.client.on_connect = self.__on_connect
        self.client.on_publish = self.__on_publish

    def start(self):
        """ Start this publisher
        """
        self.client.connect("127.0.0.1")
        self.client.loop_start()

    def stop(self):
        """ Stop this publisher
        """
        self.client.disconnect()
        self.client.loop_stop()

    def __on_connect(self, client, userdata, flags, return_code):
        """ Called when client receives CONNACK
        """
        if return_code != 0:
            logging.error("[%s] CONNECTING FAILED: rc = %s", self.name, return_code)
            raise RuntimeError("Connecting failed")

        logging.info("[%s] CONNECTED: %s - %s - %s - %s",
                     self.name, client, userdata, flags, return_code)

    def publish(self, topic, msg, qos=0, retain=False):
        """ Publish a message
        """
        (result, mid) = self.client.publish(topic, msg, qos, retain)
        if result != mqtt.MQTT_ERR_SUCCESS:
            logging.error("Failed to publish message: result = %s", result)
            raise RuntimeError("Failed to publish message: result = %s" % result)
        logging.info("[%s] Sent message '%s': mid = %s", self.name, msg, mid)

    def __on_publish(self, client, userdata, mid):
        """ Called when publish completed successfully
        """
        logging.info("[%s] Successfully published message: mid = %s (client:%s|userdata:%s)",
                     self.name, mid, client, userdata)

    def disconnect_ungracefully(self):
        """ Force connection close without sending a DISCONNECT
        """
        self.client._sock.close()

SERVER_UNDER_TEST = ServerUnderTest("ServerUnderTest")

def shutdown_server():
    """ Shutdown server
    """
    SERVER_UNDER_TEST.stop()

atexit.register(shutdown_server)
