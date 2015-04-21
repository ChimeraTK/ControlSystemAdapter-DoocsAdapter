#ifndef eq_dpvatestsrv_h
#define eq_dpvatestsrv_h


#define CodeDpvatestsrv 10	// eq_fct_type number for the .conf file


#include <eq_fct.h>
//FIXME: The doocs adapter has to go into a sub-namespace
//#include <DOOCS_Adapter/DOOCSProcessVariableFactory.hpp>      // FIXME: czy ten komentarz ma tu sens?
#include <DOOCSProcessVariableFactory.hpp>

#include <boost/noncopyable.hpp>       // 
#include <boost/scoped_ptr.hpp>        // FIXME: potrzebuje tego pierdolnika do tego testu?


#include <zmq.hpp>


#define PORT	3497    // FIXME: to do config, czy gdzies (Makefile?)



#include <sstream>
#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()



///// 0mq helpers /////////////////////////////////////////////////////////---<>
                                                                         //
zmq::context_t context (1);
zmq::socket_t zmqsocket (context, ZMQ_REQ);
zmq::message_t stubreply;


void zmqsend(const std::string & _str)
{
    zmq::message_t request (_str.length()+1);
    memcpy ((void *) request.data (), _str.c_str(), _str.length()+1);
    zmqsocket.send (request);
}


template <typename T>
void send(const T & _arg)
{
    std::string sarg = SSTR( _arg );
    zmqsend(sarg);
    zmqsocket.recv (&stubreply);
}
                                                                         //
///////////////////////////////////////////////////////////////////////////---<>



///// callback counters and functions /////////////////////////////////////---<>
                                                                         //
int _get_cb_counter = 0;
int _set_cb_counter = 0;

template <typename T>
T        on_get_callback ()
         {
             ++_get_cb_counter;
             return 0;
         }
template <typename T>
void     on_set_callback (T const & newValue, T const & oldValue)
         {
             (void)newValue; (void)oldValue;
//             if (newValue == oldValue) ++_set_cb_counter_equals;
             ++_set_cb_counter;
         }

template <>
std::string on_get_callback ()
            {
                ++_get_cb_counter;
                return std::string("0");
            }
template <>
void        on_set_callback (std::string const & newValue, std::string const & oldValue)
            {
                (void)newValue; (void)oldValue;
//                if (newValue == oldValue) ++_set_cb_counter_equals;
                ++_set_cb_counter;
            }
                                                                         //
///////////////////////////////////////////////////////////////////////////---<>






class EqFctDpvatestsrv : public EqFct, boost::noncopyable   // FIXME: potrzebuje noncopyable do tego testu?
{

private:

	mtca4u::DOOCSProcessVariableFactory DOOCSPVFactory;

    boost::shared_ptr<mtca4u::ProcessVariable<int        > > doocs_adapter_I;       // no callbacks
    boost::shared_ptr<mtca4u::ProcessVariable<int        > > doocs_adapter_I_cb;    // has callbacks
    
    boost::shared_ptr<mtca4u::ProcessVariable<float      > > doocs_adapter_F;       // no callbacks
    boost::shared_ptr<mtca4u::ProcessVariable<float      > > doocs_adapter_F_cb;    // has callbacks
    
    boost::shared_ptr<mtca4u::ProcessVariable<double     > > doocs_adapter_D;       // no callbacks
    boost::shared_ptr<mtca4u::ProcessVariable<double     > > doocs_adapter_D_cb;    // has callbacks
    
    boost::shared_ptr<mtca4u::ProcessVariable<std::string> > doocs_adapter_S;       // no callbacks
    boost::shared_ptr<mtca4u::ProcessVariable<std::string> > doocs_adapter_S_cb;    // has callbacks
    
    int testnr;

//        boost::shared_ptr< mtca4u::ProcessVariableFactory > processVariableFactory_;
//	boost::shared_ptr< mtca4u::ProcessVariable<int> > doocs_adapter_I;

public:
	EqFctDpvatestsrv ();

	void    init ();
	void	update ();
	int	fct_code()	{ return CodeDpvatestsrv; }
};

#endif
