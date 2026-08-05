# mariadb-plugin-type-macaddr

![mariabd-plugin-type-macaddr](logo/macaddr_type.png)


This plugin adds PostgreSQL-compatible `MACADDR` and `MACADDR8` data types to
MariaDB Server. They use fixed-size 6-byte and 8-byte binary storage,
respectively, with index ordering based directly on the address bytes.

Accepted input uses hexadecimal byte pairs, optionally separated consistently
by `:`, `-`, or `.` on byte boundaries. Whitespace around the value and upper-
or lower-case hexadecimal digits are accepted. Output is canonical lower-case,
colon-separated text.

`MACADDR8` accepts both EUI-64 and EUI-48 input. EUI-48 input is expanded by
inserting `ff:fe` after its first three bytes, as PostgreSQL does.

```sql
MariaDB > INSTALL SONAME 'type_macaddr';
MariaDB > SELECT plugin_name, plugin_type, plugin_library, plugin_description, plugin_author 
FROM information_schema.PLUGINS WHERE plugin_library LIKE 'type_macaddr.so';
+------------------+-------------+-----------------+------------------------------------------+---------------+
| plugin_name      | plugin_type | plugin_library  | plugin_description                       | plugin_author |
+------------------+-------------+-----------------+------------------------------------------+---------------+
| macaddr          | DATA TYPE   | type_macaddr.so | PostgreSQL-compatible 48-bit MAC address | lefred        |
| macaddr8         | DATA TYPE   | type_macaddr.so | PostgreSQL-compatible 64-bit MAC address | lefred        |
| macaddr_trunc    | FUNCTION    | type_macaddr.so | Clear the last three MAC address bytes   | lefred        |
| macaddr8_trunc   | FUNCTION    | type_macaddr.so | Clear the last five MAC address bytes    | lefred        |
| macaddr8_set7bit | FUNCTION    | type_macaddr.so | Set the modified EUI-64 seventh bit      | lefred        |
+------------------+-------------+-----------------+------------------------------------------+---------------+
5 rows in set (0.002 sec)
```

Let's take a look:

```sql
MariaDB > CREATE TABLE devices (
  ethernet MACADDR NOT NULL,
  eui64 MACADDR8,
  PRIMARY KEY (ethernet)
);
MariaDB > INSERT INTO devices VALUES
  ('08-00-2B-01-02-03', '08:00:2b:01:02:03');
MariaDB > SELECT * FROM devices;
+-------------------+-------------------------+
| ethernet          | eui64                   |
+-------------------+-------------------------+
| 08:00:2b:01:02:03 | 08:00:2b:ff:fe:01:02:03 |
+-------------------+-------------------------+
1 row in set (0.001 sec)
```

Functions:

- `MACADDR_TRUNC(value)` clears the last three bytes.
- `MACADDR8_TRUNC(value)` clears the last five bytes.
- `MACADDR8_SET7BIT(value)` sets the seventh bit for modified EUI-64.

MariaDB already reserves the overloaded numeric name `TRUNC`, so the two typed
truncation functions use explicit names instead.

