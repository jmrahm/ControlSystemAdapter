#ifndef MTCA4U_PROCESS_SCALAR_H
#define MTCA4U_PROCESS_SCALAR_H

#include <utility>

#include <boost/smart_ptr.hpp>

#include "ProcessVariable.h"
#include "ProcessVariableListener.h"

// The impl must be incuded here because the templated inline code needs it.
#include "ProcessScalarImpl.h"

namespace mtca4u {
  /**
   * Interface implemented by all process scalars.
   *
   * Instances implementing this interface are typically not thread-safe and
   * should only be used from a single thread.
   */
  template<class T>
  class ProcessScalar: public ProcessVariable {
  public:
    /**
     * Assign the content of another process variable of type T to this one.
     * It only assigns the variable content, but not the callback functions.
     * This operator behaves like set().
     */
    ProcessScalar<T> & operator=(ProcessScalar<T> const & other) {
      this->set(other);
      return *this;
    }

    /**
     * Assign the content of the template type. This operator behaves like
     * set().
     */
    ProcessScalar<T> & operator=(T const & t) {
      this->set(t);
      return *this;
    }

    /**
     * Set the value of this process variable to the one of the other process
     * variable. This does not trigger the on-set callback function, however it
     * notifies the control system that this process variable's value has
     * changed.
     */
    void set(ProcessScalar<T> const & other){
      _processScalarImpl->set(other._processScalarImpl);
    }

    /**
     * Set the value of this process variable to the specified one. This does
     * not trigger the on-set callback function, however it notifies the control
     * system that this process variable's value has changed.
     */
    void set(T const & t){
      _processScalarImpl->set(t);      
    }

    /**
     * Automatic conversion operator which returns a \b copy of this process
     * variable's value. As no reference is returned, this cannot be used for
     * assignment.
     */
    operator T() const{
      // note: the impl, whis is always held in a shared pointer, does not need the
      // operator as it does not work directly with the pointer anyway.
      // Hence the implementation of the operator is here: just call get().
       _processScalarImpl->get();     
    }

    /**
     * Returns a \b copy of this process variable's value. As no reference is
     * returned, this cannot be used for assignment.
     */
    T get() const{
      _processScalarImpl->get();
    }

    const std::type_info& getValueType() const {
      return typeid(T);
    }

    bool isArray() const {
      return false;
    }

    bool isReceiver() const {
      return _processScalarImpl->isReceiver();
    }

    bool isSender() const {
      return _processScalarImpl->isSender();
    }

    TimeStamp getTimeStamp() const {
      return _processScalarImpl->getTimeStamp();
    }

    bool receive() {
      return _processScalarImpl->receive();
    }

    bool send() {
      return _processScalarImpl->send();
    }

  protected:
    /**
     * Shared pointer holding the actual implementation of process scalar.
     */
    boost::shared_ptr< impl::ProcessScalarImpl<T> > _processScalarImpl;

    /**
     * Creates a process scalar with the specified name.
     */
    ProcessScalar(boost::shared_ptr<impl::ProcessScalarImpl <T> > & processScalarImpl) :
      ProcessVariable(processScalarImpl->name()), _processScalarImpl(processScalarImpl) {
    }

    /**
     * Protected destructor. Instances should not be destroyed through
     * pointers to this base type.
     */
    virtual ~ProcessScalar() {
    }

  };

} //namespace


namespace mtca4u{
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
      boost::shared_ptr<TimeStampSource> timeStampSource = boost::shared_ptr<
          TimeStampSource>(),
      boost::shared_ptr<ProcessVariableListener> sendNotificationListener =
          boost::shared_ptr<ProcessVariableListener>());

} // namespace mtca4u

// ProcessScalarImpl.h must be included after the class definition and the
// template function declaration, because it depends on it.
#include "ProcessScalarImpl.h"

namespace mtca4u {

  template<class T>
  typename ProcessScalar<T>::SharedPtr createSimpleProcessScalar(
      const std::string & name, T initialValue) {
    auto theImpl = boost::make_shared<typename impl::ProcessScalarImpl<T> >(
      impl::ProcessScalarImpl<T>::STAND_ALONE, name, initialValue);
    return boost::make_shared< ProcessScalar<T> >( theImpl );
  }

  template<class T>
  typename std::pair<typename ProcessScalar<T>::SharedPtr,
      typename ProcessScalar<T>::SharedPtr> createSynchronizedProcessScalar(
      const std::string & name, T initialValue, std::size_t numberOfBuffers,
      boost::shared_ptr<TimeStampSource> timeStampSource,
      boost::shared_ptr<ProcessVariableListener> sendNotificationListener) {
    boost::shared_ptr<typename impl::ProcessScalarImpl<T> > receiver =
        boost::make_shared<typename impl::ProcessScalarImpl<T> >(
            impl::ProcessScalarImpl<T>::RECEIVER, name, initialValue,
            numberOfBuffers);
    typename ProcessScalar<T>::SharedPtr sender = boost::make_shared<
        typename impl::ProcessScalarImpl<T> >(
        impl::ProcessScalarImpl<T>::SENDER, timeStampSource,
        sendNotificationListener, receiver);
    return std::make_pair( boost::make_shared< ProcessScalar<T> >(sender),
			   boost::make_shared< ProcessScalar<T> >(receiver) );
  }

} // namespace mtca4u

#endif // MTCA4U_PROCESS_SCALAR_H
