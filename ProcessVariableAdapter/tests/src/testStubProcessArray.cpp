#include <sstream>
#include <algorithm>

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "StubProcessArray.h"

namespace mtca4u{
/** The test class for the StubProcessArray.
 *  It is templated to be tested with all data types.
 */
template<class T>
class StubProcessArrayTest
{
 public:
  StubProcessArrayTest();
  static void testConstructors();
//  void testAssignment();
//  void testOnSetCallbackFunction();
//  void testOnGetCallbackFunction();
//  void testSetters();
//  void testSetWithoutCallback();
//  void testEquals();
void testIterators();

 private:
//  unsigned int _callbackCounter;
//  unsigned int _callbackWithEqualValuesCounter;
//  unsigned int _getCounter;
//  mtca4u::StubProcessArray<T> _processArray;
  std::vector<T> _container;
  //  void set(T const & newValue, T const & oldValue);
  //  T get();
};

/** The boost test suite which executes the StubProcessArrayTest.
 */
template <class T>
class StubProcessArrayTestSuite : public test_suite {
public:
  StubProcessArrayTestSuite(): test_suite("StubProcessArray test suite") {
    boost::shared_ptr< StubProcessArrayTest<T> > 
      processArrayTest( new StubProcessArrayTest<T> );

    add( BOOST_TEST_CASE( &StubProcessArrayTest<T>::testConstructors ) );

    add( BOOST_CLASS_TEST_CASE( &StubProcessArrayTest<T>::testIterators,
				processArrayTest ) );
//    add( BOOST_CLASS_TEST_CASE( &StubProcessArrayTest<T>::testOnSetCallbackFunction,
//				processArrayTest ) );
//    add( BOOST_CLASS_TEST_CASE( &StubProcessArrayTest<T>::testOnGetCallbackFunction,
//				processArrayTest ) );
//    add( BOOST_CLASS_TEST_CASE( &StubProcessArrayTest<T>::testSetters,
//				processArrayTest ) );
//    add( BOOST_CLASS_TEST_CASE( &StubProcessArrayTest<T>::testSetWithoutCallback,
//				processArrayTest ) );
//    add( BOOST_CLASS_TEST_CASE( &StubProcessArrayTest<T>::testEquals,
//				processArrayTest ) );
//    add( BOOST_CLASS_TEST_CASE( &StubProcessArrayTest<T>::testConversionOperator,
//				processArrayTest ) );

  }
};

template <class T>
StubProcessArrayTest<T>::StubProcessArrayTest() 
//  : _callbackCounter(0),  _callbackWithEqualValuesCounter(0), _getCounter(0),
//    _processT(0), _t(0){
{
}

template <class T>
void StubProcessArrayTest<T>::testConstructors(){
  mtca4u::StubProcessArray<T> processA1(12);

  //  mtca4u::StubProcessArray<T> processT2(2);
  //  BOOST_CHECK( processT2 == 2 );
  
  // copy constructor is private, object cannot be copied
  //mtca4u::StubProcessArray<T> processT3( processT2 );
  //  BOOST_CHECK( processT3 == 2 );
}

template <class T>
void StubProcessArrayTest<T>::testIterators(){
  mtca4u::StubProcessArray<T> processArray(12);
  int i =42;
  for (typename mtca4u::ProcessArray<T>::iterator it= processArray.begin();
       it != processArray.end(); ++it){
    *it=i++;
  }

  // use the fact that we re friend and can directly access the underlying vector to verify
  for (size_t j = 0; j < processArray._container.size(); ++j){
    BOOST_CHECK( processArray._container[j] == static_cast<T>(j+42) );
  }

  //constant array with begin and end
  mtca4u::StubProcessArray<T> const & constArray = processArray;
  i=42;
  for (typename mtca4u::ProcessArray<T>::const_iterator it= constArray.begin();
       it != constArray.end(); ++it){
    BOOST_CHECK( *it == i++ );    
  }
  
  //non-constant array with const_iterator
  i=42;
  for (typename mtca4u::ProcessArray<T>::const_iterator it= processArray.cbegin();
       it != processArray.cend(); ++it){
    BOOST_CHECK( *it == i++ );    
  }

  // check that the iterators run with std algorithms
  std::sort( processArray.rbegin(), processArray.rend() );

  // again use the container to check on the modifications
  i=42+processArray._container.size();
  for (size_t j = 0; j < processArray._container.size(); ++j){
    BOOST_CHECK( processArray._container[j] == static_cast<T>(--i) );
  }
 
  //constant array with rbegin and rend
  i=42;
  for (typename mtca4u::ProcessArray<T>::const_reverse_iterator it= constArray.rbegin();
       it != constArray.rend(); ++it){
    BOOST_CHECK( *it == i++ );    
  }
  
  //non-constant array with const_iterator
  i=42;
  for (typename mtca4u::ProcessArray<T>::const_reverse_iterator it= processArray.crbegin();
       it != processArray.crend(); ++it){
    BOOST_CHECK( *it == i++ );    
  }
}


//template <class T>
//void StubProcessArrayTest<T>::testAssignment(){
//  _processT = 3;
//  BOOST_CHECK( _processT == 3 );
//  
//  mtca4u::StubProcessArray<T> processT2(2);
//  _processT = processT2;
//  BOOST_CHECK( _processT == 2 );
//
//  // test self assigmnent, nothing should happen
//  _processT = _processT;
//  BOOST_CHECK( _processT == 2 );
//
//  mtca4u::ProcessArray<T> * processArrayPointer = &processT2;
//  processT2 = 17;
//  _processT = *processArrayPointer;
//  BOOST_CHECK( _processT == 17 );
//}
//
//template <class T>
//void StubProcessArrayTest<T>::set(T const & newValue, T const & oldValue){
//  if (newValue == oldValue){
//    ++_callbackWithEqualValuesCounter;
//  }
//  _t = newValue;
//  ++_callbackCounter;
//}
//
//template <class T>
//void StubProcessArrayTest<T>::testOnSetCallbackFunction(){
//  _processT.setOnSetCallbackFunction( boost::bind( &StubProcessArrayTest<T>::set,
//						   this, _1, _2 ) );
//  _processT.set(5);
//  BOOST_CHECK( _t == 5 );
//  BOOST_CHECK( _callbackCounter == 1 );
//  BOOST_CHECK( _callbackWithEqualValuesCounter == 0 );
//
//  _processT.clearOnSetCallbackFunction();
//  _processT.set(6);
//  BOOST_CHECK( _t == 5 );
//  BOOST_CHECK( _callbackCounter == 1 );
//  BOOST_CHECK( _callbackWithEqualValuesCounter == 0 );
//
//  // turn on the callback again, we use it to check the other operators
//  _processT.setOnSetCallbackFunction( boost::bind( &StubProcessArrayTest<T>::set,
//						   this, _1, _2 ) );  
//
//  // test same value assignment. Callback must be triggered
//  _processT.set(6);
//  BOOST_CHECK( _t == 6 );
//  BOOST_CHECK( _callbackCounter == 2 );
//  BOOST_CHECK( _callbackWithEqualValuesCounter == 1 );
//
//  // test self assignment. Callback should be triggered even now
//  _processT.set(_processT);
//  BOOST_CHECK( _t == 6 );
//  BOOST_CHECK( _callbackCounter == 3 );
//  BOOST_CHECK( _callbackWithEqualValuesCounter == 2 );
//}
//
//template <class T>
//T StubProcessArrayTest<T>::get(){
//  ++_getCounter;
//  return _t;
//}
//
//template <class T>
//void StubProcessArrayTest<T>::testOnGetCallbackFunction(){
//  _processT.setOnGetCallbackFunction( boost::bind( &StubProcessArrayTest<T>::get, this ) );
//  
//  BOOST_CHECK( _processT == 6 );
//  _t = 19;
//  BOOST_CHECK( _processT.get() == 19 );
//  BOOST_CHECK( _processT == 19 );
//  BOOST_CHECK( _getCounter == 1 );
//
//  _t = 17;
//
//  _processT.clearOnGetCallbackFunction();
//  BOOST_CHECK( _processT.get() == 19 );
//  BOOST_CHECK( _processT == 19 );
//  BOOST_CHECK( _getCounter == 1 );
//}
//
//template <class T>
//void StubProcessArrayTest<T>::testSetters(){
//  _processT.set( 7 );
//  BOOST_CHECK( _processT == 7 );
//  BOOST_CHECK( _t == 7 );
//  BOOST_CHECK( _callbackCounter == 4 );
//
//  mtca4u::StubProcessArray<T> processT1(8);
//  _processT.set( processT1 );
//  BOOST_CHECK( _processT == 8 );
//  BOOST_CHECK( _t == 8 );
//  BOOST_CHECK( _callbackCounter == 5 );
//
//  processT1=88;
//  mtca4u::ProcessArray<T> * processArrayPointer = &processT1;
//  _processT.set( *processArrayPointer );
//  BOOST_CHECK( _processT == 88 );
//  BOOST_CHECK( _t == 88 );
//  BOOST_CHECK( _callbackCounter == 6 );
//  
// 
//}
//
//template <class T>
//void StubProcessArrayTest<T>::testSetWithoutCallback(){
//  _processT.setWithoutCallback( 9 );
//  BOOST_CHECK( _processT == 9 );
//  BOOST_CHECK( _t == 88 );
//  BOOST_CHECK( _callbackCounter == 6 );
//
//  mtca4u::StubProcessArray<T> processT1(10);
//  _processT.setWithoutCallback( processT1 );
//  BOOST_CHECK( _processT == 10 );
//  BOOST_CHECK( _t == 88 );
//  BOOST_CHECK( _callbackCounter == 6 );
//  
//  // test self asignment, nothing should happen
//   _processT.setWithoutCallback( _processT );
//  BOOST_CHECK( _processT == 10 );
//  BOOST_CHECK( _t == 88 );
//  BOOST_CHECK( _callbackCounter == 6 );
//}
//
//template <class T>
//void StubProcessArrayTest<T>::testEquals(){
//  // we are using this all along. It uses the implicit conversion to T
//  _processT = 11;
//  BOOST_CHECK( _processT == 11 );
// 
//  // we can even compare two processarrays like this. Note that only the
//  // content of the member T is compared, not the callback functions.
//  mtca4u::StubProcessArray<T> processT1(12);
//  _processT.set( processT1 );
//  BOOST_CHECK( _processT ==  processT1 );
//}
//
//template <class T>
//void StubProcessArrayTest<T>::testConversionOperator(){
//  T a = _processT;
//  BOOST_CHECK( a == 12 );
//  BOOST_CHECK( a == _processT );
//  mtca4u::StubProcessArray<T> processT1(4);
//  BOOST_CHECK( _processT * processT1 == 48 );
//  BOOST_CHECK( 2.5 * processT1 == 10. );
//  _processT.set(_processT/3);
//  // compare to double. Implicit use if comparison (int, double)
//  BOOST_CHECK( _processT == 4. );
//  BOOST_CHECK( _t == 4. );
//
//  // This does not and should not compile. The implicit conversion returns a const
//  // reference so the callback is not bypassed. And there is no *= operator defined.
//  //  _processT *= 2;
//}

}//namespace mtca4u

test_suite*
init_unit_test_suite( int /*argc*/, char* /*argv*/ [] )
{
  framework::master_test_suite().p_name.value = "StubProcessArray test suite";

  framework::master_test_suite().add( new mtca4u::StubProcessArrayTestSuite<int32_t> );
//  framework::master_test_suite().add( new mtca4u::StubProcessArrayTestSuite<uint32_t> );
//  framework::master_test_suite().add( new mtca4u::StubProcessArrayTestSuite<int16_t> );
//  framework::master_test_suite().add( new mtca4u::StubProcessArrayTestSuite<uint16_t> );
//  framework::master_test_suite().add( new mtca4u::StubProcessArrayTestSuite<int8_t> );
//  framework::master_test_suite().add( new mtca4u::StubProcessArrayTestSuite<uint8_t> );
//  framework::master_test_suite().add( new mtca4u::StubProcessArrayTestSuite<double>);
//  framework::master_test_suite().add( new mtca4u::StubProcessArrayTestSuite<float>);

  return NULL;
}

