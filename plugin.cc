/* Copyright (c) 2019,2024,2025,2026 MariaDB Corporation
   Copyright (c) 2026 lefred (Frédéric Descamps)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License. */

#define MYSQL_SERVER
#include "mariadb.h"
#include "sql_class.h"
#include "sql_type_macaddr.h"
#include "macaddr_functions.h"
#include <mysql/plugin_data_type.h>

static struct st_mariadb_data_type descriptor_macaddr=
{ MariaDB_DATA_TYPE_INTERFACE_VERSION, Type_handler_macaddr::singleton() };
static struct st_mariadb_data_type descriptor_macaddr8=
{ MariaDB_DATA_TYPE_INTERFACE_VERSION, Type_handler_macaddr8::singleton() };

#define MAC_PLUGIN_ENTRY(TYPE, DESCRIPTOR, NAME, DESCRIPTION) \
{ TYPE, DESCRIPTOR, NAME, "lefred", DESCRIPTION, PLUGIN_LICENSE_GPL, \
  0, 0, 0x0100, NULL, NULL, "0.1.0", MariaDB_PLUGIN_MATURITY_BETA }

maria_declare_plugin(type_macaddr)
  MAC_PLUGIN_ENTRY(MariaDB_DATA_TYPE_PLUGIN, &descriptor_macaddr,
                   "macaddr", "PostgreSQL-compatible 48-bit MAC address"),
  MAC_PLUGIN_ENTRY(MariaDB_DATA_TYPE_PLUGIN, &descriptor_macaddr8,
                   "macaddr8", "PostgreSQL-compatible 64-bit MAC address"),
  MAC_PLUGIN_ENTRY(MariaDB_FUNCTION_PLUGIN, &plugin_descriptor_macaddr_trunc,
                   "macaddr_trunc", "Clear the last three MAC address bytes"),
  MAC_PLUGIN_ENTRY(MariaDB_FUNCTION_PLUGIN, &plugin_descriptor_macaddr8_trunc,
                   "macaddr8_trunc", "Clear the last five MAC address bytes"),
  MAC_PLUGIN_ENTRY(MariaDB_FUNCTION_PLUGIN, &plugin_descriptor_macaddr8_set7bit,
                   "macaddr8_set7bit", "Set the modified EUI-64 seventh bit")
maria_declare_plugin_end;

