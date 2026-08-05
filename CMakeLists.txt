# Copyright (c) 2019,2024,2025,2026 MariaDB Corporation
# Copyright (c) 2026 lefred (Frédéric Descamps)

MYSQL_ADD_PLUGIN(type_macaddr
                 plugin.cc sql_type_macaddr.cc macaddr_functions.cc
                 MODULE_ONLY RECOMPILE_FOR_EMBEDDED)

