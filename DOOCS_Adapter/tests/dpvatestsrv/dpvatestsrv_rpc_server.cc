#include        "eq_dpvatestsrv.h"
#include        "eq_sts_codes.h"
#include        "eq_fct_errors.h"



////////////////////////////////////////////////////---<>
                                                  //
char*   object_name = "DpvatestsrvServer";

EqFct * eq_create (int eq_code, void *)
{
   EqFct* eqn;
   switch (eq_code) {
      case CodeDpvatestsrv:
         eqn =  new EqFctDpvatestsrv ();
         break;
      default:
         eqn = (EqFct *) 0;
         break;
   }
   return eqn;
}

void eq_init_prolog () {}
void eq_init_epilog () {}

void refresh_prolog () {}
void refresh_epilog () {}

void interrupt_usr1_prolog(int)  {}
void interrupt_usr2_prolog(void) {}
void interrupt_usr1_epilog(int)  {}
void interrupt_usr2_epilog(void) {}
void post_init_prolog(void)      {}
void post_init_epilog(void)      {}
void eq_cancel(void)             {}
                                                  //
////////////////////////////////////////////////////---<>



EqFctDpvatestsrv::EqFctDpvatestsrv ( ) :        
    EqFct              ("NAME = location" )
,   DOOCSPVFactory     (this)
,   doocs_adapter_I    (DOOCSPVFactory.getProcessVariable<int        >("TESTTYPE_INT"))
,   doocs_adapter_I_cb (DOOCSPVFactory.getProcessVariable<int        >("TESTTYPE_INT_CB"))
,   doocs_adapter_F    (DOOCSPVFactory.getProcessVariable<float      >("TESTTYPE_FLOAT"))
,   doocs_adapter_F_cb (DOOCSPVFactory.getProcessVariable<float      >("TESTTYPE_FLOAT_CB"))
,   doocs_adapter_D    (DOOCSPVFactory.getProcessVariable<double     >("TESTTYPE_DOUBLE"))
,   doocs_adapter_D_cb (DOOCSPVFactory.getProcessVariable<double     >("TESTTYPE_DOUBLE_CB"))
,   doocs_adapter_S    (DOOCSPVFactory.getProcessVariable<std::string>("TESTTYPE_STRING"))
,   doocs_adapter_S_cb (DOOCSPVFactory.getProcessVariable<std::string>("TESTTYPE_STRING_CB"))
,   doocs_array_I      (DOOCSPVFactory.getProcessArray<int   >        ("TESTARRAY_INT", ARRAY_SIZE))
,   doocs_array_I_cb   (DOOCSPVFactory.getProcessArray<int   >        ("TESTARRAY_INT_CB", ARRAY_SIZE))
,   testnr(0)
{
    // wear callbacks
    doocs_adapter_I_cb->setOnSetCallbackFunction(boost::bind (&on_set_callback<int        >, _1, _2));
    doocs_adapter_I_cb->setOnGetCallbackFunction(boost::bind (&on_get_callback<int        >));
    doocs_adapter_F_cb->setOnSetCallbackFunction(boost::bind (&on_set_callback<float      >, _1, _2));
    doocs_adapter_F_cb->setOnGetCallbackFunction(boost::bind (&on_get_callback<float      >));
    doocs_adapter_D_cb->setOnSetCallbackFunction(boost::bind (&on_set_callback<double     >, _1, _2));
    doocs_adapter_D_cb->setOnGetCallbackFunction(boost::bind (&on_get_callback<double     >));
    doocs_adapter_S_cb->setOnSetCallbackFunction(boost::bind (&on_set_callback<std::string>, _1, _2));
    doocs_adapter_S_cb->setOnGetCallbackFunction(boost::bind (&on_get_callback<std::string>));
    doocs_array_I_cb  ->setOnSetCallbackFunction(boost::bind (&on_arrayset_callback<int>, _1));
    doocs_array_I_cb  ->setOnGetCallbackFunction(boost::bind (&on_arrayget_callback<int>, _1));
    // 0mq: connect to the controller
	std::string proto = "tcp";
	std::string host  = "localhost";
	std::string port  = SSTR(PORT);
	std::string url = proto + "://" + host +":" +port;
    zmqsocket.connect (url.c_str());
}



void EqFctDpvatestsrv::init ( )
{
    // 0mq: transmit process PID to the controller, so that the controller can kill the DOOCS srv in the end
    send(getpid());
}


void    EqFctDpvatestsrv::update ( )
{
    switch (testnr++) {
    case 0: // DOOCS_adapter<int>
    {
        // 0mq: value ==> controller
        send(doocs_adapter_I->getWithoutCallback());
        // 0mq: cb_counters ==> controller
        send(_get_cb_counter);
        send(_set_cb_counter);
        break;
    }
    case 1: // DOOCS_adapter<float>
    {
        // 0mq: value ==> controller
        send(doocs_adapter_F->getWithoutCallback());
        // 0mq: cb_counters ==> controller
        send(_get_cb_counter);
        send(_set_cb_counter);
        break;
    }
    case 2: // DOOCS_adapter<double>
    {
        // 0mq: value ==> controller
        send(doocs_adapter_D->getWithoutCallback());
        // 0mq: cb_counters ==> controller
        send(_get_cb_counter);
        send(_set_cb_counter);
        break;
    }
    case 3: // DOOCS_adapter<string>
    {
        // 0mq: value ==> controller
        send(doocs_adapter_S->getWithoutCallback());
        // 0mq: cb_counters ==> controller
        send(_get_cb_counter);
        send(_set_cb_counter);
        break;
    }
    case 4: // DOOCS_array<int>
    {
        // 0mq: value ==> controller
        //    (the below sequence's aim is to read the arrays contents (explained in testDOOCSProcessVariableFactory.cpp))
        mtca4u::ProcessArray<int>    const * doocs_array_I_raw = doocs_array_I.get();
        mtca4u::DOOCSProcessVariableArray< int, mtca4u::m4uD_array<int> > const * DPVA_doocs_array_I_rawconst = static_cast< mtca4u::DOOCSProcessVariableArray< int, mtca4u::m4uD_array<int> > const * >( doocs_array_I_raw );
        mtca4u::DOOCSProcessVariableArray< int, mtca4u::m4uD_array<int> >       * DPVA_doocs_array_I_raw      =  const_cast< mtca4u::DOOCSProcessVariableArray< int, mtca4u::m4uD_array<int> >       * >( DPVA_doocs_array_I_rawconst );
        //    (only now getter is accessible)
        send(DPVA_doocs_array_I_raw->getWithoutCallback()[0]);
        // 0mq: cb_counters ==> controller
        send(_get_cb_counter);
        send(_set_cb_counter);
        break;
    }
    default:
        break;
    }
}

