#ifndef TEST_COSADE_EQ_FCT_H_
#define	TEST_COSADE_EQ_CT_H_

#include "eq_fct.h"
#include "IndependentControlCore.h"

/**
 * Instantiate the IndependentControlCore in an EqFct.
 */
class TestScalarsEqFct : public EqFct
{
public:
    /**
     * The unique EqFct code of this class. It must be different for each
     * EqFct class in the server.
     */
    static const int code = 500;

    // Exposed properties (D_fcts)
    mtca4u::DoocsProcessScalar<float, D_float> myDoocsFloat_;
    
    // Class methods
    TestScalarsEqFct();
    virtual ~TestScalarsEqFct();

    /**
     * Return the numeric code used to identify this EqFct in the configuration
     * file.
     */
    virtual int fct_code() { return code; }

    /**
     * Perform initializations after the configuration file has been read.
     */
    virtual void init();

    /**
     * Perform periodic tasks.
     */
    virtual void update();
};


#endif
