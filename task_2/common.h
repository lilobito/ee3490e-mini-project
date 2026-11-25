/* common.h -- declarations shared by power_sim, power_process and
   power_comm.  */

#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <time.h>

/* Text form of a time stamp, as in "2026:11:10 16:00:00".  */
#define TIME_FORMAT "%Y:%m:%d %H:%M:%S"
#define TIME_TEXT_SIZE 20

#define NAME_SIZE 32
#define LINE_SIZE 256
#define MAX_LOADS 100

/* Measuring range of the power meter, in watts.  */
#define METER_MIN 1.0
#define METER_MAX 3000.0

/* Results of read_load_specs.  */
#define LOAD_OK 0
#define LOAD_NOT_FOUND 1
#define LOAD_INVALID 2

/* One line of the load specification file.  */
typedef struct
{
  int code;
  char name[NAME_SIZE];
  double min_power;
  double max_power;
  double failure_power;
} LoadSpec;

/* One line of the power data file.  */
typedef struct
{
  int id;
  time_t time;
  double power;
  int load;
} Sample;

/* One line of load_condition.csv.  */
typedef struct
{
  int id;
  time_t time;                  /* start of the hour */
  double power;                 /* average power over that hour */
  double energy;                /* energy accumulated so far */
  int load;
  int condition;                /* operating code 0..5 of Table 1 */
} HourRecord;

void log_error (const char *log_name, int code, const char *description);
int split_csv_line (char *line, char *fields[], int max_fields);
int parse_int (const char *text, int *value);
int parse_double (const char *text, double *value);
int parse_time (const char *text, time_t *value);
void format_time (time_t value, char *buffer, size_t size);
int read_load_specs (const char *path, LoadSpec specs[], int max_specs,
                     int *count);

#endif /* COMMON_H */
