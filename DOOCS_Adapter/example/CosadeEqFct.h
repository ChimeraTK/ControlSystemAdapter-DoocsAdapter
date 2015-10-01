#ifndef COSADE_EQ_FCT_H
#define COSADE_EQ_FCT_H

#include <eq_fct.h>
//FIXME: The doocs adapter has to go into a sub-namespace
//#include <DOOCS_Adapter/DOOCS_Adapter.h>
#include <DoocsAdapter.h>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

// Include your business logic
#include "IndependentControlCore.h"

static const int CODE_COSADE = 10; // eq_fct_type number for the .conf file

/** COSADE is the COntrol Sytem ADapter Example.
 *  It is a simple example server which embeds control system independent business logic 
 *  using the MTCA4U control system adapter.
 *
 *  P.S. We use boost::noncpoyable to disable copy constructor and assignment operator.
 */
class CosadeEqFct : public EqFct , boost::noncopyable {

private:
  /* You need an instance of the mtca4u DoocsAdapter.
   */
  mtca4u::DoocsAdapter doocsAdapter_;

  /* You also need an instance of your business logic. It has to be allocated dynamically. 
   * We use smart pointers to avoid the hassle with memory management.
   */
  boost::scoped_ptr<IndependentControlCore> controlCore_;

public:
    CosadeEqFct();
    
    void init();
    void update();
    
    int fct_code() {return CODE_COSADE;}
};

#endif // COSADE_EQ_FCT_H
