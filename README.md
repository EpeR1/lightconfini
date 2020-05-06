
# Lightweight INI style Configuration manager  

## Supports (currently)

- Reading & processing sections from file.
- Reading parameter/value pairs from an ini file.  
- Converting readed values to int/float/string.
- Building linked list from parameters.
- Multiline values are supported.
- Reading comments.  

Automatically switch between UNIX ('\n') and Windows ('\r\n') line endings. MAC ('\r' only) is not supported currently.  
Comments can be started with ';' or '#' signs.  
Multiline values are supported is they are surrounded with Double-Quotation-Mark (") signs.  
Uses Finite State Machine technique for file processing.  


## Future Plans  

- Writing and Rewriting support for ini files.
- Writing support for comments.
- Modifying one or more values/parameters/sections in ini files.

