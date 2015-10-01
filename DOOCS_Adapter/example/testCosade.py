#!/usr/bin/python2

# The doocs python tools are currently only available from a network drive,
# so we can as well hard-code the path
import sys
sys.path.append("/home/ttflinac/user/python-2.7/Debian/")

import doocs
import unittest
import time

class CosadeTest(unittest.TestCase):

    def setUp(self):
        self.__location__ = "TEST.DOOCS/LOCALHOST_610498009/COSADELOCATION/"
        print("FIXME: starting the doocs server should happen here. Do it manually to run this test.")

    def tearDown(self):
        print("FIXME: stopping the doocs server should happen here")

    # After some time then monitor voltage should reflect target voltage.
    # As update is only called every second, this can take some time
    # (up to a second plus safety factor = two seconds).
    # To decrase the latency we check every 100 ms and sleep if the value
    # is not there yet.
    # Afterwards check that the property has arrived (might still be 
    def waitForValueOnProperty(self, property_name, expected_value):
        # 20 tries of 0.1 second are 2 seconds
        for i in range(20):
            # if the read value is correct break the loop and return
            if doocs.read(self.__location__ + property_name) == expected_value:
                break
            # Otherwise sleep and retry (after the last sleep there still 
            # is a read in the evaluation)
            time.sleep(0.1)
        self.assertEqual(doocs.read(self.__location__ + property_name),expected_value)

    def testWriteAndReadBack(self):
        # set the target voltage to 0
        doocs.write(self.__location__ + "TARGET_VOLTAGE","0")
        self.waitForValueOnProperty("MONITOR_VOLTAGE",0)
        # now change it and recheck (it could have been 0 in the first place)
        doocs.write(self.__location__ + "TARGET_VOLTAGE","42")
        self.waitForValueOnProperty("MONITOR_VOLTAGE",42)

if __name__ == '__main__':
    unittest.main()
