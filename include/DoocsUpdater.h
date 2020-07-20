#ifndef __DOOCS_UPDATER_H__
#define __DOOCS_UPDATER_H__

#include <ChimeraTK/TransferElementAbstractor.h>
#include <ChimeraTK/TransferElement.h>
#include <boost/noncopyable.hpp>
#include <eq_fct.h>
#include <map>
#include <unordered_map>

namespace ChimeraTK {

  /** A class to synchronise DeviceToControlSystem variable to Doocs.
   *  It contains a list of TransferElements and a thread which is monitoring them
   * for updates. The thread has to be started with the run() functions, which
   * returns immediately when the thread is started, and (FIXME can be stopped by
   * the stop() function which returns after the thread has been joined). This
   * happens latest in the destructor.
   */
  class DoocsUpdater : public boost::noncopyable {
   public:
    ~DoocsUpdater();
    void update(); // Update all variables once. This is a convenience function
                   // for testing.

    void updateLoop(); // Endless loop with interruption point around the update
                       // function.

    void run();
    void stop();

    // Add a variable to be updated. Together with the TransferElementAbstractor
    // pointing to the ChimeraTK::ProcessArray, the EqFct* to obtain the lock for
    // and a function to be called which executes the actual update should be
    // specified. The lock is held while the updaterFunction is called, so it must
    // neither obtained nor freed within the updaterFunction.
    void addVariable(
        const ChimeraTK::TransferElementAbstractor& variable, EqFct* eq_fct, std::function<void()> updaterFunction);

   protected:
    std::list<ChimeraTK::TransferElementAbstractor> _elementsToRead;
    boost::thread _syncThread; // we have to use boost thread to use interruption points
    // FIXME: make this an unordered map
    std::map<ChimeraTK::TransferElementID, std::vector<std::function<void()>>> _toDoocsUpdateMap;
    std::map<ChimeraTK::TransferElementID, std::vector<EqFct*>> _toDoocsEqFctMap;
  };
} // namespace ChimeraTK

#endif // __DOOCS_UPDATER_H__
