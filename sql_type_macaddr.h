#ifndef SQL_TYPE_MACADDR_INCLUDED
#define SQL_TYPE_MACADDR_INCLUDED
/* Copyright (c) 2019,2024,2025,2026 MariaDB Corporation
   Copyright (c) 2026 lefred (Frédéric Descamps)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License. */

#include "sql_type_fixedbin_storage.h"

template <size_t Bytes, bool AcceptEui48>
class Macaddr_storage: public FixedBinTypeStorage<Bytes, Bytes * 3 - 1>
{
  typedef FixedBinTypeStorage<Bytes, Bytes * 3 - 1> Base;
public:
  using Base::Base;
  bool ascii_to_fbt(const char *str, size_t length);
  size_t to_string(char *dst, size_t dstsize) const;
  static const Name &default_value();
  const uchar *bytes() const
  { return reinterpret_cast<const uchar *>(this->m_buffer); }
};

typedef Macaddr_storage<6, false> Macaddr6_storage;
typedef Macaddr_storage<8, true> Macaddr8_storage;

#include "sql_type_fixedbin.h"

typedef Type_handler_fbt<Macaddr6_storage> Type_handler_macaddr;
typedef Type_handler_fbt<Macaddr8_storage> Type_handler_macaddr8;

#endif
