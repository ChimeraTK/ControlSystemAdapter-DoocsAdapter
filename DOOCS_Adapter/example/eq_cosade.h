#ifndef COSADE_SERVER_H
#define COSADE_SERVER_H

#include <eq_fct.h>
//FIXME: The doocs adapter has to go into a sub-namespace
//#include <DOOCS_Adapter/DOOCSProcessVariableFactory.hpp>
#include <DOOCSProcessVariableFactory.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

// include your business logic
#include "IndependentControlCore.h"

static const int CODE_COSADE = 10; // eq_fct_type number for the .conf file

/* COSADE is the COntrol Sytem ADapter Example.
 * It is a simple example server which embeds control system independent business logic 
 * using the MTCA4U control system adapter.
 *
 * P.S. We use boost::noncpoyable to disable copy constructor and assignment operator.
 */
class CosadeServer : public EqFct , boost::noncopyable {

private:
    /* You need in an instance of the DOOCS process variable factory. It holds all 
     * the process variables for you. It has to be stored as a shared pointer to ProcessVariableFactory
     * so it can be handed over to the business logic.
     */
    boost::shared_ptr< mtca4u::ProcessVariableFactory > processVariableFactory_;

    /* You also need an instance of your business logic. It is allocated dynamically. 
     * We use smart pointers to avoid the hassle with memory management.
     */
    boost::scoped_ptr<IndependentControlCore> controlCore_;

public:
    CosadeServer(); // In the constuctor the process variable factory is initialised.
    
    void init(); // In init() the business logic is initialised 
    void update(); // In update() the business logic is called
    
    int fct_code() {return CODE_COSADE;}
};

#endif // COSADE_SERVER_H
