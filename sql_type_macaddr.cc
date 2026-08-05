/* Copyright (c) 2019,2024,2025,2026 MariaDB Corporation
   Copyright (c) 2026 lefred (Frédéric Descamps)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License. */

#define MYSQL_SERVER
#include "mariadb.h"
#include "sql_class.h"
#include "sql_type_macaddr.h"

static int hex_value(char c)
{
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

template <size_t Bytes, bool AcceptEui48>
bool Macaddr_storage<Bytes, AcceptEui48>::ascii_to_fbt(const char *input,
                                                       size_t length)
{
  while (length && my_isspace(&my_charset_latin1, *input))
  { ++input; --length; }
  while (length && my_isspace(&my_charset_latin1, input[length - 1]))
    --length;

  char digits[16];
  size_t count= 0;
  char separator= 0;
  /*
    Track the length (in hex digits) of each separator-delimited group, so
    the groups can be checked against the fixed set of forms PostgreSQL
    documents for macaddr/macaddr8, instead of accepting a separator at any
    even digit boundary. group_len[16] is generous: a valid group is never
    shorter than 2 digits, and a bogus run of bare separators is rejected
    once it overflows the array.
  */
  size_t group_len[16];
  size_t n_groups= 0;
  size_t cur_group= 0;
  for (size_t i= 0; i < length; ++i)
  {
    if (hex_value(input[i]) >= 0)
    {
      if (count >= sizeof(digits)) goto error;
      digits[count++]= input[i];
      ++cur_group;
      continue;
    }
    if (input[i] != ':' && input[i] != '-' && input[i] != '.') goto error;
    if (!separator) separator= input[i];
    else if (separator != input[i]) goto error;
    if (n_groups >= array_elements(group_len)) goto error;
    group_len[n_groups++]= cur_group;
    cur_group= 0;
  }
  if (n_groups >= array_elements(group_len)) goto error;
  group_len[n_groups++]= cur_group;

  if (count != Bytes * 2 && !(AcceptEui48 && count == 12)) goto error;

  if (separator == '.')
  {
    /* Cisco dotted form: groups of 4 hex digits, e.g. "0800.2b01.0203". */
    for (size_t i= 0; i < n_groups; ++i)
      if (group_len[i] != 4) goto error;
  }
  else if (separator)
  {
    /*
      ':' or '-' separated form: either every group is a single hex byte
      (e.g. "08:00:2b:01:02:03"), or there are exactly two groups, an OUI
      of 3 bytes followed by the remaining bytes as one group
      (e.g. "08002b:010203", "08002b-0102030405").
    */
    bool all_pairs= true;
    for (size_t i= 0; i < n_groups; ++i)
      if (group_len[i] != 2) { all_pairs= false; break; }
    if (!all_pairs && !(n_groups == 2 && group_len[0] == 6))
      goto error;
  }
  /* No separator at all (e.g. "08002b010203") needs no further check. */

  uchar parsed[8];
  for (size_t i= 0; i < count / 2; ++i)
    parsed[i]= static_cast<uchar>((hex_value(digits[i * 2]) << 4) |
                                  hex_value(digits[i * 2 + 1]));
  if (AcceptEui48 && count == 12)
  {
    memcpy(this->m_buffer, parsed, 3);
    this->m_buffer[3]= static_cast<char>(0xff);
    this->m_buffer[4]= static_cast<char>(0xfe);
    memcpy(this->m_buffer + 5, parsed + 3, 3);
  }
  else
    memcpy(this->m_buffer, parsed, Bytes);
  return false;
error:
  bzero(this->m_buffer, Bytes);
  return true;
}

template <size_t Bytes, bool AcceptEui48>
size_t Macaddr_storage<Bytes, AcceptEui48>::to_string(char *dst,
                                                      size_t dstsize) const
{
  static const char hex[]= "0123456789abcdef";
  if (dstsize < Bytes * 3) return 0;
  size_t pos= 0;
  for (size_t i= 0; i < Bytes; ++i)
  {
    if (i) dst[pos++]= ':';
    uchar byte= static_cast<uchar>(this->m_buffer[i]);
    dst[pos++]= hex[byte >> 4];
    dst[pos++]= hex[byte & 15];
  }
  dst[pos]= '\0';
  return pos;
}

template <size_t Bytes, bool AcceptEui48>
const Name &Macaddr_storage<Bytes, AcceptEui48>::default_value()
{
  static Name mac6(STRING_WITH_LEN("00:00:00:00:00:00"));
  static Name mac8(STRING_WITH_LEN("00:00:00:00:00:00:00:00"));
  return Bytes == 6 ? mac6 : mac8;
}

template class Macaddr_storage<6, false>;
template class Macaddr_storage<8, true>;
