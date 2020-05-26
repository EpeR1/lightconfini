/* INI fájl olvasás */

#include <stdio.h>
#include <string.h> /* strncpy + strerror */
#include <stdlib.h> /* malloc(), atoi(), exit(EXIT_FALIURE) */
#include <errno.h>  /* errno */
#include <stdarg.h>
#include <stdint.h> /* int64_t */

#define ini_write_c
#include "inirw_internal.h"
#include "lightconfini.h"

