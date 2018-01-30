The tests included in the subdirectories check for functionality and
conformance of the Qt MQTT module.

To be able to run the tests successfully, a broker needs to be available and
reachable.

The continuous integration utilized the paho conformance test broker. It can
be obtained at this location:
https://github.com/eclipse/paho.mqtt.testing

For the unit tests being able to locate this script, use the
MQTT_TEST_BROKER_LOCATION environment variable and set it
to “<install-path>/interoperability/startbroker.py”.

Alternatively, any broker can be instantiated and passed to the auto tests
by using the environment variable MQTT_TEST_BROKER. This must point to a
valid url. The broker must run on the standardized port 1883.

Note, that the unit tests verify functionality against the MQTT 3.1.1 version
of the standard, hence the broker needs to be compliant to the adherent
specifications.
