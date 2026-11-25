/* test_buoi2.c -- checks parse_int, parse_double, parse_time and
   format_time.  Test program only: it is not part of the submission.  */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "common.h"

/* Unix time of 2026:11:10 16:00:00 in Vietnam (UTC+7), taken from the
   packet example of Task 3.  */
#define EXAMPLE_TIME_TEXT "2026:11:10 16:00:00"
#define EXAMPLE_UNIX_TIME 1794301200L

static int failures;

static void
report (int ok, const char *what, const char *detail)
{
  printf ("%s  %-38s %s\n", ok ? "DAT" : "SAI", what, detail);
  if (!ok)
    failures++;
}

/* Check parse_int on TEXT: EXPECT_OK is the expected return value and
   EXPECT_VALUE the expected number when EXPECT_OK is 1.  */
static void
check_int (const char *label, const char *text, int expect_ok,
           int expect_value)
{
  char detail[64];
  int value = 0;
  int got = parse_int (text, &value);
  int ok = (got == expect_ok) && (!got || value == expect_value);

  if (got)
    snprintf (detail, sizeof detail, "-> 1, gia tri %d", value);
  else
    snprintf (detail, sizeof detail, "-> 0 (khong hop le)");
  report (ok, label, detail);
}

static void
check_double (const char *label, const char *text, int expect_ok,
              double expect_value)
{
  char detail[64];
  double value = 0.0;
  int got = parse_double (text, &value);
  int ok = (got == expect_ok) && (!got || value == expect_value);

  if (got)
    snprintf (detail, sizeof detail, "-> 1, gia tri %.1f", value);
  else
    snprintf (detail, sizeof detail, "-> 0 (khong hop le)");
  report (ok, label, detail);
}

/* Check parse_time on TEXT.  When it should succeed, format_time must
   give back exactly the same text.  */
static void
check_time (const char *label, const char *text, int expect_ok)
{
  char detail[64];
  char back[TIME_TEXT_SIZE];
  time_t value = 0;
  int got = parse_time (text, &value);
  int ok = (got == expect_ok);

  if (got)
    {
      format_time (value, back, sizeof back);
      ok = ok && strcmp (back, text) == 0;
      snprintf (detail, sizeof detail, "-> 1, in lai: %s", back);
    }
  else
    snprintf (detail, sizeof detail, "-> 0 (khong hop le)");
  report (ok, label, detail);
}

int
main (void)
{
  time_t first, second;
  char detail[64];

  printf ("=== parse_int ===\n");
  check_int ("so duong \"12\"", "12", 1, 12);
  check_int ("so am \"-2\"", "-2", 1, -2);
  check_int ("so 0 \"0\"", "0", 1, 0);
  check_int ("chuoi rong \"\"", "", 0, 0);
  check_int ("mot dau cach \" \"", " ", 0, 0);
  check_int ("dau cach o dau \" 5\"", " 5", 0, 0);
  check_int ("dau cach o cuoi \"5 \"", "5 ", 0, 0);
  check_int ("co chu \"12a\"", "12a", 0, 0);
  check_int ("so thuc \"1.5\"", "1.5", 0, 0);
  check_int ("chu \"abc\"", "abc", 0, 0);
  check_int ("qua lon \"99999999999\"", "99999999999", 0, 0);

  printf ("\n=== parse_double ===\n");
  check_double ("so thuc \"25.4\"", "25.4", 1, 25.4);
  check_double ("so am \"-10.8\"", "-10.8", 1, -10.8);
  check_double ("so nguyen \"3\"", "3", 1, 3.0);
  check_double ("so 0 \"0.0\"", "0.0", 1, 0.0);
  check_double ("chuoi rong \"\"", "", 0, 0.0);
  check_double ("mot dau cach \" \"", " ", 0, 0.0);
  check_double ("hai dau cham \"1.2.3\"", "1.2.3", 0, 0.0);
  check_double ("co chu \"25.4W\"", "25.4W", 0, 0.0);
  check_double ("vo cuc \"inf\"", "inf", 0, 0.0);
  check_double ("khong phai so \"nan\"", "nan", 0, 0.0);

  printf ("\n=== parse_time va format_time ===\n");
  check_time ("hop le", "2026:11:10 16:00:00", 1);
  check_time ("cuoi ngay", "2026:12:31 23:59:59", 1);
  check_time ("29/2 nam nhuan 2028", "2028:02:29 12:00:00", 1);
  check_time ("29/2 nam thuong 2027", "2027:02:29 12:00:00", 0);
  check_time ("thang 13", "2026:13:10 16:00:00", 0);
  check_time ("thang 0", "2026:00:10 16:00:00", 0);
  check_time ("ngay 31/11", "2026:11:31 00:00:00", 0);
  check_time ("gio 24", "2026:11:10 24:00:00", 0);
  check_time ("phut 60", "2026:11:10 16:60:00", 0);
  check_time ("thieu giay (trong de)", "2026:11:10 15:16:", 0);
  check_time ("sai dau phan cach", "2026-11-10 16:00:00", 0);
  check_time ("thieu so 0 o dau", "2026:11:10 9:00:00", 0);
  check_time ("thua ky tu o cuoi", "2026:11:10 16:00:00x", 0);
  check_time ("chuoi rong", "", 0);

  printf ("\n=== Tinh toan tren time_t ===\n");
  if (parse_time ("2026:11:10 16:00:00", &first)
      && parse_time ("2026:11:10 17:00:00", &second))
    {
      snprintf (detail, sizeof detail, "-> chenh %ld giay",
                (long) (second - first));
      report (second - first == 3600, "16:00 den 17:00 la 3600 giay", detail);
    }
  else
    report (0, "16:00 den 17:00 la 3600 giay", "-> parse_time loi");

  if (parse_time (EXAMPLE_TIME_TEXT, &first))
    {
      snprintf (detail, sizeof detail, "-> %ld", (long) first);
      if ((long) first == EXAMPLE_UNIX_TIME)
        report (1, "Unix time khop vi du Task 3", detail);
      else
        printf ("CANH BAO  Unix time khac vi du Task 3 %s: mui gio "
                "cua may khong phai UTC+7\n", detail);
    }

  printf ("\nTong so truong hop SAI: %d\n", failures);
  return failures == 0 ? 0 : 1;
}
