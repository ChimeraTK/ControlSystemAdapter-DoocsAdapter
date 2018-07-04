#ifndef __DOOCS_UPDATER_H__
#define __DOOCS_UPDATER_H__

#include <unordered_map>
#include <map>
#include <mtca4u/TransferElement.h>
#include <boost/noncopyable.hpp>

namespace ChimeraTK{
  /** A class to synchronise DeviceToControlSystem variable to Doocs.
   *  It contains a list of TransferElements and a thread which is monitoring them for updates.
   *  The thread has to be started with the run() functions, which returns immediately
   *  when the thread is started, and (FIXME can be stopped by the stop() function which
   *  returns after the thread has been joined). This happens latest in the destructor.
   */
  class DoocsUpdater: public boost::noncopyable{
  public:
    ~DoocsUpdater();
    void update(); // Update all variables once. This is a convenience function for testing.

    void updateLoop(); // Endless loop with interruption point around the update function.

    void run();
    void stop();

    void addVariable( const ChimeraTK::TransferElementAbstractor &variable, std::function<void ()> updaterFunction);
  protected:
    std::list< ChimeraTK::TransferElementAbstractor > _elementsToRead;
    boost::thread _syncThread;// we have to use boost thread to use interruption points
    //FIXME: make this an unordered map
    std::map< mtca4u::TransferElementID, std::vector< std::function<void ()> > > _toDoocsUpdateMap;

  };
}

#endif // __DOOCS_UPDATER_H__
