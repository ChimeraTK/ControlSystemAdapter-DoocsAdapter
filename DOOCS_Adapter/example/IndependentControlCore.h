#ifndef _INDEPENDENT_CONTOL_CORE_H_
#define _INDEPENDENT_CONTOL_CORE_H_

#include <boost/scoped_ptr.hpp>

#include <ControlSystemAdapter/DevicePVManager.h>

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
  boost::shared_ptr<mtca4u::DevicePVManager> _processVariableManager;

  /** The target voltage to be transmitted to the hardware */
  boost::shared_ptr< mtca4u::DeviceProcessScalar<int> > _targetVoltage;

  /** The monitor voltage which is read back from the hardware */
  boost::shared_ptr< mtca4u::DeviceProcessScalar<int> > _monitorVoltage;
  
  Hardware _hardware; ///< Some hardware
 
  boost::atomic<bool> _stopDeviceThread;
  boost::scoped_ptr< boost::thread > _deviceThread;

  void mainLoop();

 public:
  /** The constructor gets an instance of the variable factory to use. 
   *  The variables in the factory should already be initialised because the hardware is initialised here.
   */
  IndependentControlCore(boost::shared_ptr<mtca4u::DevicePVManager> & processVariableManager)
    //initialise all process variables, using the factory
    : _processVariableManager( processVariableManager ),
    _targetVoltage( processVariableManager->getProcessScalar<int>("TARGET_VOLTAGE") ),
    _monitorVoltage( processVariableManager->getProcessScalar<int>("MONITOR_VOLTAGE") ),
    _stopDeviceThread(false){

    // initialise the hardware here

    // start the device thread, which is executing the main loop
    _deviceThread.reset( new boost::thread( boost::bind( &IndependentControlCore::mainLoop, this ) ) );
  }
  
  ~IndependentControlCore(){
    // Stop the device thread. Set the flag variable and wait until the thread terminates.
    _stopDeviceThread=true;
    _deviceThread->join();
    std::cout << "thread stopped " << std::endl;
  }

  void readFromHardware(){
    *_monitorVoltage = _hardware.getVoltage();
    std::cout << "reading from hardware: " << *_monitorVoltage << std::endl;
  }

  void writeToHardware(){
    _hardware.setVoltage( *_targetVoltage );
    std::cout << "writing to hardware: " << *_targetVoltage << std::endl;
  }
  
  static void registerProcessVariables(boost::shared_ptr<mtca4u::DevicePVManager> & processVariableManager){
    processVariableManager->createProcessScalar<int>("TARGET_VOLTAGE");
    processVariableManager->createProcessScalar<int>("MONITOR_VOLTAGE");
  }

};

inline void IndependentControlCore::mainLoop(){
  while (! _stopDeviceThread) {
    std::cout << "IndependentControlCore::mainLoop()" <<std::endl;
    _processVariableManager->processSynchronization(10000);//, 100);
    writeToHardware();
    readFromHardware();
    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
  }
}

#endif // _INDEPENDENT_CONTOL_CORE_H_
