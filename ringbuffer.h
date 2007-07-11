#ifndef RINGBUFFER_H
#define RINGBUFFER_H

template <class T, int potSize = 12> class RingBuffer {

public:
  static const int size = 1 << potSize;

  RingBuffer()
    : _writeAt(0)
    , _readAt(0)
    , _count(0)
  {}

  int writeAt() {
    return _writeAt;
  }

  void put(const T &e) {
    d[_writeAt] = e;
    _writeAt = (_writeAt + 1) & (size - 1);
    q_atomic_increment(&_count);
  }

  T &get() {
    q_atomic_decrement(&_count);
    T &r = d[_readAt];
    _readAt = (_readAt + 1) & (size - 1);
    return r;
  }

  int count() {
    return _count;
  }

protected:
  T d[size];

  int _writeAt;
  int _readAt;
  int _count;

};

#endif

