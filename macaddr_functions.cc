/* Copyright (c) 2019,2024,2025,2026 MariaDB Corporation
   Copyright (c) 2026 lefred (Frédéric Descamps)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License. */

#define MYSQL_SERVER
#include "mariadb.h"
#include "sql_class.h"
#include "item_func.h"
#include "sql_type_macaddr.h"
#include "macaddr_functions.h"

/*
  Delegate to Handler::Fbt_null(Item*), the same conversion path used by
  CAST(... AS MACADDR[8]). This covers the native fast path (arg already of
  the macaddr type), the text/hex parse path, and the raw-binary path
  (a BINARY/VARBINARY/X'...' argument of exactly the right byte length is
  copied as-is, not re-parsed as hex text) identically to the type cast.
*/
template <class Storage, class Handler>
static bool value_from_item(Item *item, Storage *value)
{
  typename Handler::Fbt_null parsed(item);
  if (parsed.is_null())
    return true;
  NativeBuffer<Storage::binary_length() + 1> native;
  if (parsed.to_fbt().to_native(&native)) return true;
  *value= Storage(native.ptr(), native.length());
  return false;
}

template <class Storage, class Handler, size_t Keep, bool Set7Bit>
class Item_func_macaddr_result: public Handler::Item_fbt_func
{
  typedef typename Handler::Item_fbt_func Base;
  LEX_CSTRING function_name;
  bool calculate(Storage *result)
  {
    Storage input;
    if (value_from_item<Storage, Handler>(Base::args[0], &input)) return true;
    char native[Storage::binary_length()];
    memcpy(native, input.bytes(), sizeof(native));
    if (Set7Bit) native[0]= static_cast<char>(native[0] | 0x02);
    else bzero(native + Keep, sizeof(native) - Keep);
    *result= Storage(native, sizeof(native));
    return false;
  }
public:
  Item_func_macaddr_result(THD *thd, Item *arg, LEX_CSTRING name)
    : Base(thd, arg), function_name(name) {}
  bool fix_length_and_dec(THD *thd) override
  {
    Base::fix_length_and_dec(thd);
    Base::set_maybe_null();
    return false;
  }
  bool val_native(THD *, Native *to) override
  {
    Storage value;
    return Base::null_value= calculate(&value) ||
      to->copy(value.to_lex_cstring().str, Storage::binary_length());
  }
  String *val_str(String *to) override
  {
    Storage value;
    if ((Base::null_value= calculate(&value))) return nullptr;
    char text[Storage::max_char_length() + 1];
    size_t length= value.to_string(text, sizeof(text));
    if (!length || to->copy(text, length, &my_charset_latin1))
    {
      Base::null_value= true;
      return nullptr;
    }
    return to;
  }
  LEX_CSTRING func_name_cstring() const override { return function_name; }
  Item *shallow_copy(THD *thd) const override
  { return get_item_copy<Item_func_macaddr_result>(thd, this); }
};

typedef Item_func_macaddr_result<Macaddr6_storage, Type_handler_macaddr, 3,
                                 false> Item_func_macaddr_trunc;
typedef Item_func_macaddr_result<Macaddr8_storage, Type_handler_macaddr8, 3,
                                 false> Item_func_macaddr8_trunc;
typedef Item_func_macaddr_result<Macaddr8_storage, Type_handler_macaddr8, 8,
                                 true> Item_func_macaddr8_set7bit;

template <class ItemType>
class Create_macaddr_func: public Create_func_arg1
{
  LEX_CSTRING name;
public:
  explicit Create_macaddr_func(LEX_CSTRING name_arg): name(name_arg) {}
  Item *create_1_arg(THD *thd, Item *arg) override
  { return new (thd->mem_root) ItemType(thd, arg, name); }
};

static Create_macaddr_func<Item_func_macaddr_trunc>
  create_macaddr_trunc("macaddr_trunc"_LEX_CSTRING);
static Create_macaddr_func<Item_func_macaddr8_trunc>
  create_macaddr8_trunc("macaddr8_trunc"_LEX_CSTRING);
static Create_macaddr_func<Item_func_macaddr8_set7bit>
  create_macaddr8_set7bit("macaddr8_set7bit"_LEX_CSTRING);

Plugin_function plugin_descriptor_macaddr_trunc(&create_macaddr_trunc);
Plugin_function plugin_descriptor_macaddr8_trunc(&create_macaddr8_trunc);
Plugin_function plugin_descriptor_macaddr8_set7bit(&create_macaddr8_set7bit);

