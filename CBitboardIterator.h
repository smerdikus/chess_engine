//
// Created by Petr Smerda on 24.08.2024.
//

#ifndef SFML_CHESS_CBITBOARDITERATOR_H
#define SFML_CHESS_CBITBOARDITERATOR_H
#include <cstdint>
#include <iterator>

class CBitboardIterator {
public:
  using value_type = uint64_t;
  using pointer = const value_type*;
  using reference = const value_type&;
  using iterator_category = std::input_iterator_tag;
  using difference_type = std::ptrdiff_t;

  CBitboardIterator(uint64_t bitboard) : bitboard_(bitboard), current_(0) {
    advance();
  }

  CBitboardIterator& operator++() {
    advance();
    return *this;
  }

  CBitboardIterator operator++(int) {
    CBitboardIterator tmp = *this;
    advance();
    return tmp;
  }

  bool operator==(const CBitboardIterator& other) const {
    return bitboard_ == other.bitboard_ && current_ == other.current_;
  }

  bool operator!=(const CBitboardIterator& other) const {
    return !(*this == other);
  }

  reference operator*() const {
    return current_;
  }

private:
  void advance() {
    if (bitboard_ != 0) {
      current_ = bitboard_ & -bitboard_;
      bitboard_ &= bitboard_ - 1;
    } else {
      current_ = 0;
    }
  }

  uint64_t bitboard_;
  uint64_t current_;
};

class CBitboardRange {
public:
  CBitboardRange(uint64_t bitboard) : bitboard_(bitboard) {}

  CBitboardIterator begin() const {
    return CBitboardIterator(bitboard_);
  }

  CBitboardIterator end() const {
    return CBitboardIterator(0);
  }

private:
  uint64_t bitboard_;
};


#endif //SFML_CHESS_CBITBOARDITERATOR_H
