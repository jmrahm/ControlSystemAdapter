#include "DeviceSynchronizationUtility.h"

namespace ChimeraTK {

  DeviceSynchronizationUtility::DeviceSynchronizationUtility(
      DevicePVManager::SharedPtr pvManager) :
      _pvManager(pvManager) {
    // We choose the load factor very small. This will increase the memory
    // footprint, however because of frequent lookups we want to get the
    // performance as close to O(1) as reasonably possible.
    _receiveNotificationListeners.max_load_factor(0.5);
  }

  void DeviceSynchronizationUtility::addReceiveNotificationListener(
      std::string const & processVariableName,
      ProcessVariableListener::SharedPtr receiveNotificationListener) {
    _receiveNotificationListeners.erase(processVariableName);
    _receiveNotificationListeners.insert(
        std::make_pair(processVariableName, receiveNotificationListener));
  }

  void DeviceSynchronizationUtility::removeReceiveNotificationListener(
      std::string const & processVariableName) {
    _receiveNotificationListeners.erase(processVariableName);
  }

  void DeviceSynchronizationUtility::receiveAll() {
    ProcessVariable::SharedPtr pv;
    while ((pv = _pvManager->nextNotification())) {
      if (pv->receive()) {
        boost::unordered_map<std::string, ProcessVariableListener::SharedPtr>::iterator listenerIterator =
            _receiveNotificationListeners.find(pv->getName());
        if (listenerIterator != _receiveNotificationListeners.end()) {
          ProcessVariableListener::SharedPtr receiveListener(
              listenerIterator->second);
          receiveListener->notify(pv);
          while (pv->receive()) {
            receiveListener->notify(pv);
          }
        } else {
          while (pv->receive()) {
            continue;
          }
        }
      }
    }
  }

  void DeviceSynchronizationUtility::sendAll() {
    std::vector<ProcessVariable::SharedPtr> processVariables(
        _pvManager->getAllProcessVariables());
    for (std::vector<ProcessVariable::SharedPtr>::iterator i =
        processVariables.begin(); i != processVariables.end(); ++i) {
      if ((*i)->isSender()) {
        (*i)->send();
      }
    }
  }

  void DeviceSynchronizationUtility::waitForNotifications(
      long timeoutInMicroseconds, long checkIntervalInMicroseconds) {
    boost::chrono::high_resolution_clock::time_point limit =
        boost::chrono::high_resolution_clock::now()
            + boost::chrono::microseconds(timeoutInMicroseconds);
    boost::chrono::microseconds checkInterval(checkIntervalInMicroseconds);
    do {
      receiveAll();
      if (timeoutInMicroseconds > 0 && checkIntervalInMicroseconds > 0) {
        boost::this_thread::sleep_for(checkInterval);
      } else {
        return;
      }
      // Look for a pending notification until we reach the timeout.
    } while (boost::chrono::high_resolution_clock::now() < limit);
  }

} // namespace ChimeraTK
