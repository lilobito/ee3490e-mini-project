/* test_buoi1.c -- checks log_error and split_csv_line.
   Test program only: it is not part of the submission.  */

#include <stdio.h>
#include <string.h>
#include "common.h"

#define MAX_TEST_FIELDS 8
#define TEST_LOG "test.log"

static int failures;

/* Split a copy of TEXT and compare the result with EXPECTED_COUNT and
   the strings in EXPECTED.  LABEL describes the case on screen.  */
static void
check_split (const char *label, const char *text, int expected_count,
             const char *expected[])
{
  char line[LINE_SIZE];
  char *fields[MAX_TEST_FIELDS];
  int count;
  int i;
  int ok;

  snprintf (line, sizeof line, "%s", text);
  count = split_csv_line (line, fields, MAX_TEST_FIELDS);
  ok = (count == expected_count);
  for (i = 0; ok && i < count && i < MAX_TEST_FIELDS; i++)
    ok = (strcmp (fields[i], expected[i]) == 0);

  printf ("%s  %-34s -> %2d truong:", ok ? "DAT" : "SAI", label, count);
  for (i = 0; i < count && i < MAX_TEST_FIELDS; i++)
    printf (" [%s]", fields[i]);
  if (count > MAX_TEST_FIELDS)
    printf (" (chi luu %d truong dau)", MAX_TEST_FIELDS);
  printf ("\n");
  if (!ok)
    failures++;
}

/* Write two lines with log_error, read the log back and compare.  */
static void
check_log (void)
{
  const char *expected[] = { "Error 01: invalid command",
                             "Error 12: test message" };
  char line[LINE_SIZE];
  FILE *file;
  int i = 0;
  int ok = 1;

  remove (TEST_LOG);
  log_error (TEST_LOG, 1, "invalid command");
  log_error (TEST_LOG, 12, "test message");

  file = fopen (TEST_LOG, "r");
  if (file == NULL)
    ok = 0;
  else
    {
      while (fgets (line, sizeof line, file) != NULL)
        {
          line[strcspn (line, "\r\n")] = '\0';
          if (i >= 2 || strcmp (line, expected[i]) != 0)
            ok = 0;
          i++;
        }
      fclose (file);
    }
  if (i != 2)
    ok = 0;
  printf ("%s  log_error ghi dung 2 dong vao %s\n", ok ? "DAT" : "SAI",
          TEST_LOG);
  if (!ok)
    failures++;
}

int
main (void)
{
  const char *load_line[] = { "1", "TV", "40", "200", "250" };
  const char *empty_power[] = { "2", "2026:11:10 15:16:03", "", "3" };
  const char *blanks[] = { "", " ", " ", "" };
  const char *single[] = { "abc" };
  const char *nothing[] = { "" };
  const char *windows[] = { "5", "Lamp", "5", "15", "20" };
  const char *many[] = { "1", "2", "3", "4", "5", "6", "7", "8" };

  printf ("=== log_error ===\n");
  check_log ();

  printf ("\n=== split_csv_line ===\n");
  check_split ("dong file tai, co \\n o cuoi", "1,TV,40,200,250\n", 5,
               load_line);
  check_split ("cong suat bi trong", "2,2026:11:10 15:16:03,,3", 4,
               empty_power);
  check_split ("dong toan dau phay va dau cach", ", , ,", 4, blanks);
  check_split ("khong co dau phay", "abc", 1, single);
  check_split ("dong rong", "", 1, nothing);
  check_split ("ket thuc kieu Windows \\r\\n", "5,Lamp,5,15,20\r\n", 5,
               windows);
  check_split ("10 truong, mang chi chua 8", "1,2,3,4,5,6,7,8,9,10", 10,
               many);

  printf ("\nTong so truong hop SAI: %d\n", failures);
  return failures == 0 ? 0 : 1;
}
