#ifndef __myd_int__
#define __myd_int__


#include <boost/bind.hpp>



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






#endif /* __myd_int__ */

