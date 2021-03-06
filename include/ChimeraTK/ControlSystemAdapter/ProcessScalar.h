#ifndef CHIMERA_TK_CONTROL_SYSTEM_ADAPTER_PROCESS_SCALAR_H
#define CHIMERA_TK_CONTROL_SYSTEM_ADAPTER_PROCESS_SCALAR_H

#include <utility>

#include <boost/smart_ptr.hpp>

#include <limits>
#include <stdexcept>
#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include "ProcessVariable.h"
#include "ProcessVariableListener.h"
#include "TimeStampSource.h"
#include "VersionNumberSource.h"

namespace ChimeraTK {

  /**
   * The scalar implementation of a ProcessVariable. This implementation is used for all
   * three use cases (sender, receiver, and stand-alone).
   *
   * This class is not thread-safe and  should only be used from a single thread.
   */
  template<class T>
  class ProcessScalar: public ProcessVariable {

  public:
    /**
     * Type of the instance. This defines the behavior (send or receive
     * possible, modifications allowed, etc.). This enum type is private so
     * that only friend functions can construct instances of this class. The
     * constructors themselves have to be public so that boost::make_shared
     * can be used.
     */
    enum InstanceType {
      /**
       * Instance acts on is own.
       */
      STAND_ALONE,

      /**
       * Instance acts as the sender in a sender / receiver pair.
       */
      SENDER,

      /**
       * Instance acts as the receiver in a sender / receiver pair.
       */
      RECEIVER
    };

    /**
     * Type alias for a shared pointer to this type.
     */
    typedef boost::shared_ptr<ProcessScalar> SharedPtr;

    /**
     * Creates a process scalar that works independently. This means that the
     * instance is not synchronized with any other instance and thus the send
     * and receive operations are not supported. However, all other operations
     * can be used like on any process variable.
     */
    ProcessScalar(InstanceType instanceType, const std::string& name,
        T initialValue) :
        ProcessVariable(name), _instanceType(instanceType), _value(
            initialValue), _versionNumber(0) {
      // It would be better to do the validation before initializing, but this
      // would mean that we would have to initialize twice.
      if (instanceType != STAND_ALONE) {
        throw std::invalid_argument(
            "This constructor may only be used for a stand-alone process scalar.");
      }
    }

    /**
     * Creates a process scalar that acts as a receiver. A receiver is
     * intended to work as a tandem with a sender and receives values that
     * have been set to the sender.
     *
     * This constructor creates the buffers and queues that are needed for the
     * send/receive process and are shared with the sender.
     */
    ProcessScalar(InstanceType instanceType, const std::string& name,
        T initialValue, std::size_t numberOfBuffers,
        VersionNumberSource::SharedPtr versionNumberSource) :
        ProcessVariable(name), _instanceType(instanceType), _value(
            initialValue), _versionNumber(0), _bufferQueue(
            boost::make_shared<boost::lockfree::queue<Buffer> >(
                numberOfBuffers)), _versionNumberSource(versionNumberSource) {
      // It would be better to do the validation before initializing, but this
      // would mean that we would have to initialize twice.
      if (instanceType != RECEIVER) {
        throw std::invalid_argument(
            "This constructor may only be used for a receiver process scalar.");
      }
      if (numberOfBuffers < 1) {
        throw std::invalid_argument(
            "The number of buffers must be at least one.");
      }
    }

    /**
     * Creates a process scalar that acts as a sender. A sender is intended
     * intended to work as a tandem with a receiver and send set values to
     * it. It uses the buffers and queues that have been created by the
     * receiver.
     *
     * If the <code>swappable</code> option is true, the swap operation is
     * allowed on the receiver. The <code>swappable</code> flag must be the
     * same for the sender and receiver. Allowing swapping has the side effect
     * that a value cannot be read after having been sent.
     *
     * The optional send-notification listener is notified every time the
     * sender's {@link ProcessScalar::send()} method is called. It can be
     * used to queue a request for the receiver's
     * {@link ProcessScalar::receive()} method to be called. The process
     * variable passed to the listener is the receiver and not the sender.
     */
    ProcessScalar(InstanceType instanceType,
        TimeStampSource::SharedPtr timeStampSource,
        VersionNumberSource::SharedPtr versionNumberSource,
        ProcessVariableListener::SharedPtr sendNotificationListener,
        ProcessScalar::SharedPtr receiver) :
        ProcessVariable(receiver->getName()), _instanceType(instanceType), _timeStamp(
            receiver->_timeStamp), _value(receiver->_value), _versionNumber(0), _bufferQueue(
            receiver->_bufferQueue), _receiver(receiver), _timeStampSource(
            timeStampSource), _versionNumberSource(versionNumberSource), _sendNotificationListener(
            sendNotificationListener) {
      // It would be better to do the validation before initializing, but this
      // would mean that we would have to initialize twice.
      if (instanceType != SENDER) {
        throw std::invalid_argument(
            "This constructor may only be used for a sender process scalar.");
      }
      if (!receiver) {
        throw std::invalid_argument(
            "The pointer to the receiver must not be null.");
      }
      if (receiver->_instanceType != RECEIVER) {
        throw std::invalid_argument(
            "The pointer to the receiver must point to an instance that is actually a receiver.");
      }
    }

    /**
     * Assign the content of another process variable of type T to this one.
     * It only assigns the variable content, but not the callback functions.
     * This operator behaves like set().
     *
     * FIXME: Should we remove this? It might not behave as 
     * expected, and the base class is intentionally non-copyable.
     */
    ProcessScalar<T> & operator=(ProcessScalar<T> const & other) {
      set(other);
      return (*this);
    }

    /**
     * Assign the content of the template type. This operator behaves like
     * set().
     */
    ProcessScalar<T> & operator=(T const & t) {
      set(t);
      return *this;
    }

    /**
     * Automatic conversion operator which returns a \b copy of this process
     * variable's value. As no reference is returned, this cannot be used for
     * assignment.
     */
    operator T() const {
      return get();
    }

    /**
     * Set the value of this process variable to the specified one. This does
     * not trigger the on-set callback function, however it notifies the control
     * system that this process variable's value has changed.
     */
    void set(T const & t) {
      _value = t;
    }

    /**
     * Sets and sends the value of this process variable only if the version
     * number of the new value is greater than the version number of the current
     * value.
     *
     * This is a convenience method that is provided for when the value of a
     * process variable is updated with a value calculated from the value of a
     * different process variable. Calling this method is equivalent to the
     * following code:
     *
     * <pre>
     * if (newVersionNumber > getVersionNumber()) {
     *   set(t);
     *   send(newVersionNumber);
     * }
     * </pre>
     *
     * By using this method, one can easily avoid overwriting a more recent
     * value with an older value, which could otherwise happen due to updates
     * being handled asynchronously.
     */
    void setAndSendIfNewVersionGreater(T const & t, VersionNumber newVersionNumber) {
      if (newVersionNumber > getVersionNumber()) {
        set(t);
        send(newVersionNumber);
      }
    }

    // FIXME: this belongs to ProcessScalar, not the Impl. Moved here to remove the impl
    // inheritence for creating the pimpl pattern, where impl and not are typedefed.
    const std::type_info& getValueType() const {
      return typeid(T);
    }

    bool isArray() const {
      return false;
    }

    /**
     * Set the value of this process variable to the one of the other process
     * variable. This does not trigger the on-set callback function, however it
     * notifies the control system that this process variable's value has
     * changed.
     */
    void set(ProcessScalar<T> const & other) {
      set(other.get());
    }

    /**
     * Returns a \b copy of this process variable's value. As no reference is
     * returned, this cannot be used for assignment.
     */
    T get() const {
      return _value;
    }

    // -----------------------------------------------
    // Implementations of the derrived functions. They are documented in ProcessVariable

    bool isReceiver() const {
      return _instanceType == RECEIVER;
    }

    bool isSender() const {
      return _instanceType == SENDER;
    }

    // deprecated
    TimeStamp getTimeStamp() const {
      return _timeStamp;
    }

    /**
     * Returns the version number that is associated with the current value.
     * This is the version number that was received with the last
     * {@link receive()} operation or sent with the last {@link send()}
     * operation (whichever happened later).
     *
     * The version number is used to resolve conflicting updates. When an update
     * is received using the {@link receive()} method, it is only used if its
     * value has a version number that is greater than the version number of the
     * current value. Initially, each process variable has a version number of
     * zero.
     *
     * When this process variable has not been initialized with a version number
     * source, its version number always stays at zero and the version-number
     * logic is disabled.
     */
    VersionNumber getVersionNumber() const {
      return _versionNumber;
    }

    bool receive() {
      if (_instanceType != RECEIVER) {
        throw std::logic_error(
            "Receive operation is only allowed for a receiver process variable.");
      }
      Buffer nextBuffer;
      if (_bufferQueue->pop(nextBuffer)) {
        if (!_versionNumberSource
            || nextBuffer.versionNumber > getVersionNumber()) {
          _timeStamp = nextBuffer.timeStamp;
          _value = nextBuffer.value;
          _versionNumber = nextBuffer.versionNumber;
          return true;
        } else {
          return false;
        }
      } else {
        return false;
      }
    }

    /**
     * Sends the current value to the receiver. Returns <code>true</code> if an
     * empty buffer was available and <code>false</code> if no empty buffer was
     * available and thus a previously sent value has been dropped in order to
     * send the current value.
     *
     * If this process variable has a version-number source, a new version
     * number is retrieved from this source and used for the value being sent to
     * the receiver. Otherwise, a version number of zero is used.
     *
     * Throws an exception if this process variable is not a sender.
     */
    bool send() {
      VersionNumber newVersionNumber;
      if (_versionNumberSource) {
        newVersionNumber = _versionNumberSource->nextVersionNumber();
      } else {
        newVersionNumber = 0;
      }
      return send(newVersionNumber);
    }

    /**
     * Sends the current value to the receiver. Returns <code>true</code> if an
     * empty buffer was available and <code>false</code> if no empty buffer was
     * available and thus a previously sent value has been dropped in order to
     * send the current value.
     *
     * The specified version number is passed to the receiver. If the receiver
     * has a value with a version number greater than or equal to the specified
     * version number, it silently discards this update.
     *
     * Throws an exception if this process variable is not a sender.
     */
    bool send(VersionNumber newVersionNumber) {
      if (_instanceType != SENDER) {
        throw std::logic_error(
            "Send operation is only allowed for a sender process variable.");
      }
      // Before sending the value, we have to update our time stamp.
      _timeStamp =
          _timeStampSource ?
              _timeStampSource->getCurrentTimeStamp() :
              TimeStamp::currentTime();
      Buffer nextBuffer;
      nextBuffer.timeStamp = _timeStamp;
      nextBuffer.value = _value;
      nextBuffer.versionNumber = newVersionNumber;
      _versionNumber = newVersionNumber;
      bool foundEmptyBuffer;
      if (_bufferQueue->bounded_push(nextBuffer)) {
        foundEmptyBuffer = true;
      } else {
        // We are not interested in the old value, but we have to provided
        // a reference.
        Buffer oldBuffer;
        // If we remove an element, pop returns true, otherwise the receiving
        // thread just took the last element and we can continue without
        // losing data.
        foundEmptyBuffer = !_bufferQueue->pop(oldBuffer);
        // Now we can be sure that the push is successful.
        _bufferQueue->bounded_push(nextBuffer);
      }
      if (_sendNotificationListener) {
        _sendNotificationListener->notify(_receiver);
      }
      return foundEmptyBuffer;
    }

  private:
    /**
     * Type for the individual buffers. Each buffer stores a value and a time
     * stamp.
     */
    struct Buffer {
      TimeStamp timeStamp;
      T value;
      VersionNumber versionNumber;
    };

    /**
     * Type this instance is representing.
     */
    InstanceType _instanceType;

    /**
     * Time stamp of the current value.
     */
    TimeStamp _timeStamp;

    /**
     * Current value.
     */
    T _value;

    /**
     * Version number associated with the value.
     */
    VersionNumber _versionNumber;

    /**
     * Queue holding the values that have been sent but not received yet.
     * We do not use an spsc_queue for this queue, because might want to take
     * elements from the sending thread, so there are two threads which might
     * consume elements and thus an spsc_queue is not safe.
     */
    boost::shared_ptr<boost::lockfree::queue<Buffer> > _bufferQueue;

    /**
     * Pointer to the receiver associated with this sender. This field is only
     * used if this process variable represents a sender.
     */
    boost::shared_ptr<ProcessScalar<T> > _receiver;

    /**
     * Time-stamp source used to update the time-stamp when sending a value.
     */
    boost::shared_ptr<TimeStampSource> _timeStampSource;

    /**
     * Version number source used to update the version number when setting a
     * new value.
     */
    boost::shared_ptr<VersionNumberSource> _versionNumberSource;

    /**
     * Listener that is notified when the process variable is sent.
     */
    boost::shared_ptr<ProcessVariableListener> _sendNotificationListener;

  };

  /**
   * Creates a simple process scalar. A simple process scalar just works on its
   * own and does not implement a synchronization mechanism. Apart from this,
   * it is fully functional, so that you get and set values.
   */
  template<class T>
  typename ProcessScalar<T>::SharedPtr createSimpleProcessScalar(
      const std::string & name = "", T initialValue = 0);

  /**
   * Creates a synchronized process scalar. A synchronized process scalar works
   * as a pair of two process scalars, where one process scalar acts as a sender
   * and the other one acts as a receiver.
   *
   * Changes that have been made to the sender can be sent to the receiver
   * through the {@link ProcessScalar::send()} method. The receiver can be
   * updated with these changes by calling its {@link ProcessScalar::receive()}
   * method.
   *
   * The synchronization is implemented in a thread-safe manner, so that the
   * sender and the receiver can safely be used by different threads without
   * a mutex. However, both the sender and receiver each only support a single
   * thread. This means that the sender or the receiver have to be protected
   * with a mutex if more than one thread wants to access either of them.
   *
   * The number of buffers specifies how many buffers are allocated for the
   * send / receive mechanism. The minimum number (and default) is two. This
   * number specifies, how many times {@link ProcessArray::send()} can be called
   * in a row without losing data when {@link ProcessArray::receive()} is not
   * called in between.
   *
   * The specified time-stamp source is used for determining the current time
   * when sending a value. The receiver will be updated with this time stamp
   * when receiving the value. If no time-stamp source is specified, the current
   * system-time when the value is sent is used.
   *
   * The optional send-notification listener is notified every time the sender's
   * {@link ProcessScalar::send()} method is called. It can be used to queue a
   * request for the receiver's {@link ProcessScalar::receive()} method to be
   * called.  The process variable passed to the listener is the receiver and
   * not the sender.
   */
  template<class T>
  typename std::pair<typename ProcessScalar<T>::SharedPtr,
      typename ProcessScalar<T>::SharedPtr> createSynchronizedProcessScalar(
      const std::string & name = "", T initialValue = 0,
      std::size_t numberOfBuffers = 1,
      TimeStampSource::SharedPtr timeStampSource = TimeStampSource::SharedPtr(),
      VersionNumberSource::SharedPtr versionNumberSource =
          VersionNumberSource::SharedPtr(),
      ProcessVariableListener::SharedPtr sendNotificationListener =
          ProcessVariableListener::SharedPtr());

  /**
   * A process variable which is not a sender/receiver pair but a single instance.
   * You can neither send nor receive it.
   * FIXME: This sounds pretty useless. Should we remove it?
   */
  template<class T>
  typename ProcessScalar<T>::SharedPtr createSimpleProcessScalar(
      const std::string & name, T initialValue) {
    return boost::make_shared<ProcessScalar<T> >(ProcessScalar<T>::STAND_ALONE,
        name, initialValue);
  }

  template<class T>
  typename std::pair<typename ProcessScalar<T>::SharedPtr,
      typename ProcessScalar<T>::SharedPtr> createSynchronizedProcessScalar(
      const std::string & name, T initialValue, std::size_t numberOfBuffers,
      TimeStampSource::SharedPtr timeStampSource,
      VersionNumberSource::SharedPtr versionNumberSource,
      ProcessVariableListener::SharedPtr sendNotificationListener) {
    boost::shared_ptr<ProcessScalar<T> > receiver = boost::make_shared<
        ProcessScalar<T> >(ProcessScalar<T>::RECEIVER, name, initialValue,
        numberOfBuffers, versionNumberSource);
    typename ProcessScalar<T>::SharedPtr sender = boost::make_shared<
        ProcessScalar<T> >(ProcessScalar<T>::SENDER, timeStampSource,
        versionNumberSource, sendNotificationListener, receiver);
    return std::pair<typename ProcessScalar<T>::SharedPtr,
        typename ProcessScalar<T>::SharedPtr>(sender, receiver);
  }

} // namespace ChimeraTK

#endif // CHIMERA_TK_CONTROL_SYSTEM_ADAPTER_PROCESS_SCALAR_H
