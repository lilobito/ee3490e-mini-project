/* task2.c -- power_process: validate and analyse the data of the power
   meters (Task 2 of the EE3490E mini project).

   Usage: power_process [load_file] [data_file]

   Task 2.1 writes power_outlier.csv and power_valid.csv, Task 2.2
   writes load_condition.csv and Task 2.3 writes power_summary.csv.
   Every error goes to task2.log; nothing is printed on the screen.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common.h"

#define LOG_NAME "task2.log"
#define DEFAULT_LOAD_NAME "load_specs.csv"
#define DEFAULT_DATA_NAME "power_sensor.csv"
#define OUTLIER_NAME "power_outlier.csv"
#define VALID_NAME "power_valid.csv"
#define CONDITION_NAME "load_condition.csv"
#define SUMMARY_NAME "power_summary.csv"

#define DATA_HEADER "id,time,power,load"
#define CONDITION_HEADER "id,time,power,energy,load,condition"
#define SUMMARY_HEADER "load,time,meters,power"

#define DATA_FIELDS 4
#define MAX_ARGUMENTS 3
#define MAX_SAMPLES 10000
#define HOURS_PER_DAY 24
#define MESSAGE_SIZE 80

/* Error codes of Task 2 and their descriptions.  Codes 01 to 09 come
   from the assignment; code 10 is one the group added.  */
#define ERR_INPUT_NOT_FOUND 1
#define ERR_INVALID_DATA_FILE 2
#define ERR_INVALID_LOAD_FORMAT 3
#define ERR_INCOMPATIBLE_LOAD 4
#define ERR_INVALID_COMMAND 5
#define ERR_INVALID_LINE 6
#define ERR_DUPLICATED 7
#define ERR_CANNOT_OVERRIDE 8
#define ERR_TOO_LARGE 9
#define ERR_LOAD_NOT_FOUND 10

#define MSG_INPUT_NOT_FOUND "input file not found or not accessible"
#define MSG_INVALID_DATA_FILE "invalid input data file"
#define MSG_INVALID_LOAD "invalid input load file"
#define MSG_INVALID_COMMAND "invalid command"
#define MSG_INVALID_LINE "invalid data at line %d"
#define MSG_DUPLICATED "data at line %d and %d are duplicated"
#define MSG_CANNOT_OVERRIDE "cannot override output file"
#define MSG_TOO_LARGE "input file is too large"
#define MSG_LOAD_NOT_FOUND "load file not found or not accessible"

/* Results of read_samples.  */
#define READ_OK 0
#define READ_NOT_FOUND 1
#define READ_BAD_FORMAT 2

/* Operating codes of Table 1 in the assignment.  */
#define CONDITION_UNKNOWN 0
#define CONDITION_OFF 1
#define CONDITION_INEFFICIENT 2
#define CONDITION_GOOD 3
#define CONDITION_OVERLOADED 4
#define CONDITION_FAILURE 5

/* Name of each operating code, indexed by the code.  */
static const char *const condition_names[] = {
  "unknown", "off", "inefficient", "good", "overloaded", "failure"
};

/* The arrays are static so that they live outside the stack: each one
   holds up to MAX_SAMPLES entries.  */
static Sample samples[MAX_SAMPLES];     /* every readable data line */
static int sample_lines[MAX_SAMPLES];   /* its line number in the file */
static Sample valid[MAX_SAMPLES];       /* samples inside the range */
static HourRecord hours[MAX_SAMPLES];   /* one per sensor and hour */
static int counted_ids[MAX_SAMPLES];    /* used by summarize_by_load */

/* ---------------------------------------------------------------- */
/* Command line and input files                                     */
/* ---------------------------------------------------------------- */

/* Store the load file name and the data file name given on the
   command line in *LOAD_PATH and *DATA_PATH.  A missing name is
   replaced by its default.  Return 1 if the command is valid, 0 if it
   has more than two names.  */
static int
parse_arguments (int argc, char *argv[], const char **load_path,
                 const char **data_path)
{
  *load_path = DEFAULT_LOAD_NAME;
  *data_path = DEFAULT_DATA_NAME;
  if (argc > MAX_ARGUMENTS)
    return 0;
  if (argc >= 2)
    *load_path = argv[1];
  if (argc >= 3)
    *data_path = argv[2];
  return 1;
}

/* Return 1 if the COUNT loads in SPECS agree with each other and with
   the meter: codes are unique and 0 <= min <= max < failure <= the
   largest value the meter can measure.  Return 0 otherwise.  */
static int
check_load_specs (const LoadSpec specs[], int count)
{
  int i;
  int j;

  for (i = 0; i < count; i++)
    {
      if (specs[i].min_power < 0.0
          || specs[i].min_power > specs[i].max_power
          || specs[i].max_power >= specs[i].failure_power
          || specs[i].failure_power > METER_MAX)
        return 0;
      for (j = 0; j < i; j++)
        if (specs[j].code == specs[i].code)
          return 0;
    }
  return 1;
}

/* Turn LINE, one line of the data file, into *SAMPLE.  Return 1 if it
   has exactly four fields with a positive id, a valid time, a number
   as power and a positive load code; return 0 otherwise.  The power is
   not range-checked here: a readable value outside the meter range is
   an outlier of Task 2.1, not an invalid line.  LINE is modified.  */
static int
parse_sample_line (char *line, Sample *sample)
{
  char *fields[DATA_FIELDS];
  Sample result;

  if (split_csv_line (line, fields, DATA_FIELDS) != DATA_FIELDS)
    return 0;
  if (!parse_int (fields[0], &result.id) || result.id <= 0)
    return 0;
  if (!parse_time (fields[1], &result.time))
    return 0;
  if (!parse_double (fields[2], &result.power))
    return 0;
  if (!parse_int (fields[3], &result.load) || result.load <= 0)
    return 0;
  *sample = result;
  return 1;
}

/* Return the index of the first of the COUNT samples in ITEMS that has
   the same sensor id and time as CANDIDATE, or -1 if there is none.  */
static int
find_duplicate (const Sample items[], int count, const Sample *candidate)
{
  int i;

  for (i = 0; i < count; i++)
    if (items[i].id == candidate->id && items[i].time == candidate->time)
      return i;
  return -1;
}

/* Write "Error CODE: ..." to the log, where the description is the
   printf pattern FORMAT filled with the line numbers FIRST and
   SECOND (SECOND is ignored when FORMAT has only one %d).  */
static void
log_line_error (int code, const char *format, int first, int second)
{
  char message[MESSAGE_SIZE];

  snprintf (message, sizeof message, format, first, second);
  log_error (LOG_NAME, code, message);
}

/* Read the data file PATH.  Every line that can be read is stored in
   ITEMS, with its line number in LINES, and *COUNT receives how many.
   The header is line 0 and data lines are numbered from 1.  Invalid
   lines (Error 06) and lines repeating the id and time of an earlier
   line (Error 07) are logged and skipped.  Blank lines are skipped
   silently.  Only the first MAX_ITEMS data lines are processed; if the
   file has more, Error 09 is logged once.  Return READ_OK,
   READ_NOT_FOUND or READ_BAD_FORMAT (no header line).  */
static int
read_samples (const char *path, Sample items[], int lines[], int max_items,
              int *count)
{
  FILE *file;
  char line[LINE_SIZE];
  Sample sample;
  int line_number = 0;
  int first;

  *count = 0;
  file = fopen (path, "r");
  if (file == NULL)
    return READ_NOT_FOUND;

  if (fgets (line, sizeof line, file) == NULL)
    {
      fclose (file);
      return READ_BAD_FORMAT;
    }
  line[strcspn (line, "\r\n")] = '\0';
  if (strcmp (line, DATA_HEADER) != 0)
    {
      fclose (file);
      return READ_BAD_FORMAT;
    }

  while (fgets (line, sizeof line, file) != NULL)
    {
      line_number++;
      line[strcspn (line, "\r\n")] = '\0';
      if (line[0] == '\0')
        continue;
      if (line_number > max_items)
        {
          log_error (LOG_NAME, ERR_TOO_LARGE, MSG_TOO_LARGE);
          break;
        }
      if (!parse_sample_line (line, &sample))
        {
          log_line_error (ERR_INVALID_LINE, MSG_INVALID_LINE, line_number, 0);
          continue;
        }
      first = find_duplicate (items, *count, &sample);
      if (first >= 0)
        {
          log_line_error (ERR_DUPLICATED, MSG_DUPLICATED, lines[first],
                          line_number);
          continue;
        }
      items[*count] = sample;
      lines[*count] = line_number;
      (*count)++;
    }
  fclose (file);
  return READ_OK;
}

/* Return the load of SPECS (COUNT entries) whose code is CODE, or NULL
   if there is none.  */
static const LoadSpec *
find_load (const LoadSpec specs[], int count, int code)
{
  int i;

  for (i = 0; i < count; i++)
    if (specs[i].code == code)
      return &specs[i];
  return NULL;
}

/* Return 1 if every one of the COUNT samples in ITEMS has a load code
   that exists in SPECS, 0 otherwise.  */
static int
all_loads_known (const Sample items[], int count, const LoadSpec specs[],
                 int spec_count)
{
  int i;

  for (i = 0; i < count; i++)
    if (find_load (specs, spec_count, items[i].load) == NULL)
      return 0;
  return 1;
}

/* ---------------------------------------------------------------- */
/* Task 2.1: valid values and outliers                              */
/* ---------------------------------------------------------------- */

/* Return 1 if POWER is a value the meter can report: 0 (load switched
   off) or a value inside the measuring range.  */
static int
is_valid_power (double power)
{
  return power == 0.0 || (power >= METER_MIN && power <= METER_MAX);
}

/* Write the header and those of the COUNT samples in ITEMS whose
   validity equals WANT_VALID to OUTPUT, in the input format.  */
static void
write_samples (FILE *output, const Sample items[], int count, int want_valid)
{
  char text[TIME_TEXT_SIZE];
  int i;

  fprintf (output, "%s\n", DATA_HEADER);
  for (i = 0; i < count; i++)
    {
      if (is_valid_power (items[i].power) != want_valid)
        continue;
      format_time (items[i].time, text, sizeof text);
      fprintf (output, "%d,%s,%.1f,%d\n", items[i].id, text, items[i].power,
               items[i].load);
    }
}

/* Task 2.1.  Copy the samples of ITEMS (COUNT entries) whose power is
   valid to VALID_ITEMS and store how many in *VALID_COUNT.  Write the
   outliers to OUTLIER_NAME, preceded by their number, and the valid
   samples to VALID_NAME.  Return 1, or 0 if a file cannot be written.  */
static int
filter_outliers (const Sample items[], int count, Sample valid_items[],
                 int *valid_count)
{
  FILE *file;
  int i;

  *valid_count = 0;
  for (i = 0; i < count; i++)
    if (is_valid_power (items[i].power))
      valid_items[(*valid_count)++] = items[i];

  file = fopen (OUTLIER_NAME, "w");
  if (file == NULL)
    return 0;
  fprintf (file, "number of outliers: %d\n", count - *valid_count);
  write_samples (file, items, count, 0);
  fclose (file);

  file = fopen (VALID_NAME, "w");
  if (file == NULL)
    return 0;
  write_samples (file, items, count, 1);
  fclose (file);
  return 1;
}

/* ---------------------------------------------------------------- */
/* Task 2.2: hourly average, energy and load condition              */
/* ---------------------------------------------------------------- */

/* Return the operating code of Table 1 for the hourly average POWER of
   a load with the specification SPEC, or CONDITION_UNKNOWN if SPEC is
   NULL.  */
static int
classify_condition (double power, const LoadSpec *spec)
{
  if (spec == NULL)
    return CONDITION_UNKNOWN;
  if (power == 0.0)
    return CONDITION_OFF;
  if (power < spec->min_power)
    return CONDITION_INEFFICIENT;
  if (power <= spec->max_power)
    return CONDITION_GOOD;
  if (power < spec->failure_power)
    return CONDITION_OVERLOADED;
  return CONDITION_FAILURE;
}

/* Return the start of the hour that contains MOMENT, in local time:
   16:42:07 gives 16:00:00 of the same day.  */
static time_t
hour_start (time_t moment)
{
  struct tm parts = *localtime (&moment);

  parts.tm_min = 0;
  parts.tm_sec = 0;
  parts.tm_isdst = -1;
  return mktime (&parts);
}

/* qsort comparison: order samples by sensor id, then by time.  */
static int
compare_samples (const void *left, const void *right)
{
  const Sample *a = left;
  const Sample *b = right;

  if (a->id != b->id)
    return a->id < b->id ? -1 : 1;
  if (a->time != b->time)
    return a->time < b->time ? -1 : 1;
  return 0;
}

/* qsort comparison: order hourly records by time, then by sensor id,
   as in the example output of the assignment.  */
static int
compare_hours (const void *left, const void *right)
{
  const HourRecord *a = left;
  const HourRecord *b = right;

  if (a->time != b->time)
    return a->time < b->time ? -1 : 1;
  if (a->id != b->id)
    return a->id < b->id ? -1 : 1;
  return 0;
}

/* Task 2.2.  Sort the COUNT valid samples in ITEMS and build one
   record per sensor and hour in RECORDS: the average of the samples of
   that hour, the energy accumulated by the sensor since its first
   hour, its load code and its operating condition.  Return the number
   of records, sorted by time and then by sensor id.  */
static int
compute_hourly (Sample items[], int count, const LoadSpec specs[],
                int spec_count, HourRecord records[])
{
  int record_count = 0;
  int previous_id = 0;
  double energy = 0.0;
  int i = 0;
  int j;

  qsort (items, count, sizeof items[0], compare_samples);
  while (i < count)
    {
      int id = items[i].id;
      time_t hour = hour_start (items[i].time);
      double sum = 0.0;
      double average;
      HourRecord *record = &records[record_count];

      if (id != previous_id)
        {
          energy = 0.0;
          previous_id = id;
        }
      j = i;
      while (j < count && items[j].id == id
             && hour_start (items[j].time) == hour)
        {
          sum += items[j].power;
          j++;
        }
      average = sum / (j - i);
      energy += average;

      record->id = id;
      record->time = hour;
      record->power = average;
      record->energy = energy;
      record->load = items[i].load;
      record->condition
        = classify_condition (average, find_load (specs, spec_count,
                                                  items[i].load));
      record_count++;
      i = j;
    }
  qsort (records, record_count, sizeof records[0], compare_hours);
  return record_count;
}

/* Write the COUNT records to CONDITION_NAME.  Return 1, or 0 if the
   file cannot be written.  */
static int
write_load_condition (const HourRecord records[], int count)
{
  FILE *file;
  char text[TIME_TEXT_SIZE];
  int i;

  file = fopen (CONDITION_NAME, "w");
  if (file == NULL)
    return 0;
  fprintf (file, "%s\n", CONDITION_HEADER);
  for (i = 0; i < count; i++)
    {
      format_time (records[i].time, text, sizeof text);
      fprintf (file, "%d,%s,%.1f,%.1f,%d,%s\n", records[i].id, text,
               records[i].power, records[i].energy, records[i].load,
               condition_names[records[i].condition]);
    }
  fclose (file);
  return 1;
}

/* ---------------------------------------------------------------- */
/* Task 2.3: total power per load type and hour of the day          */
/* ---------------------------------------------------------------- */

/* Return the position of the load whose code is CODE in SPECS (COUNT
   entries), or -1 if there is none.  */
static int
load_index (const LoadSpec specs[], int count, int code)
{
  int i;

  for (i = 0; i < count; i++)
    if (specs[i].code == code)
      return i;
  return -1;
}

/* Return 1 if ID is one of the COUNT values in IDS.  */
static int
contains (const int ids[], int count, int id)
{
  int i;

  for (i = 0; i < count; i++)
    if (ids[i] == id)
      return 1;
  return 0;
}

/* Task 2.3.  For every load type that has at least one meter, write
   24 lines to SUMMARY_NAME: the load code, the hour of the day, the
   number of meters of that type and the sum of their hourly averages
   at that hour over the whole simulation.  Records whose load is not
   in SPECS are left out.  Return 1, or 0 if the file cannot be
   written.  */
static int
summarize_by_load (const HourRecord records[], int count,
                   const LoadSpec specs[], int spec_count)
{
  static double totals[MAX_LOADS][HOURS_PER_DAY];
  int meters[MAX_LOADS];
  int counted = 0;
  FILE *file;
  int i;
  int hour;

  memset (totals, 0, sizeof totals);
  memset (meters, 0, sizeof meters);
  for (i = 0; i < count; i++)
    {
      int index = load_index (specs, spec_count, records[i].load);

      if (index < 0)
        continue;
      hour = localtime (&records[i].time)->tm_hour;
      totals[index][hour] += records[i].power;
      if (!contains (counted_ids, counted, records[i].id))
        {
          counted_ids[counted++] = records[i].id;
          meters[index]++;
        }
    }

  file = fopen (SUMMARY_NAME, "w");
  if (file == NULL)
    return 0;
  fprintf (file, "%s\n", SUMMARY_HEADER);
  for (i = 0; i < spec_count; i++)
    {
      if (meters[i] == 0)
        continue;
      for (hour = 0; hour < HOURS_PER_DAY; hour++)
        fprintf (file, "%d,%d,%d,%.1f\n", specs[i].code, hour, meters[i],
                 totals[i][hour]);
    }
  fclose (file);
  return 1;
}

/* ---------------------------------------------------------------- */
/* Main program                                                     */
/* ---------------------------------------------------------------- */

/* Empty the log file so that it only shows the errors of this run.  */
static void
clear_log (void)
{
  FILE *file = fopen (LOG_NAME, "w");

  if (file != NULL)
    fclose (file);
}

int
main (int argc, char *argv[])
{
  LoadSpec specs[MAX_LOADS];
  const char *load_path;
  const char *data_path;
  int spec_count;
  int sample_count;
  int valid_count;
  int hour_count;
  int result;

  clear_log ();
  if (!parse_arguments (argc, argv, &load_path, &data_path))
    {
      log_error (LOG_NAME, ERR_INVALID_COMMAND, MSG_INVALID_COMMAND);
      return EXIT_FAILURE;
    }

  result = read_load_specs (load_path, specs, MAX_LOADS, &spec_count);
  if (result == LOAD_NOT_FOUND)
    {
      log_error (LOG_NAME, ERR_LOAD_NOT_FOUND, MSG_LOAD_NOT_FOUND);
      return EXIT_FAILURE;
    }
  if (result == LOAD_INVALID)
    {
      log_error (LOG_NAME, ERR_INVALID_LOAD_FORMAT, MSG_INVALID_LOAD);
      return EXIT_FAILURE;
    }
  if (!check_load_specs (specs, spec_count))
    {
      log_error (LOG_NAME, ERR_INCOMPATIBLE_LOAD, MSG_INVALID_LOAD);
      return EXIT_FAILURE;
    }

  result = read_samples (data_path, samples, sample_lines, MAX_SAMPLES,
                         &sample_count);
  if (result == READ_NOT_FOUND)
    {
      log_error (LOG_NAME, ERR_INPUT_NOT_FOUND, MSG_INPUT_NOT_FOUND);
      return EXIT_FAILURE;
    }
  if (result == READ_BAD_FORMAT)
    {
      log_error (LOG_NAME, ERR_INVALID_DATA_FILE, MSG_INVALID_DATA_FILE);
      return EXIT_FAILURE;
    }
  if (!all_loads_known (samples, sample_count, specs, spec_count))
    log_error (LOG_NAME, ERR_INCOMPATIBLE_LOAD, MSG_INVALID_LOAD);

  if (!filter_outliers (samples, sample_count, valid, &valid_count))
    {
      log_error (LOG_NAME, ERR_CANNOT_OVERRIDE, MSG_CANNOT_OVERRIDE);
      return EXIT_FAILURE;
    }

  hour_count = compute_hourly (valid, valid_count, specs, spec_count, hours);
  if (!write_load_condition (hours, hour_count)
      || !summarize_by_load (hours, hour_count, specs, spec_count))
    {
      log_error (LOG_NAME, ERR_CANNOT_OVERRIDE, MSG_CANNOT_OVERRIDE);
      return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
