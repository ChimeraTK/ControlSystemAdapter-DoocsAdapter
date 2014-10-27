#ifndef __myd_int_mock__
#define __myd_int_mock__


#include "DOOCSProcessVariableAdapter.hpp"



class EqFct {}; // stub


/**
 * simplified DOOCS property model for tests
 * http://ttfinfo.desy.de/doocs/doocs_libs/serverlib/html/d__fct_8h_source.html  -> class D_int
 * http://doocs.desy.de/cgi-bin/viewvc.cgi/source/serverlib/d_fct.cc?view=markup -> D_int::set_value
 */
class D_int
{

protected:
	int				value_;
	
public:
    
    unsigned int __set_value_f_call_counter;                // __
    unsigned int __value_f_call_counter;                    // __

    D_int (const char *pn, EqFct *ef) : value_(0), __set_value_f_call_counter(0), __value_f_call_counter(0) {}
    D_int (const char *pn)            : value_(0), __set_value_f_call_counter(0), __value_f_call_counter(0) {}


	virtual void	set_value (int val)
					{
                        __set_value_f_call_counter++;       // __
						value_ = val;
					}

	virtual int		value ()
					{
                        __value_f_call_counter++;           // __
						return value_;
					}
};





class myD_int : public D_int
{
    
protected:
    DOOCSPVAdapter * doocs_adapter;     // DOOCS process variable adapter handler
	
public:
    
    unsigned int __on_set_callback_f_call_counter;                       // __
    unsigned int __on_get_callback_f_call_counter;                       // __

    myD_int (const char *pn, EqFct *ef) :    D_int(pn, ef),
                                             __on_set_callback_f_call_counter(0),
                                             __on_get_callback_f_call_counter(0)    {}

    virtual void    init (DOOCSPVAdapter * _da)
                    {
                        doocs_adapter = _da;
                        doocs_adapter->setOnSetCallbackFunction(boost::bind (&myD_int::on_set_callback, this, _1));     // here want only to do a set. original set_value() does exactly this also
                    }

	virtual void	set_value (int val)
					{
						D_int::set_value(val);
                        doocs_adapter->setWithoutCallback(val);
					}
	virtual int		value ()
					{
                        // anything else needed here?
                        // I guess not, if the values are to be remembered by both the PVA and the DOOCS type
						return D_int::value();
					}


                    // callbacks
	void    	    on_set_callback (int val)
					{
                        __on_set_callback_f_call_counter++;              // __
						D_int::set_value(val);
					}
	int             on_get_callback ()
					{
                        __on_get_callback_f_call_counter++;              // __
						return D_int::value();
					}
};






#endif /* __myd_int_mock__ */

