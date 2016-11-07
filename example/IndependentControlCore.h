#ifndef _INDEPENDENT_CONTOL_CORE_H_
#define _INDEPENDENT_CONTOL_CORE_H_

#include <boost/scoped_ptr.hpp>

#include <ChimeraTK/ControlSystemAdapter/DevicePVManager.h>
#include <ChimeraTK/ControlSystemAdapter/ProcessArray.h>
#include <ChimeraTK/ControlSystemAdapter/DeviceSynchronizationUtility.h>
#include <ChimeraTK/ControlSystemAdapter/SynchronizationDirection.h>

/** Some dummy "hardware". You can read/write a voltage (int). */
class Hardware{
  int _voltage;
 public:
  void setVoltage(int v){_voltage=v;}
  int getVoltage() const {return _voltage;}
 Hardware(): _voltage(42){}
};

/** This is just a simple example class.
 *
 *  All functions are definded inline for the sake of the example. 
 *  It is strongly recommended to use proper header/object separation for
 *  real code!
 */
class IndependentControlCore{
 private:
  ChimeraTK::DevicePVManager::SharedPtr _processVariableManager;

  /** The target voltage to be transmitted to the hardware */
  ChimeraTK::ProcessArray<int>::SharedPtr _targetVoltage;

  /** The monitor voltage which is read back from the hardware */
  ChimeraTK::ProcessArray<int>::SharedPtr _monitorVoltage;
  
  Hardware _hardware; ///< Some hardware
 
  boost::scoped_ptr< boost::thread > _deviceThread;

  void mainLoop();

 public:
  /** The constructor gets an instance of the variable factory to use. 
   *  The variables in the factory should already be initialised because the hardware is initialised here.
   */
  IndependentControlCore(boost::shared_ptr<ChimeraTK::DevicePVManager> const & processVariableManager)
    //initialise all process variables, using the factory
    : _processVariableManager( processVariableManager ),
    _targetVoltage( processVariableManager->createProcessArray<int>(ChimeraTK::controlSystemToDevice,"TARGET_VOLTAGE", 1) ),
    _monitorVoltage( processVariableManager->createProcessArray<int>(ChimeraTK::deviceToControlSystem,"MONITOR_VOLTAGE", 1) ){

    // initialise the hardware here
    _targetVoltage->accessData(0) = 0;
    _monitorVoltage->accessData(0) = 0;
    _hardware.setVoltage( _targetVoltage->accessData(0) );

    // start the device thread, which is executing the main loop
    _deviceThread.reset( new boost::thread( boost::bind( &IndependentControlCore::mainLoop, this ) ) );
  }
  
  ~IndependentControlCore(){
    // stop the device thread before any other destructors are called
    _deviceThread->interrupt();
    _deviceThread->join();
 }

};

inline void IndependentControlCore::mainLoop(){
  ChimeraTK::DeviceSynchronizationUtility syncUtil(_processVariableManager);
 
  while (!boost::this_thread::interruption_requested()) {
    syncUtil.receiveAll();
    _monitorVoltage->accessData(0) = _hardware.getVoltage();

    _hardware.setVoltage( _targetVoltage->accessData(0) );
    syncUtil.sendAll();
    boost::this_thread::sleep_for( boost::chrono::milliseconds(100) );
  }
}

#endif // _INDEPENDENT_CONTOL_CORE_H_
