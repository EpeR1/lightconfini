
# Lightweight INI style Configuration manager  

## Supports (currently)

- Reading & processing sections from file.
- Reading parameter/value pairs from an ini file.  
- Converting readed values to int/float/string.
- Building linked list from parameters.
- Multiline values are supported.
- Reading comments.
- UNICODE/UTF8 support.
- Escape sequence support.

**Automatically switch** between **UNIX ('\n')** and **Windows ('\r\n')** line endings. MAC ('\r' only) is not supported currently.  
**Comments** can be started with **';'** or **'#'** signs.  
**Multiline** values are supported if they are surrounded **with Double-Quotation-Mark (")** signs.  
Uses Finite State Machine technique for file processing.  
**Sections, Parameter names** can be **ASCII** alphabetical or numerical characters.  
**Values** in alone also ASCII, or between Double-Quotation-Marks(") can be **any UNICODE/UTF8** characters.  
Maximum line length can be 2^64/2 = 2^63 characters.  

## Future Plans  

- Writing and Rewriting support for ini files.
- Writing support for comments.
- Modifying one or more values/parameters/sections in ini files.
