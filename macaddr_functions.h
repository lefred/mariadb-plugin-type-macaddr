#ifndef MACADDR_FUNCTIONS_INCLUDED
#define MACADDR_FUNCTIONS_INCLUDED
/* Copyright (c) 2019,2024,2025,2026 MariaDB Corporation
   Copyright (c) 2026 lefred (Frédéric Descamps)
   GPL version 2. */
#include <mysql/plugin_function.h>
extern Plugin_function plugin_descriptor_macaddr_trunc;
extern Plugin_function plugin_descriptor_macaddr8_trunc;
extern Plugin_function plugin_descriptor_macaddr8_set7bit;
#endif
