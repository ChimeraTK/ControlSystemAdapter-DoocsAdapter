#ifndef __d_fct__
#define __d_fct__


class EqFct; // predecl (in DOOCS this goes in d_fct_gen.h - but nevermind here)
class EqAdr;
class EqData;




class D_fct
{
public:
    virtual void    get   (EqAdr *, EqData *, EqData *, EqFct *) {}
    virtual void    set   (EqAdr *, EqData *, EqData *, EqFct *) {}
};


/**
 * simplified DOOCS property model for tests
 * http://ttfinfo.desy.de/doocs/doocs_libs/serverlib/html/d__fct_8h_source.html  -> class D_double
 * http://doocs.desy.de/cgi-bin/viewvc.cgi/source/serverlib/d_fct.cc?view=markup -> D_double::set_value
 */
class D_double : public D_fct
{

protected:
    const char          * pn_; // suppress -Wunused-parameter
	const EqFct         * ef_; // suppress -Wunused-parameter
    
	double				value_;
    
public:

    D_double (const char *pn, EqFct *ef) : pn_(pn), ef_(ef), value_(0) {}
    D_double (const char *pn)            : pn_(pn),          value_(0) {}


	virtual void	set_value (double val)
					{
						value_ = val;
					}

	virtual double	value ()
					{
						return value_;
					}
};






/**
 * simplified DOOCS property model for tests
 * http://ttfinfo.desy.de/doocs/doocs_libs/serverlib/html/d__fct_8h_source.html  -> class D_float
 * http://doocs.desy.de/cgi-bin/viewvc.cgi/source/serverlib/d_fct.cc?view=markup -> D_float::set_value
 */
class D_float : public D_fct
{

protected:
    const char          * pn_; // suppress -Wunused-parameter
	const EqFct         * ef_; // suppress -Wunused-parameter
    
	float				value_;
    
public:

    D_float (const char *pn, EqFct *ef) : pn_(pn), ef_(ef), value_(0) {}
    D_float (const char *pn)            : pn_(pn),          value_(0) {}


	virtual void	set_value (float val)
					{
						value_ = val;
					}

	virtual float	value ()
					{
						return value_;
					}
};






/**
 * simplified DOOCS property model for tests
 * http://ttfinfo.desy.de/doocs/doocs_libs/serverlib/html/d__fct_8h_source.html  -> class D_int
 * http://doocs.desy.de/cgi-bin/viewvc.cgi/source/serverlib/d_fct.cc?view=markup -> D_int::set_value
 */
class D_int : public D_fct
{

protected:
    const char          * pn_; // suppress -Wunused-parameter
	const EqFct         * ef_; // suppress -Wunused-parameter
    
	int				value_;
    
public:
    
    D_int (const char *pn, EqFct *ef) : pn_(pn), ef_(ef), value_(0) {}
    D_int (const char *pn)            : pn_(pn),          value_(0) {}


	virtual void	set_value (int val)
					{
						value_ = val;
					}

	virtual int		value ()
					{
						return value_;
					}
};






#include <string>
#include <cstring>

#define STRING_LENGTH 80    // I don't know where this belongs. The DOOCS doc just says 80.


/**
 * simplified DOOCS property model for tests
 * http://ttfinfo.desy.de/doocs/doocs_libs/serverlib/html/d__fct_8h_source.html  -> class D_string
 * http://doocs.desy.de/cgi-bin/viewvc.cgi/source/serverlib/d_fct.cc?view=markup -> D_string::set_value
 */
 
// code mostly taken from http://doocs.desy.de/cgi-bin/viewvc.cgi/source/serverlib/d_fct.cc

class D_string : public D_fct
{

protected:
    const char          * pn_; // suppress -Wunused-parameter
	const EqFct         * ef_; // suppress -Wunused-parameter
    
    unsigned short  type_; // 0: user defined length, 1: dynamic length
    unsigned short  len_;
    char            *txt_;
    std::string     text_; // for dynamic string

    
public:

                        D_string (const char *pn, EqFct *ef) : pn_(pn), ef_(ef)
                        {
                            type_ = 1;
                            len_  = 0;
                            txt_  = (char *) 0;
                        }
                        D_string (const char *pn)            : pn_(pn)
                        {
                            type_ = 1;
                            len_  = 0;
                        }


	virtual void	    set_value (const char *str)
                        {
                            if (!str) return;
                    
                            int slen;
                            slen = (int) strlen (str) + 1;
                            if (slen > STRING_LENGTH) slen = STRING_LENGTH;
                    
                            if (type_) {
                    
                                text_.clear (); // clear string buffer for Java
                    
                                // for dynamic allocation
                                text_ = str;
                                txt_  = (char *) text_.c_str ();
                                len_  = (int) text_.length ();
                    
                            } else {
                                memset (txt_, 0, len_); // clear string buffer for Java
                                if (slen >= len_) {
                                    strncpy (txt_, str, len_);
                                    txt_ [len_ - 1] = '\0';
                                } else {
                                    strncpy (txt_, str, slen);
                                    txt_ [slen] = '\0';
                                }
                            }
                        }

	virtual const char* value ()
                        {
                            return type_ ? text_.c_str () : txt_;
                        }
};





#include <vector>


/**
 * simplified DOOCS property model for tests
 * http://ttfinfo.desy.de/doocs/doocs_libs/serverlib/html/d__fct_8h_source.html  -> class D_spectrum
 * http://doocs.desy.de/cgi-bin/viewvc.cgi/source/serverlib/D_spectrum.cc?view=co
 */
class D_spectrum : public D_fct
{

protected:

    const char          * pn_; // suppress -Wunused-parameter
    int                 max_length_;
	const EqFct         * ef_; // suppress -Wunused-parameter
    
	std::vector<float>  value_;


public:

    D_spectrum  (const char *pn, int maxl, EqFct *ef) : pn_(pn), max_length_(maxl), ef_(ef), value_(maxl, 0) {}

    void  fill_spectrum (int i, float data)
          {
              if ((i >= 0) && (i < max_length_)) value_[i] = data;
          }
    float read_spectrum (int i) const
          {
              if ((i >= 0) && (i < max_length_)) return value_[i];
              return 0.0;
          }
    int   length (void)
          {
              return value_.size();
          }
};



#endif /* __d_fct__ */
