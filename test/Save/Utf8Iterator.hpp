#include <cinttypes>
#include <string>

// A very simple UTF8 iterator.

class Utf8Iterator {

private:
  std::string::const_iterator mStringIterator;

  const unsigned char MASK_1 = 0x80;
  const unsigned char MASK_2 = 0x40;
  const unsigned char MASK_3 = 0x20;
  const unsigned char MASK_4 = 0x10;
  const unsigned char MASK_5 = 0x08;

public:
  Utf8Iterator(std::string::const_iterator it) : mStringIterator(it) {}

  Utf8Iterator(const Utf8Iterator &source) : mStringIterator(source.mStringIterator) {}

  Utf8Iterator &operator=(const Utf8Iterator &rhs) {
    mStringIterator = rhs.mStringIterator;
    return *this;
  }

  ~Utf8Iterator() {}

  Utf8Iterator &operator++() {
    char firstByte = *mStringIterator;

    std::string::difference_type offset = 1;

    if (firstByte & MASK_1) {
      // The first byte has a value greater than 127, so not ASCII
      if (firstByte & MASK_3) {
        // The first byte has a value greater than
        // 224, and so it must be at least a three-octet code point.
        if (firstByte & MASK_4) {
          // The first byte has a value greater than
          // 240, and so it must be a four-octet code point.
          offset = 4;
        } else offset = 3;
      } else {
        offset = 2;
      }
    }

    mStringIterator += offset;

    return *this;
  }

  Utf8Iterator operator++(int) {
    Utf8Iterator temp = *this;
    ++(*this);
    return temp;
  }

  Utf8Iterator &operator--() {
    --mStringIterator;

    if (*mStringIterator & MASK_1) {
      // The previous byte is not an ASCII character.
      --mStringIterator;
      if ((*mStringIterator & MASK_2) == 0) {
        --mStringIterator;
        if ((*mStringIterator & MASK_3) == 0) { --mStringIterator; }
      }
    }

    return *this;
  }

  Utf8Iterator operator--(int) {
    Utf8Iterator temp = *this;
    --(*this);
    return temp;
  }

  bool operator==(const Utf8Iterator &rhs) const { return mStringIterator == rhs.mStringIterator; }

  bool operator!=(const Utf8Iterator &rhs) const { return mStringIterator != rhs.mStringIterator; }

  bool operator==(std::string::iterator rhs) const { return mStringIterator == rhs; }

  bool operator==(std::string::const_iterator rhs) const { return mStringIterator == rhs; }

  bool operator!=(std::string::iterator rhs) const { return mStringIterator != rhs; }

  bool operator!=(std::string::const_iterator rhs) const { return mStringIterator != rhs; }

  char32_t operator*() const {
    char32_t chr = 0;

    char firstByte = *mStringIterator;

    if (firstByte & MASK_1) {
      // The first byte has a value greater than 127, and so is beyond the ASCII range.
      if (firstByte & MASK_3) {
        // The first byte has a value greater than
        // 191, and so it must be at least a three-octet code point.
        if (firstByte & MASK_4) {
          // The first byte has a value greater than
          // 224, and so it must be a four-octet code point.
          chr = ((firstByte & 0x07) << 18) + ((*(mStringIterator + 1) & 0x3f) << 12) +
                ((*(mStringIterator + 2) & 0x3f) << 6) + (*(mStringIterator + 3) & 0x3f);
        } else {
          chr = ((firstByte & 0x0f) << 12) + ((*(mStringIterator + 1) & 0x3f) << 6) +
                (*(mStringIterator + 2) & 0x3f);
        }
      } else {
        chr = ((firstByte & 0x1f) << 6) + (*(mStringIterator + 1) & 0x3f);
      }
    } else {
      chr = firstByte;
    }

    return chr;
  }
};
