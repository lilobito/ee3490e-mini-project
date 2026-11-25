/* test_buoi3.c -- checks read_load_specs.  The program writes each
   test file itself, reads it back and removes it.
   Test program only: it is not part of the submission.  */

#include <stdio.h>
#include <string.h>
#include "common.h"

#define TEMP_FILE "tam_load_specs.csv"
#define SPEC_ROOM 3

static int failures;

static void
write_file (const char *content)
{
  FILE *file = fopen (TEMP_FILE, "w");

  if (file != NULL)
    {
      fputs (content, file);
      fclose (file);
    }
}

static const char *
result_name (int result)
{
  switch (result)
    {
    case LOAD_OK:
      return "LOAD_OK";
    case LOAD_NOT_FOUND:
      return "LOAD_NOT_FOUND";
    case LOAD_INVALID:
      return "LOAD_INVALID";
    default:
      return "?";
    }
}

/* Write CONTENT to a file, call read_load_specs and compare the result
   with EXPECT_RESULT and, on success, the number of loads.  */
static void
check (const char *label, const char *content, int expect_result,
       int expect_count)
{
  LoadSpec specs[SPEC_ROOM];
  int count = -1;
  int result;
  int ok;

  write_file (content);
  result = read_load_specs (TEMP_FILE, specs, SPEC_ROOM, &count);
  ok = (result == expect_result)
       && (result != LOAD_OK || count == expect_count);
  printf ("%s  %-40s -> %s, %d loai tai\n", ok ? "DAT" : "SAI", label,
          result_name (result), count);
  if (!ok)
    failures++;
  remove (TEMP_FILE);
}

int
main (void)
{
  LoadSpec specs[MAX_LOADS];
  int count = 0;
  int result;
  int ok;

  printf ("=== read_load_specs voi file mau ===\n");
  result = read_load_specs ("load_specs.csv", specs, MAX_LOADS, &count);
  ok = (result == LOAD_OK && count == 6 && specs[2].code == 3
        && strcmp (specs[2].name, "Desktop") == 0
        && specs[2].min_power == 60.0 && specs[2].max_power == 700.0
        && specs[2].failure_power == 1000.0);
  printf ("%s  %-40s -> %s, %d loai tai\n", ok ? "DAT" : "SAI",
          "load_specs.csv: 6 loai, thu 3 la Desktop", result_name (result),
          count);
  if (ok)
    printf ("     specs[2] = { %d, \"%s\", %.1f, %.1f, %.1f }\n",
            specs[2].code, specs[2].name, specs[2].min_power,
            specs[2].max_power, specs[2].failure_power);
  else
    failures++;

  printf ("\n=== read_load_specs voi file tu tao ===\n");
  check ("hai loai tai", "code,name,min,max,failure\n1,TV,40,200,250\n"
         "5,Lamp,5,15,20\n", LOAD_OK, 2);
  check ("ket thuc kieu Windows \\r\\n",
         "code,name,min,max,failure\r\n1,TV,40,200,250\r\n", LOAD_OK, 1);
  check ("dong trong o cuoi duoc bo qua",
         "code,name,min,max,failure\n1,TV,40,200,250\n\n", LOAD_OK, 1);
  check ("file 0 byte", "", LOAD_INVALID, 0);
  check ("chi co dong tieu de", "code,name,min,max,failure\n",
         LOAD_INVALID, 0);
  check ("thieu dong tieu de", "1,TV,40,200,250\n", LOAD_INVALID, 0);
  check ("tieu de dung dau cham phay",
         "code;name;min;max;failure\n1;TV;40;200;250\n", LOAD_INVALID, 0);
  check ("thieu mot cot", "code,name,min,max,failure\n1,TV,40,200\n",
         LOAD_INVALID, 0);
  check ("thua mot cot", "code,name,min,max,failure\n1,TV,40,200,250,9\n",
         LOAD_INVALID, 0);
  check ("ma tai khong phai so",
         "code,name,min,max,failure\nx,TV,40,200,250\n", LOAD_INVALID, 0);
  check ("ma tai bang 0", "code,name,min,max,failure\n0,TV,40,200,250\n",
         LOAD_INVALID, 0);
  check ("ten trong", "code,name,min,max,failure\n1,,40,200,250\n",
         LOAD_INVALID, 0);
  check ("cong suat khong phai so",
         "code,name,min,max,failure\n1,TV,abc,200,250\n", LOAD_INVALID, 0);
  check ("4 loai tai, mang chi chua 3",
         "code,name,min,max,failure\n1,A,1,2,3\n2,B,1,2,3\n3,C,1,2,3\n"
         "4,D,1,2,3\n", LOAD_INVALID, 0);

  printf ("\n=== file khong ton tai ===\n");
  result = read_load_specs ("khong_co_file_nay.csv", specs, MAX_LOADS,
                            &count);
  ok = (result == LOAD_NOT_FOUND);
  printf ("%s  %-40s -> %s\n", ok ? "DAT" : "SAI", "ten file khong ton tai",
          result_name (result));
  if (!ok)
    failures++;

  printf ("\nTong so truong hop SAI: %d\n", failures);
  return failures == 0 ? 0 : 1;
}
