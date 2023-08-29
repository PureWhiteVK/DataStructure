#pragma once

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <iostream>
#include <unordered_map>

// for stable check
class Integer {
public:
  Integer() {}
  ~Integer() {}

  Integer(int v) {
    m_value = v;
    set_uuid();
  }

  Integer &operator=(int v) {
    m_value = v;
    set_uuid();
    return *this;
  }

  Integer(const Integer &v) {
    m_value = v;
    m_uuid = v.m_uuid;
  }

  Integer &operator=(const Integer &v) {
    m_value = v.m_value;
    m_uuid = v.m_uuid;
    return *this;
  }
  
  Integer(Integer &&v) {
    m_value = v;
    m_uuid = v.m_uuid;
    v.m_value = 0;
    v.m_uuid = INVALID_ID;
  }

  Integer &operator=(Integer &&v) {
    m_value = v.m_value;
    m_uuid = v.m_uuid;
    v.m_value = 0;
    v.m_uuid = INVALID_ID;
    return *this;
  }

  operator int() const { return m_value; }

  bool operator<(const Integer &i) const { return m_value < i.m_value; }
  bool operator<=(const Integer &i) const { return m_value <= i.m_value; }
  bool operator>(const Integer &i) const { return m_value > i.m_value; }
  bool operator>=(const Integer &i) const { return m_value >= i.m_value; }
  bool operator==(const Integer &i) const {
    return m_value == i.m_value && m_uuid == i.m_uuid;
  }

  friend std::ostream &operator<<(std::ostream &os, const Integer &v) {
    os << fmt::format("{}[{:#016x}]", v.m_value, v.m_uuid);
    return os;
  }

private:
  void set_uuid() { m_uuid = reinterpret_cast<uintptr_t>(this); }

private:
  static constexpr int INVALID_ID = 0;
  int m_value{};
  uint64_t m_uuid{};
};
