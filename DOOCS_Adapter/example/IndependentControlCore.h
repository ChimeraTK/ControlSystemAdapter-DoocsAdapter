#ifndef _INDEPENDENT_CONTOL_CORE_H_
#define _INDEPENDENT_CONTOL_CORE_H_

#include <ControlSystemAdapter/ProcessVariableFactory.h>

/** Some dummy "hardware". You can read/write a voltage (int). */
class Hardware{
  int _voltage;
 public:
  void setVoltage(int v){_voltage=v;}
  int getVoltage() const {return _voltage;}
 Hardware(): _voltage(42){}
};

/** This is just a simple example class. It does not use 
 *  the callback registration mechanism yet because this will change in future.
 *
 *  All functions are definded inline for the sake of the example. 
 *  It is strongly recommended to use proper header/object separation for
 *  real code!
 */
class IndependentControlCore{
 private:
  /** The target voltage to be transmitted to the hardware */
  boost::shared_ptr< mtca4u::ProcessVariable<int> > targetVoltage;

  /** The monitor voltage which is read back from the hardware */
  boost::shared_ptr< mtca4u::ProcessVariable<int> > monitorVoltage;

  Hardware _hardware; ///< Some hardware

 public:
  /** The constructor gets an instance of the variable factory to use. 
   *  The variables in the factory should already be initialised because the hardware is initialised here.
   */
  IndependentControlCore(boost::shared_ptr<mtca4u::ProcessVariableFactory> & processVariableFactory)
    //initialise all process variables, using the factory
    : targetVoltage( processVariableFactory->getProcessVariable<int>("TARGET_VOLTAGE") ),
    monitorVoltage( processVariableFactory->getProcessVariable<int>("MONITOR_VOLTAGE") ){
    // initialise the hardware here
  }
  

  void readFromHardware(){
    *monitorVoltage = _hardware.getVoltage();
  }

  void writeToHardware(){
    _hardware.setVoltage( *targetVoltage );
  }
  
};

#endif // _INDEPENDENT_CONTOL_CORE_H_
