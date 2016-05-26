#!/usr/bin/python 


import time
import subprocess
import sys

import zmq

import unittest





class DPVATest(unittest.TestCase):
    
    def helpTestTypedAdapter(self, addrNoCB, addrCB, expectedGetCntr, expectedSetCntr):
        '''
        The actual setting of the property value and 0mq-peeking internal DOOCSsrv state.
        '''
        #---- doocsput 10 -> doocs_adapter_X, doocsput 20 -> doocs_adapter_X_cb ------
        try:
            doocsget_i = subprocess.check_output('doocsput -c "%s" -d 10' % addrNoCB, shell=True).strip()
            subprocess.check_output('doocsput -c "%s" -d 20' % addrCB  , shell=True)   # "swallows" output
        except subprocess.CalledProcessError as ex:
            self.fail("CalledProcessError - status %d from %s" % (ex.returncode, ex.cmd))
        #---- 0mq: peek doocs_adapter_I's val, test setting/getting, unblock ---------
        di_val = self.socket.recv().strip('\0')        # peek
        self.assertEqual(doocsget_i, '10')             # test property
        self.assertEqual(di_val    , '10')             # test adapterval
        self.socket.send(b"") # stub send              # unblock
        #---- 0mq: peek doocs_adapter_I_cb's counters, test, unblock -----------------
        dicb_g = self.socket.recv().strip('\0')        # peek            # a single doocsput rpc calls pattern is
        self.assertEqual(dicb_g, str(expectedGetCntr)) # test            #   get, set, get
        self.socket.send(b"") # stub send              # unblock
        dicb_s = self.socket.recv().strip('\0')        # peek
        self.assertEqual(dicb_s, str(expectedSetCntr)) # test
        self.socket.send(b"") # stub send              # unblock
        #-----------------------------------------------------------------------------
        

    def setUp(self):
        self.host             = '*'     # FIXME: to wszystko tez bedzie pewnie jakis config
        self.port             = 3497
        self.doocs_exec       = "dpvatestsrv_server"
        self.dspectrum_input  = "dspectrum_input1"   # data to set the DOOCS_array with

        # 0mq: prepare response socket
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REP)
        self.socket.bind("tcp://"+self.host+":" + str(self.port))

        # fire up doocs ---------------------------------------------------------
        print "[py: fire up doocs]"
        try:
            subprocess.check_call("./"+self.doocs_exec+ " &", shell=True)
        except subprocess.CalledProcessError as ex:
            print "CalledProcessError - status %d from %s" % (ex.returncode, ex.cmd)
            sys.exit()
        #------------------------------------------------------------------------

        # 0mq: get DOOCS pid ----------------------------------------------------
        self.doocs_pid = self.socket.recv().strip('\0')
        self.socket.send(b"") # stub send
        print "[py: server:", "doocs_pid =", self.doocs_pid, "]"
        #------------------------------------------------------------------------



    def testDPVA(self):
        '''
        test CS-side property setting behind DOOCS_adapter<int>
        '''
        adapterAddrNoCB = "TEST.DOOCS/LOCALHOST_610498009/DPVATESTSRVLOCATION/TESTTYPE_INT"
        adapterAddrCB   = "TEST.DOOCS/LOCALHOST_610498009/DPVATESTSRVLOCATION/TESTTYPE_INT_CB"
        expectedGetCntr = 2
        expectedSetCntr = 1
        self.helpTestTypedAdapter(adapterAddrNoCB, adapterAddrCB, expectedGetCntr, expectedSetCntr)

        '''
        test CS-side property setting behind DOOCS_adapter<float>
        '''
        adapterAddrNoCB = "TEST.DOOCS/LOCALHOST_610498009/DPVATESTSRVLOCATION/TESTTYPE_FLOAT"
        adapterAddrCB   = "TEST.DOOCS/LOCALHOST_610498009/DPVATESTSRVLOCATION/TESTTYPE_FLOAT_CB"
        expectedGetCntr = 4
        expectedSetCntr = 2
        self.helpTestTypedAdapter(adapterAddrNoCB, adapterAddrCB, expectedGetCntr, expectedSetCntr)

        '''
        test CS-side property setting behind DOOCS_adapter<double>
        '''
        adapterAddrNoCB = "TEST.DOOCS/LOCALHOST_610498009/DPVATESTSRVLOCATION/TESTTYPE_DOUBLE"
        adapterAddrCB   = "TEST.DOOCS/LOCALHOST_610498009/DPVATESTSRVLOCATION/TESTTYPE_DOUBLE_CB"
        expectedGetCntr = 6
        expectedSetCntr = 3
        self.helpTestTypedAdapter(adapterAddrNoCB, adapterAddrCB, expectedGetCntr, expectedSetCntr)

        '''
        test CS-side property setting behind DOOCS_adapter<string>
        '''
        adapterAddrNoCB = "TEST.DOOCS/LOCALHOST_610498009/DPVATESTSRVLOCATION/TESTTYPE_STRING"
        adapterAddrCB   = "TEST.DOOCS/LOCALHOST_610498009/DPVATESTSRVLOCATION/TESTTYPE_STRING_CB"
        expectedGetCntr = 8
        expectedSetCntr = 4
        self.helpTestTypedAdapter(adapterAddrNoCB, adapterAddrCB, expectedGetCntr, expectedSetCntr)


        #---- doocsput 10 -> doocs_adapter_X, doocsput 20 -> doocs_adapter_X_cb ------
        try:
            subprocess.check_output('doocsput -c "TEST.DOOCS/LOCALHOST_610498009/DPVATESTSRVLOCATION/TESTARRAY_INT" -a dspectrum_input1', shell=True)
            doocsget_last = subprocess.check_output('doocsget -a -c "TEST.DOOCS/LOCALHOST_610498009/DPVATESTSRVLOCATION/TESTARRAY_INT"', shell=True).strip().split()[-1]
        except subprocess.CalledProcessError as ex:
            self.fail("CalledProcessError - status %d from %s" % (ex.returncode, ex.cmd))
        #---- 0mq: peek doocs_adapter_I's val, test setting/getting, unblock ---------
        da_val = self.socket.recv().strip('\0')        # peek
        self.assertEqual(doocsget_last, '5')           # test property
        self.assertEqual(da_val    , '5')              # test adapterval        # FIXME: "5" originates from a file, so probably should not be hardcoded here
        self.socket.send(b"") # stub send              # unblock
        #---- 0mq: peek doocs_adapter_I_cb's counters, test, unblock -----------------
        dicb_g = self.socket.recv().strip('\0')        # peek            # a single doocsput rpc calls pattern is
        self.assertEqual(dicb_g, '8') # test           #                 #   get, set, get
        self.socket.send(b"") # stub send              # unblock
        dicb_s = self.socket.recv().strip('\0')        # peek            # a single doocsget rpc calls pattern is
        self.assertEqual(dicb_s, '4') # test                             #   get, get
        self.socket.send(b"") # stub send              # unblock
        #-----------------------------------------------------------------------------



    def tearDown(self):
        # kill doocs ------------------------------------------------------------
        print "[py: kill doocs]"
        try:
            subprocess.check_call("kill " + self.doocs_pid, shell=True)
        except subprocess.CalledProcessError as ex:
            print "CalledProcessError - status %d from %s" % (ex.returncode, ex.cmd)
            sys.exit()
        #------------------------------------------------------------------------

        time.sleep(1)





if __name__ == '__main__':
    unittest.main()

