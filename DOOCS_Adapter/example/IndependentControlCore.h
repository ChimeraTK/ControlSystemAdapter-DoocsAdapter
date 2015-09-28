#ifndef _INDEPENDENT_CONTOL_CORE_H_
#define _INDEPENDENT_CONTOL_CORE_H_

#include <boost/scoped_ptr.hpp>

#include <ControlSystemAdapter/DevicePVManager.h>
#include <ControlSystemAdapter/DeviceSynchronizationUtility.h>

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
  mtca4u::DevicePVManager::SharedPtr _processVariableManager;

  /** The target voltage to be transmitted to the hardware */
  mtca4u::ProcessScalar<int>::SharedPtr _targetVoltage;

  /** The monitor voltage which is read back from the hardware */
  mtca4u::DeviceProcessScalar<int>::SharedPtr > _monitorVoltage;
  
  Hardware _hardware; ///< Some hardware
 
  boost::scoped_ptr< boost::thread > _deviceThread;

  void mainLoop();

 public:
  /** The constructor gets an instance of the variable factory to use. 
   *  The variables in the factory should already be initialised because the hardware is initialised here.
   */
  IndependentControlCore(boost::shared_ptr<mtca4u::DevicePVManager> & processVariableManager)
    //initialise all process variables, using the factory
    : _processVariableManager( processVariableManager ),
    _targetVoltage( pvManager->createProcessScalarControlToDeviceSystem<int>("TARGET_VOLTAGE") ),
    _monitorVoltage( pvManager->createProcessScalarDeviceToControlSystem<int>("MONITOR_VOLTAGE") ){

    // initialise the hardware here

    // start the device thread, which is executing the main loop
    _deviceThread.reset( new boost::thread( boost::bind( &IndependentControlCore::mainLoop, this ) ) );
  }
  
  ~IndependentControlCore(){
    // The destructor of the boost::thread wil send interrupt_requested, which is evaluated inside the loop.
    // No actions needed in the desructor.
  }

};

inline void IndependentControlCore::mainLoop(){
  mtca4u::DeviceSynchronizationUtility syncUtil(_processVariableManager);
 
  while (!boost::this_thread::interruption_requested()) {
    std::cout << "IndependentControlCore::mainLoop()" <<std::endl;

    syncUtil.receiveAll();
    *_monitorVoltage = _hardware.getVoltage();

    _hardware.setVoltage( *_targetVoltage );
    syncUtil.sendAll();    

    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
  }
}

#endif // _INDEPENDENT_CONTOL_CORE_H_
