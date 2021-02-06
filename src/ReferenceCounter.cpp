
#include "ReferenceCounter.h"


namespace Stewardess
{

  ReferenceCounter::ReferenceCounter() :
    _data( new ReferenceData() )
  {
  }


  ReferenceCounter::~ReferenceCounter()
  {
    if ( _data != nullptr )
    {
      this->_checkSubtract();
    }
  }


  ReferenceCounter::ReferenceCounter( const ReferenceCounter& other ) :
    _data( nullptr )
  {
    _data = other._data;
    if ( _data )
    {
      _data->references += 1;
    }
  }


  ReferenceCounter::ReferenceCounter( ReferenceCounter&& other ) :
    _data( nullptr )
  {
    _data = std::exchange( other._data, this->_data ) ;
  }


  ReferenceCounter& ReferenceCounter::operator=( const ReferenceCounter& other )
  {
    if ( _data )
    {
      this->_checkSubtract();
    }

    _data = other._data;

    if ( _data )
    {
      _data->references += 1;
    }

    return *this;
  }


  ReferenceCounter& ReferenceCounter::operator=( ReferenceCounter&& other )
  {
    if ( _data )
    {
      this->_checkSubtract();
    }

    _data = other._data;

    other._data = nullptr;

    return *this;
  }


  void ReferenceCounter::_checkSubtract()
  {
    // If zero, delete
    if ( _data->references == 0 )
    {
      delete _data;
      _data = nullptr;
    }
    else
    {
      _data->references -= 1;
    }
  }


  size_t ReferenceCounter::getNumber() const
  {
    if ( _data )
    {
      return _data->references;
    }
    else
    {
      return -1;
    }
  }


  ReferenceCounter::operator bool() const
  {
    if ( _data )
    {
      return true;
    }
    else
    {
      return false;
    }
  }

}

