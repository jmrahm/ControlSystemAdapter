#ifndef CHIMERA_TK_CONTROL_SYSTEM_ADAPTER_PROCESS_VARIABLE_H
#define CHIMERA_TK_CONTROL_SYSTEM_ADAPTER_PROCESS_VARIABLE_H

namespace ChimeraTK {
  class ProcessVariable;
}

#include <string>
#include <typeinfo>

#include <boost/smart_ptr.hpp>

#include "ProcessVariableDecl.h"
#include "TimeStamp.h"
#include "VersionNumber.h"

namespace ChimeraTK {

  /**
   * Base class for all {@link ProcessScalar}s and {@link ProcessArray}s.
   * This class provides methods that are independent of the actual type or
   * implementation of a process variable.
   */
  class ProcessVariable: public boost::enable_shared_from_this<ProcessVariable> {
  public:
    /**
     * Type alias for a shared pointer to a process variable.
     */
    typedef boost::shared_ptr<ProcessVariable> SharedPtr;

    /**
     * Returns the name that identifies the process variable.
     */
    const std::string& getName() const;

    /**
     * Returns the \c std::type_info for the value type of this process
     * variable or array. This can be used to determine the type of a process
     * variable at runtime.
     */
    virtual const std::type_info& getValueType() const =0;

    /**
     * Returns the version number that is associated with the current value.
     * The version number is used to resolve conflicting updates. An update
     * (e.g. received using the {@link receive()} method) is only used if its
     * value has a version number that is greater than the version number of the
     * current value. Initially, each process variable has a version number of
     * zero.
     */
    virtual VersionNumber getVersionNumber() const =0;

    /**
     * Returns <code>true</code> if this object is an instance of
     * {@link ProcessArray}\<T\> and <code>false</code> if this object is an
     * instance of {@link ProcessScalar}\<T\>.
     */
    virtual bool isArray() const =0;

    /**
     * Returns <code>true</code> if this object represents the receiver in a
     * sender / receiver pair, <code>false</code> otherwise.
     */
    virtual bool isReceiver() const =0;

    /**
     * Returns <code>true</code> if this object represents the sender in a
     * sender / receiver pair, <code>false</code> otherwise.
     */
    virtual bool isSender() const =0;

    /**
     * Returns the time stamp associated with the current value of the process
     * variable. Typically, this is the time when the value was updated.
     */
    virtual TimeStamp getTimeStamp() const =0;

    /**
     * Receives a new value from the sender if this process variable is a
     * receiver. Returns <code>true</code> if a new value has been received and
     * <code>false</code> if there is no new value available.
     *
     * Throws an exception if this process variable is not a receiver.
     */
    virtual bool receive() =0;

    /**
     * Sends the current value to the receiver if this process variable is a
     * sender. Returns <code>true</code> if an empty buffer was available and
     * <code>false</code> if no empty buffer was available and thus a previously
     * sent value has been dropped in order to send the current value.
     *
     * Throws an exception if this process variable is not a sender.
     */
    virtual bool send() =0;

    /**
     * Forces the next {@link send()} operation to use the specified version
     * number instead of retrieving a new version number from the version number
     * source.
     *
     * When the version number of this process variable's current value is less
     * than the specified version number, the specified version number is used
     * for the next <code>send()</code> operation and this method returns
     * <code>true</code>. Otherwise, the version number used for the next
     * <code>send()</code> operation is not touched and this method returns
     * <code>false</code>.
     *
     * When updating this process variable with a value that is based on the
     * value of another process variable that has just been received, this
     * method should be called before modifying this process variable's value
     * and the value should only be modified if this method returns
     * <code>true</code>. This way, process variables that depend on each other
     * in a circular way will only be updated once in the case of an externally
     * triggered update instead of ending up in an infinite update loop.
     *
     * The version number set by this method is only effective for the next
     * <code>send()</code> operation and is also the effective version number
     * used for this process variable (e.g. returned by
     * {@link getVersionNumber()}) until then. If a new value with a version
     * number greater than the version number set by this method is received for
     * this process variable (by calling {@link receive()}) before calling
     * <code>send()</code>, the version number of this process variable is set
     * to the version number of the received value and the next send() operation
     * falls back to the default behavior of retrieving a new version number
     * from the version number source.
     */
    virtual bool useOriginVersionNumberForNextSend(
        VersionNumber originalVersionNumber) =0;

    /**
     * Forces the next {@link send()} operation to use the version number from
     * the specified process variable instead of retrieving a new version number
     * from the version number source.
     *
     * When the version number of this process variable's current value is less
     * than the version number associated with the specified process variable's
     * current value, the version number of the specified process variable is
     * used for the next <code>send()</code> operation and this method returns
     * <code>true</code>. Otherwise, the version number used for the next
     * <code>send()</code> operation is not touched and this method returns
     * <code>false</code>.
     *
     * When updating this process variable with a value that is based on the
     * value of another process variable that has just been received, this
     * method should be called before modifying this process variable's value
     * and the value should only be modified if this method returns
     * <code>true</code>. This way, process variables that depend on each other
     * in a circular way will only be updated once in the case of an externally
     * triggered update instead of ending up in an infinite update loop.
     *
     * The version number set by this method is only effective for the next
     * <code>send()</code> operation and is also the effective version number
     * used for this process variable (e.g. returned by
     * {@link getVersionNumber()}) until then. If a new value with a version
     * number greater than the version number set by this method is received for
     * this process variable (by calling {@link receive()}) before calling
     * <code>send()</code>, the version number of this process variable is set
     * to the version number of the received value and the next send() operation
     * falls back to the default behavior of retrieving a new version number
     * from the version number source.
     *
     * This method is only supplied for convenience and is equivalent to
     * <code>(originalProcessVariable) ? useOriginVersionNumberForNextSend(originalProcessVariable->getVersionNumber()) : false</code>.
     */
    inline bool useOriginVersionNumberForNextSend(
        ProcessVariable::SharedPtr originalProcessVariable) {
      return
          (originalProcessVariable) ?
              useOriginVersionNumberForNextSend(
                  originalProcessVariable->getVersionNumber()) :
              false;
    }

  protected:
    /**
     * Creates a process variable with the specified name.
     */
    ProcessVariable(const std::string& name = std::string());

    /**
     * Virtual destructor. The destructor is protected so that an instance
     * cannot be destroyed through a pointer to this interface.
     */
    virtual ~ProcessVariable();

  private:
    /**
     * Identifier uniquely identifying the process variable.
     */
    std::string _name;

    // Theoretically, the copy constructor and copy assignment operator should
    // not be generated if a destructor is defined. However, some compilers do
    // not enforce this. By making the constructor and operator private, we
    // ensure that a process variable is never copied accidentally. This also
    // applies to derived classes (the automatically generated implementations
    // need the implementation in the parent class), so that it is sufficient
    // if we do it once here.
    ProcessVariable(const ProcessVariable&);
    ProcessVariable& operator=(const ProcessVariable&);

  };

}

#endif // CHIMERA_TK_CONTROL_SYSTEM_ADAPTER_PROCESS_VARIABLE_H
