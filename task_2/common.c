/* common.c -- functions shared by power_sim, power_process and
   power_comm.  */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

/* Header line that every load specification file must start with.  */
#define LOAD_HEADER "code,name,min,max,failure"
#define LOAD_FIELDS 5

/* Append one line "Error AB: DESCRIPTION" to the log file LOG_NAME.
   CODE is always printed with two digits.  Nothing is printed on the
   screen, as the assignment requires.  If the log file cannot be
   opened there is nowhere left to report the problem, so the function
   simply returns.  */
void
log_error (const char *log_name, int code, const char *description)
{
  FILE *log_file;

  log_file = fopen (log_name, "a");
  if (log_file == NULL)
    return;
  fprintf (log_file, "Error %02d: %s\n", code, description);
  fclose (log_file);
}

/* Split LINE at every comma.  A pointer to the start of each field is
   stored in FIELDS, at most MAX_FIELDS of them.  LINE is modified: a
   trailing newline is removed and every comma is replaced by '\0', so
   each field becomes a string of its own.  Empty fields are kept,
   which strtok would lose.  Return the number of fields in LINE; it
   can be larger than MAX_FIELDS, so the caller can detect a line with
   too many fields.  */
int
split_csv_line (char *line, char *fields[], int max_fields)
{
  char *start = line;
  char *comma;
  int count = 0;

  line[strcspn (line, "\r\n")] = '\0';
  for (;;)
    {
      if (count < max_fields)
        fields[count] = start;
      count++;
      comma = strchr (start, ',');
      if (comma == NULL)
        break;
      *comma = '\0';
      start = comma + 1;
    }
  return count;
}

/* Convert TEXT, a whole decimal integer such as "12" or "-2", and
   store it in *VALUE.  Return 1 on success.  Return 0, leaving *VALUE
   unchanged, if TEXT is empty, has a space or any other character
   that is not part of the number, or does not fit in an int.  The
   caller decides whether a negative value is acceptable.  */
int
parse_int (const char *text, int *value)
{
  char *end;
  long number;

  if (isspace ((unsigned char) text[0]))
    return 0;
  errno = 0;
  number = strtol (text, &end, 10);
  if (end == text || *end != '\0' || errno == ERANGE)
    return 0;
#if LONG_MAX > INT_MAX
  /* Only needed where long is wider than int; on Windows both have 32
     bits and strtol has already reported the overflow as ERANGE.  */
  if (number < INT_MIN || number > INT_MAX)
    return 0;
#endif
  *value = (int) number;
  return 1;
}

/* Convert TEXT, a whole decimal number such as "25.4" or "-10.8", and
   store it in *VALUE.  Return 1 on success.  Return 0, leaving *VALUE
   unchanged, if TEXT is empty, has a space or any other character
   that is not part of the number, or is too large, infinite or not a
   number.  The caller decides which range is acceptable.  */
int
parse_double (const char *text, double *value)
{
  char *end;
  double number;

  if (isspace ((unsigned char) text[0]))
    return 0;
  errno = 0;
  number = strtod (text, &end);
  if (end == text || *end != '\0' || errno == ERANGE)
    return 0;
  if (!isfinite (number))
    return 0;
  *value = number;
  return 1;
}

/* Convert TEXT, a local time written exactly as "YYYY:MM:DD hh:mm:ss",
   into a time_t and store it in *VALUE.  Return 1 on success.  Return
   0, leaving *VALUE unchanged, if TEXT does not follow the pattern
   character by character, or names a date or time that does not
   exist, such as month 13, 31 November or hour 24.  */
int
parse_time (const char *text, time_t *value)
{
  /* 'd' stands for any digit; other characters must match exactly.  */
  static const char pattern[] = "dddd:dd:dd dd:dd:dd";
  const size_t length = sizeof pattern - 1;
  struct tm parts;
  int year, month, day, hour, minute, second;
  time_t result;
  size_t i;

  if (strlen (text) != length)
    return 0;
  for (i = 0; i < length; i++)
    {
      if (pattern[i] == 'd')
        {
          if (!isdigit ((unsigned char) text[i]))
            return 0;
        }
      else if (text[i] != pattern[i])
        return 0;
    }

  sscanf (text, "%4d:%2d:%2d %2d:%2d:%2d",
          &year, &month, &day, &hour, &minute, &second);
  if (month < 1 || month > 12 || day < 1 || day > 31
      || hour > 23 || minute > 59 || second > 59)
    return 0;

  memset (&parts, 0, sizeof parts);
  parts.tm_year = year - 1900;
  parts.tm_mon = month - 1;
  parts.tm_mday = day;
  parts.tm_hour = hour;
  parts.tm_min = minute;
  parts.tm_sec = second;
  parts.tm_isdst = -1;
  result = mktime (&parts);

  /* mktime silently turns 31 November into 1 December, so a date that
     does not exist shows up as a changed day or month.  */
  if (result == (time_t) -1 || parts.tm_mday != day
      || parts.tm_mon != month - 1)
    return 0;
  *value = result;
  return 1;
}

/* Write VALUE as local time in the form "YYYY:MM:DD hh:mm:ss" into
   BUFFER, which holds SIZE characters; TIME_TEXT_SIZE is enough.  If
   the time cannot be converted, BUFFER becomes an empty string.  */
void
format_time (time_t value, char *buffer, size_t size)
{
  struct tm *parts;

  if (size == 0)
    return;
  parts = localtime (&value);
  if (parts == NULL || strftime (buffer, size, TIME_FORMAT, parts) == 0)
    buffer[0] = '\0';
}

/* Read the load specification file PATH into SPECS, which holds at
   most MAX_SPECS entries, and store the number of loads in *COUNT.
   Return LOAD_OK on success, LOAD_NOT_FOUND if the file cannot be
   opened, or LOAD_INVALID if it is empty, lacks the header line, has
   a line that is not five valid fields, or has more than MAX_SPECS
   loads.  Blank lines are skipped.  *COUNT stays 0 unless the result
   is LOAD_OK.  Nothing is written to a log: each program turns the
   result into its own error code.  */
int
read_load_specs (const char *path, LoadSpec specs[], int max_specs,
                 int *count)
{
  FILE *file;
  char line[LINE_SIZE];
  char *fields[LOAD_FIELDS];
  LoadSpec spec;
  int result = LOAD_OK;

  *count = 0;
  file = fopen (path, "r");
  if (file == NULL)
    return LOAD_NOT_FOUND;

  if (fgets (line, sizeof line, file) == NULL)
    result = LOAD_INVALID;
  else
    {
      line[strcspn (line, "\r\n")] = '\0';
      if (strcmp (line, LOAD_HEADER) != 0)
        result = LOAD_INVALID;
    }

  while (result == LOAD_OK && fgets (line, sizeof line, file) != NULL)
    {
      line[strcspn (line, "\r\n")] = '\0';
      if (line[0] == '\0')
        continue;
      if (split_csv_line (line, fields, LOAD_FIELDS) != LOAD_FIELDS
          || !parse_int (fields[0], &spec.code) || spec.code <= 0
          || fields[1][0] == '\0' || strlen (fields[1]) >= NAME_SIZE
          || !parse_double (fields[2], &spec.min_power)
          || !parse_double (fields[3], &spec.max_power)
          || !parse_double (fields[4], &spec.failure_power)
          || *count >= max_specs)
        {
          result = LOAD_INVALID;
          break;
        }
      strcpy (spec.name, fields[1]);
      specs[*count] = spec;
      (*count)++;
    }

  if (result == LOAD_OK && *count == 0)
    result = LOAD_INVALID;
  if (result != LOAD_OK)
    *count = 0;
  fclose (file);
  return result;
}
