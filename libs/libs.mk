#######################################################################################################################
# Download and build support libraries that we do not have native (RPM) packages for
#######################################################################################################################

# -------------------------------------------------------------------------------- 
# Common
# -------------------------------------------------------------------------------- 

# http://stackoverflow.com/questions/18136918/how-to-get-current-directory-of-your-makefile
CWD                             := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

# -------------------------------------------------------------------------------- 
# Paho-C MQTT client library (Eclipse) used in integration tests
# -------------------------------------------------------------------------------- 

PAHO_C_URL                      := http://download.eclipse.org/paho/1.1/eclipse-paho-mqtt-c-unix-1.0.3.tar.gz
PAHO_C_DIR                      := $(CWD)/org.eclipse.paho.mqtt.c
PAHO_C_LIBS                     := $(PAHO_C_DIR)/lib
PAHO_C_INC                      := $(PAHO_C_DIR)/include

# -------------------------------------------------------------------------------- 
# Boost Asio Queue Extension by Hans Ewetz
# -------------------------------------------------------------------------------- 

BA_QUEUE_EXT_DIR                := $(CWD)/boost-asio-queue-extension
BA_QUEUE_EXT_INC                := $(BA_QUEUE_EXT_DIR)

#######################################################################################################################
# Rules
#######################################################################################################################

paho-c                          : $(PAHO_C_DIR)

$(PAHO_C_DIR)                   : 
	@mkdir $(PAHO_C_DIR) > /dev/null 2>&1; \
		pushd $(PAHO_C_DIR) > /dev/null; \
		echo "Downloading paho c library from: " $(PAHO_C_URL); \
		curl --progress-bar $(PAHO_C_URL) | tar -xz; \
		echo "[DONE]"; \
		popd > /dev/null;

