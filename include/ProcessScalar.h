#ifndef MTCA4U_PROCESS_SCALAR_H
#define MTCA4U_PROCESS_SCALAR_H

#include <utility>

#include <boost/smart_ptr.hpp>

#include "ProcessVariable.h"
#include "ProcessVariableListener.h"

namespace mtca4u {
  /**
   * Interface implemented by all process scalars.
   *
   * Instances implementing this interface are typically not thread-safe and
   * should only be used from a single thread.
   */
  template<class T>
  class ProcessScalarHowItLookedLike: public ProcessVariable {
  public:
    /**
     * Assign the content of another process variable of type T to this one.
     * It only assigns the variable content, but not the callback functions.
     * This operator behaves like set().
     */
    //    ProcessScalar<T> & operator=(ProcessScalar<T> const & other) {
    //      this->set(other);
    //      return *this;
    //    }

    /**
     * Assign the content of the template type. This operator behaves like
     * set().
     */
    //    ProcessScalar<T> & operator=(T const & t) {
    //      this->set(t);
    //      return *this;
    //    }

    /**
     * Set the value of this process variable to the one of the other process
     * variable. This does not trigger the on-set callback function, however it
     * notifies the control system that this process variable's value has
     * changed.
     */
    //    virtual void set(ProcessScalar<T> const & other)=0;

    /**
     * Set the value of this process variable to the specified one. This does
     * not trigger the on-set callback function, however it notifies the control
     * system that this process variable's value has changed.
     */
    //virtual void set(T const & t)=0;

    /**
     * Automatic conversion operator which returns a \b copy of this process
     * variable's value. As no reference is returned, this cannot be used for
     * assignment.
     */
    //virtual operator T() const =0;

    /**
     * Returns a \b copy of this process variable's value. As no reference is
     * returned, this cannot be used for assignment.
     */
    //virtual T get() const =0;

    //    const std::type_info& getValueType() const {
    //      return typeid(T);
    //    }

    //    bool isArray() const {
    //      return false;
    //    }

  protected:
    /**
     * Creates a process scalar with the specified name.
     */
    //ProcessScalar(const std::string& name = std::string()) :
    //        ProcessVariable(name) {
    //    }

    /**
     * Protected destructor. Instances should not be destroyed through
     * pointers to this base type.
     */
    //    virtual ~ProcessScalar() {
    //    }

  };

} //namespace

// FIXME ProcessScalarImpl.h must be included before defining the creator functions, because
// these have been hacked to return the impl.
#include "ProcessScalarImpl.h"

namespace mtca4u{
  // for the transition phase ProcessScalar and Impl are the same
  template<class T>
    using ProcessScalar = impl::ProcessScalarImpl<T>;

 template<class T>
   typename std::pair<typename ProcessScalar<T>::SharedPtr,
   typename ProcessScalar<T>::SharedPtr >createSynchronizedProcessScalar(
      const std::string & name = "", T initialValue = 0,
      std::size_t numberOfBuffers = 1,
      boost::shared_ptr<TimeStampSource> timeStampSource = boost::shared_ptr<
          TimeStampSource>(),
      boost::shared_ptr<ProcessVariableListener> sendNotificationListener =
          boost::shared_ptr<ProcessVariableListener>()){
     return impl::createSynchronizedProcessScalar( name, initialValue,
						   numberOfBuffers, timeStampSource,
						   sendNotificationListener );
   }
 
 template<class T>
   typename ProcessScalar<T>::SharedPtr createSimpleProcessScalar(
      const std::string & name = "", T initialValue = 0) {
   return impl::createSimpleProcessScalar(name, initialValue);
 }
} // namespace mtca4u

#endif // MTCA4U_PROCESS_SCALAR_H
