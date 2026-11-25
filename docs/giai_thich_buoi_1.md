# Buổi 1 – Giải thích chi tiết

Tài liệu này giải thích mọi thứ đã có trong thư mục sau buổi 1: file `common.h`, hai hàm đầu tiên trong `common.c`, và chương trình thử. Đọc theo thứ tự từ trên xuống; mỗi phần dựa vào phần trước.

> **Cách đọc trong VS Code:** mở file này rồi bấm `Ctrl + Shift + V` để xem dạng đã định dạng. Bấm vào dòng **"Xem đáp án"** để mở đáp án của các câu hỏi.

---

## Mục lục

1. [Bức tranh chung: các file đã có và vai trò của chúng](#1-bức-tranh-chung)
2. [Một chương trình C được biên dịch như thế nào](#2-một-chương-trình-c-được-biên-dịch-như-thế-nào)
3. [Kiến thức nền: biến, địa chỉ, con trỏ](#3-kiến-thức-nền-biến-địa-chỉ-con-trỏ)
4. [Kiến thức nền: chuỗi trong C](#4-kiến-thức-nền-chuỗi-trong-c)
5. [Kiến thức nền: struct và typedef](#5-kiến-thức-nền-struct-và-typedef)
6. [Đọc file common.h từng dòng](#6-đọc-file-commonh-từng-dòng)
7. [Kiến thức nền: làm việc với file](#7-kiến-thức-nền-làm-việc-với-file)
8. [Hàm log_error](#8-hàm-log_error)
9. [Hàm split_csv_line](#9-hàm-split_csv_line)
10. [Chương trình thử và cách chạy](#10-chương-trình-thử-và-cách-chạy)
11. [Câu hỏi tự kiểm tra](#11-câu-hỏi-tự-kiểm-tra)
12. [Thuật ngữ cần nói được khi vấn đáp](#12-thuật-ngữ-cần-nói-được-khi-vấn-đáp)

---

## 1. Bức tranh chung

```
Documents\EE3490E\
├── MSSV1_MSSV2_mini_project_20261\     ← thư mục sẽ nộp
│   ├── task_1\                         (của A)
│   ├── task_2\                         (của bạn)
│   │   ├── common.h    khai báo dùng chung: hằng số, 3 struct, 7 hàm
│   │   └── common.c    thân các hàm dùng chung (hôm nay có 2 hàm)
│   └── task_3\                         (của A)
├── tests\                              ← chỉ để thử, KHÔNG nộp
│   ├── load_specs.csv      file thông số tải mẫu
│   ├── power_sensor.csv    file dữ liệu mẫu nhỏ
│   └── test_buoi1.c        chương trình kiểm tra 2 hàm của hôm nay
├── chay_thu_buoi1.bat      bấm đúp để biên dịch và chạy chương trình kiểm tra
├── kiem_tra.bat            file kiểm tra môi trường (đã chạy)
└── giai_thich_buoi_1.md    file bạn đang đọc
```

Vì sao có `common.h` và `common.c`? Cả ba chương trình của nhóm (`power_sim`, `power_process`, `power_comm`) đều cần những việc giống nhau: ghi lỗi vào file log, tách một dòng CSV, đổi chuỗi thành số, đổi chuỗi thành thời gian. Viết những việc đó **một lần** trong `common.c`, rồi cả ba chương trình cùng dùng. Như vậy:

- không phải viết lại ba lần;
- sửa một lỗi ở một chỗ là cả ba chương trình cùng được sửa;
- cả ba chương trình xử lý dữ liệu theo cùng một cách, nên file của chương trình này khớp với chương trình kia.

Đề yêu cầu mỗi thư mục `task_x` tự chứa đủ mã nguồn, nên cuối cùng `common.h` và `common.c` được **chép** vào cả ba thư mục.

---

## 2. Một chương trình C được biên dịch như thế nào

Khi bạn gõ:

```
gcc -Wall -Wextra -o power_process task2.c common.c
```

gcc làm ba bước:

| Bước | Tên | Làm gì |
|---|---|---|
| 1 | Tiền xử lý (preprocess) | Thực hiện các dòng bắt đầu bằng `#`. `#include "common.h"` được thay bằng **toàn bộ nội dung** file `common.h`. `#define METER_MAX 3000.0` làm mọi chỗ ghi `METER_MAX` bị thay bằng `3000.0`. |
| 2 | Biên dịch (compile) | Dịch **từng file `.c`** riêng rẽ thành mã máy (file đối tượng `.o`). Lúc này mỗi file chỉ cần biết hàm mình gọi *có tồn tại* và *nhận tham số gì*, chưa cần thân hàm. |
| 3 | Liên kết (link) | Ghép các file `.o` lại, nối mỗi lời gọi hàm với thân hàm thật, tạo ra `power_process.exe`. |

Từ đó suy ra vai trò của hai loại file:

- **File `.h` (header)**: chứa *khai báo*, tức lời giới thiệu: "có một hàm tên `log_error`, nhận ba tham số kiểu này, trả về kiểu kia". Bước 2 cần những lời giới thiệu này để kiểm tra bạn gọi hàm có đúng không.
- **File `.c`**: chứa *định nghĩa*, tức thân hàm thật. Bước 3 cần những thân hàm này.

Ý nghĩa các phần của lệnh gcc:

| Phần | Nghĩa |
|---|---|
| `-Wall -Wextra` | Bật gần như mọi cảnh báo. Cảnh báo của gcc thường chỉ thẳng vào một lỗi thật, nên coi mỗi cảnh báo là một lỗi phải sửa. |
| `-o power_process` | Tên file chạy được tạo ra (`power_process.exe` trên Windows). |
| `task2.c common.c` | Các file `.c` cần biên dịch và ghép lại. Quên `common.c` thì bước 3 báo `undefined reference to 'log_error'`. |
| `-I đường_dẫn` | (dùng trong file `.bat`) Bảo gcc tìm file `.h` ở thư mục đó. |

---

## 3. Kiến thức nền: biến, địa chỉ, con trỏ

Đây là phần quan trọng nhất của C. Hai hàm hôm nay và gần như mọi hàm sau này đều dùng nó.

### 3.1. Biến nằm ở đâu

Bộ nhớ máy tính là một dãy rất dài các ô, mỗi ô 1 byte, mỗi ô có một số thứ tự gọi là **địa chỉ**. Khi bạn khai báo `int count = 5;`, máy dành ra 4 ô liền nhau (một `int` dài 4 byte), ghi số 5 vào đó, và nhớ rằng tên `count` ứng với ô bắt đầu ở một địa chỉ nào đó, giả sử `1000`.

```
địa chỉ:   1000  1001  1002  1003
          ┌─────┬─────┬─────┬─────┐
count:    │        giá trị 5      │
          └─────┴─────┴─────┴─────┘
```

### 3.2. Con trỏ là gì

**Con trỏ là một biến chứa địa chỉ của một biến khác.**

```c
int count = 5;
int *p = &count;   /* p chứa địa chỉ của count, tức 1000 */
*p = 7;            /* ghi 7 vào ô mà p trỏ tới: count bây giờ bằng 7 */
```

| Ký hiệu | Đọc là | Nghĩa |
|---|---|---|
| `int *p` (khi khai báo) | "p là con trỏ tới int" | `p` sẽ chứa địa chỉ của một `int` |
| `&count` | "địa chỉ của count" | Lấy địa chỉ của biến |
| `*p` (khi dùng) | "giá trị tại p" | Đi tới ô có địa chỉ trong `p`, đọc hoặc ghi ở đó |
| `p->name` | "trường name của struct mà p trỏ tới" | Viết tắt của `(*p).name` |

### 3.3. Vì sao cần con trỏ: hàm chỉ nhận bản sao

Khi gọi hàm, C **chép giá trị** của đối số vào tham số. Hàm sửa tham số là sửa bản sao, biến bên ngoài không đổi:

```c
void set_ten (int x) { x = 10; }

int a = 1;
set_ten (a);       /* a vẫn bằng 1 */
```

Muốn hàm sửa được biến bên ngoài, đưa cho hàm **địa chỉ** của biến:

```c
void set_ten (int *x) { *x = 10; }

int a = 1;
set_ten (&a);      /* a bây giờ bằng 10 */
```

Đây chính là cách các hàm của nhóm trả kết quả. Ví dụ `parse_int (text, &value)` (viết ở buổi 2): hàm dùng `return` để báo **đúng hay sai** (1 hoặc 0), và ghi **số đọc được** vào `value` qua con trỏ.

### 3.4. Mảng và con trỏ

Tên của một mảng, khi dùng trong biểu thức, chính là địa chỉ phần tử đầu tiên. Vì vậy truyền mảng vào hàm thực chất là truyền địa chỉ, và hàm sửa được nội dung mảng. Hai cách viết tham số sau là **như nhau**:

```c
int split_csv_line (char *line, char *fields[], int max_fields);
int split_csv_line (char *line, char **fields,  int max_fields);
```

Với một con trỏ `p` trỏ vào giữa mảng, `p + 1` là địa chỉ của phần tử kế tiếp. Hàm `split_csv_line` dùng đúng điều này: `start = comma + 1` nghĩa là "trường kế tiếp bắt đầu ngay sau dấu phẩy".

### 3.5. `const`

`const char *text` nghĩa là "con trỏ tới các ký tự **không được sửa**". Hàm nhận tham số như vậy cam kết chỉ đọc chuỗi. Nếu lỡ viết lệnh sửa, gcc báo lỗi ngay. Hàm nào sửa chuỗi (như `split_csv_line`) thì không có `const`.

---

## 4. Kiến thức nền: chuỗi trong C

C không có kiểu "chuỗi" riêng. **Chuỗi là một mảng `char`, kết thúc bằng ký tự `'\0'`** (ký tự có mã bằng 0). Mọi hàm xử lý chuỗi đọc từ đầu cho tới khi gặp `'\0'` thì dừng.

```c
char name[8] = "Lamp";
```

```
chỉ số:   0     1     2     3     4      5    6    7
        ┌─────┬─────┬─────┬─────┬──────┬────┬────┬────┐
        │ 'L' │ 'a' │ 'm' │ 'p' │ '\0' │ ?  │ ?  │ ?  │
        └─────┴─────┴─────┴─────┴──────┴────┴────┴────┘
```

- Mảng 8 phần tử chứa được tối đa **7** ký tự, vì phải chừa 1 chỗ cho `'\0'`.
- Đó là lý do `TIME_TEXT_SIZE` bằng 20: chuỗi `2026:11:10 16:00:00` dài 19 ký tự, cộng 1 cho `'\0'`.

**Hệ quả quan trọng:** nếu ghi `'\0'` vào giữa một chuỗi, chuỗi bị "cắt" tại đó. Mọi hàm đọc chuỗi sẽ dừng ở vị trí mới. `split_csv_line` dựa hoàn toàn vào điều này.

Các hàm chuỗi dùng hôm nay (trong `<string.h>`):

| Hàm | Trả về | Ví dụ |
|---|---|---|
| `strlen (s)` | Số ký tự trước `'\0'` | `strlen ("Lamp")` là 4 |
| `strchr (s, c)` | Con trỏ tới ký tự `c` **đầu tiên** trong `s`, hoặc `NULL` nếu không có | `strchr ("a,b", ',')` trỏ vào dấu phẩy |
| `strcspn (s, tap)` | Số ký tự ở **đầu** `s` mà không thuộc `tap` | `strcspn ("abc\n", "\r\n")` là 3 |
| `strcmp (a, b)` | 0 nếu hai chuỗi giống hệt nhau | Dùng trong chương trình thử |
| `snprintf (dst, kich_thuoc, ...)` | In vào chuỗi `dst`, không bao giờ vượt `kich_thuoc` | Chép chuỗi an toàn |

**Không so sánh chuỗi bằng `==`.** `a == b` so sánh hai *địa chỉ*, không so sánh nội dung. Luôn dùng `strcmp (a, b) == 0`.

---

## 5. Kiến thức nền: struct và typedef

`struct` gom nhiều giá trị có liên quan thành một khối:

```c
typedef struct
{
  int code;
  char name[NAME_SIZE];
  double min_power;
  double max_power;
  double failure_power;
} LoadSpec;
```

- `struct { ... }` mô tả hình dạng khối dữ liệu.
- `typedef ... LoadSpec;` đặt tên ngắn cho hình dạng đó, để sau này viết `LoadSpec spec;` thay vì `struct { ... } spec;`.
- Truy cập trường bằng dấu chấm: `spec.min_power`. Nếu có con trỏ `const LoadSpec *p = &spec;` thì dùng mũi tên: `p->min_power`.
- Mảng struct: `LoadSpec specs[MAX_LOADS];` là 100 khối, `specs[2].name` là tên của loại tải thứ ba.

**Mỗi struct của nhóm ứng với đúng một dòng của một loại file:**

| Struct | Một dòng của file | Cột trong file → trường trong struct |
|---|---|---|
| `LoadSpec` | `load_specs.csv` | `code` → `code`, `name` → `name`, `min` → `min_power`, `max` → `max_power`, `failure` → `failure_power` |
| `Sample` | `power_sensor.csv` | `id` → `id`, `time` → `time`, `power` → `power`, `load` → `load` |
| `HourRecord` | `load_condition.csv` (Task 2.2 tạo ra) | `id`, `time`, `power`, `energy`, `load`, `condition` |

Đọc một dòng file = tách dòng thành các trường, kiểm tra từng trường, rồi điền vào một struct. Toàn bộ Task 2 là xử lý trên các mảng struct này.

---

## 6. Đọc file common.h từng dòng

### 6.1. Include guard

```c
#ifndef COMMON_H
#define COMMON_H
...
#endif /* COMMON_H */
```

Ở bước tiền xử lý, `#include` dán nguyên nội dung file vào. Nếu một file `.c` include `common.h` hai lần (trực tiếp, hoặc gián tiếp qua một file `.h` khác), ba struct bị khai báo hai lần và gcc báo lỗi `redefinition`.

Ba dòng này hoạt động như sau:
- Lần đầu: `COMMON_H` chưa được định nghĩa, nên `#ifndef` ("nếu chưa định nghĩa") đúng. Tiền xử lý đi vào, định nghĩa `COMMON_H`, và lấy toàn bộ nội dung.
- Lần sau: `COMMON_H` đã có, `#ifndef` sai, toàn bộ nội dung tới `#endif` bị bỏ qua.

### 6.2. Hai dòng include thư viện

```c
#include <stddef.h>   /* cho kiểu size_t */
#include <time.h>     /* cho kiểu time_t */
```

`common.h` dùng `size_t` (trong prototype của `format_time`) và `time_t` (trong `Sample`, `HourRecord`), nên phải include nơi định nghĩa hai kiểu đó. Nguyên tắc: **một file `.h` tự include đủ những gì nó cần**, để file nào include nó cũng không bị thiếu.

- `time_t`: số giây tính từ 00:00:00 ngày 1/1/1970 (giờ UTC). Thời gian lưu dạng số thì so sánh, cộng trừ và sắp xếp được. Đây là lý do `Sample` lưu thời gian bằng `time_t` chứ không giữ chuỗi.
- `size_t`: kiểu số nguyên không âm dùng cho kích thước, ví dụ kết quả của `sizeof`.

### 6.3. Các hằng số

```c
#define TIME_FORMAT "%Y:%m:%d %H:%M:%S"
#define TIME_TEXT_SIZE 20
#define NAME_SIZE 32
#define LINE_SIZE 256
#define MAX_LOADS 100
#define METER_MIN 1.0
#define METER_MAX 3000.0
#define LOAD_OK 0
#define LOAD_NOT_FOUND 1
#define LOAD_INVALID 2
```

| Hằng | Dùng để | Vì sao giá trị này |
|---|---|---|
| `TIME_FORMAT` | Mẫu in và đọc thời gian (buổi 2) | `%Y` năm 4 chữ số, `%m` tháng, `%d` ngày, `%H:%M:%S` giờ phút giây, đúng mẫu của đề |
| `TIME_TEXT_SIZE` | Kích thước bộ đệm chứa chuỗi thời gian | 19 ký tự + `'\0'` |
| `NAME_SIZE` | Độ dài tối đa tên loại tải | Tên trong đề dài nhất 7 ký tự; 32 là dư an toàn |
| `LINE_SIZE` | Kích thước bộ đệm đọc một dòng file | Dòng dữ liệu dài khoảng 30 ký tự; 256 là dư an toàn |
| `MAX_LOADS` | Số loại tải tối đa trong file tải | Đề có 6 loại; 100 là dư an toàn |
| `METER_MIN`, `METER_MAX` | Dải đo của đồng hồ, dùng ở Task 2.1 | Thông số kỹ thuật trong đề: 1 W đến 3000 W |
| `LOAD_OK`, `LOAD_NOT_FOUND`, `LOAD_INVALID` | Ba kết quả của `read_load_specs` (buổi 3) | Tên dễ hiểu hơn số 0, 1, 2 |

Vì sao dùng `#define` thay vì viết số trực tiếp: đề chấm phong cách lập trình 10% và trừ điểm khi "hard-coding". Hơn nữa, đọc `if (power > METER_MAX)` hiểu ngay, còn `if (power > 3000.0)` thì người đọc phải tự đoán 3000 là gì. Muốn đổi giá trị chỉ cần sửa một chỗ.

### 6.4. Ba struct

Đã giải thích ở mục 5. Ghi chú thêm về `HourRecord`:
- `time` là **đầu giờ**, ví dụ 16:00:00 đại diện cho khoảng 16:00:00 đến 16:59:59.
- `condition` lưu **mã số** 0 đến 5 theo Bảng 1 của đề, không lưu chữ. So sánh và lưu số thì gọn; chỉ lúc ghi ra file mới đổi mã thành chữ (`good`, `off`...).

### 6.5. Bảy prototype

```c
void log_error (const char *log_name, int code, const char *description);
int split_csv_line (char *line, char *fields[], int max_fields);
int parse_int (const char *text, int *value);
int parse_double (const char *text, double *value);
int parse_time (const char *text, time_t *value);
void format_time (time_t value, char *buffer, size_t size);
int read_load_specs (const char *path, LoadSpec specs[], int max_specs,
                     int *count);
```

Prototype là dòng đầu của hàm, kết thúc bằng dấu chấm phẩy, không có thân. Nó cho gcc biết tên hàm, kiểu từng tham số và kiểu trả về. Thân hàm nằm trong `common.c`: hôm nay có 2 hàm đầu, buổi 2 thêm 4 hàm, buổi 3 thêm `read_load_specs`.

Hai quy ước của cả nhóm, nhìn vào prototype là thấy:

1. **Hàm kiểm tra trả `int`: 1 là hợp lệ, 0 là không.** Kết quả thật đi qua tham số con trỏ: `int *value`, `double *value`, `time_t *value`, `int *count`.
2. **`const` cho biết hàm có sửa chuỗi hay không.** Chỉ `split_csv_line` nhận `char *line` không `const`, vì nó sửa dòng. `format_time` nhận `char *buffer` không `const` vì nó ghi kết quả vào bộ đệm.

---

## 7. Kiến thức nền: làm việc với file

Trong C, file được mở thành một **luồng** kiểu `FILE *` (trong `<stdio.h>`). Mọi thao tác đọc ghi đi qua con trỏ này.

```c
FILE *file = fopen ("ten_file.txt", "a");  /* mở */
if (file == NULL)                          /* luôn kiểm tra */
  return;
fprintf (file, "Error %02d: %s\n", 1, "abc");  /* ghi */
fclose (file);                             /* đóng */
```

| Chế độ mở | Nghĩa | File chưa có | File đã có |
|---|---|---|---|
| `"r"` | Đọc | `fopen` trả `NULL` | Đọc từ đầu |
| `"w"` | Ghi | Tạo mới | **Xóa sạch** nội dung cũ rồi ghi |
| `"a"` | Ghi nối (append) | Tạo mới | Giữ nội dung cũ, ghi thêm vào cuối |

`fopen` trả `NULL` khi thất bại: file không tồn tại (với `"r"`), không có quyền, hoặc file đang ở chế độ chỉ đọc (với `"w"`, `"a"`). Dùng con trỏ `NULL` để đọc ghi sẽ làm chương trình sập, nên **luôn kiểm tra ngay sau `fopen`**. Đây cũng chính là cách phát hiện các lỗi như Error 01 và Error 08 của Task 2.

`fclose` đẩy hết dữ liệu còn trong bộ đệm xuống đĩa rồi giải phóng file. Quên `fclose` có thể làm mất những dòng cuối.

**`fprintf` và định dạng:** giống `printf`, thêm tham số file ở đầu. Các định dạng dùng trong dự án:

| Định dạng | In ra | Ví dụ |
|---|---|---|
| `%d` | Số nguyên | `3` |
| `%02d` | Số nguyên, ít nhất 2 chữ số, thêm 0 ở đầu nếu thiếu | `1` in thành `01`, `12` in thành `12` |
| `%.1f` | Số thực, 1 chữ số sau dấu chấm (làm tròn) | `25.46` in thành `25.5` |
| `%s` | Chuỗi | `invalid command` |
| `\n` | Xuống dòng | |

---

## 8. Hàm log_error

### 8.1. Đề yêu cầu gì

> Chương trình cần lưu trữ mọi lỗi phát sinh trong quá trình thực thi vào các tệp log, KHÔNG in ra màn hình. [...] Mỗi lỗi phải được ghi vào một dòng [...] với định dạng `Error AB: DESCRIPTION`, AB là mã lỗi gồm 2 chữ số.

Task 2 ghi vào `task2.log`. Ví dụ: `Error 06: invalid data at line 3`.

### 8.2. Code

```c
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
```

### 8.3. Giải thích từng dòng

| Dòng | Giải thích |
|---|---|
| `void` | Hàm không trả giá trị gì. Ghi log là việc "làm xong là thôi", nơi gọi không cần biết kết quả. Kiểu trả về nằm riêng một dòng theo phong cách GNU. |
| `log_error (const char *log_name, int code, const char *description)` | Ba tham số: tên file log (ví dụ `"task2.log"`), mã lỗi (ví dụ `6`), mô tả (ví dụ `"invalid data at line 3"`). Hai chuỗi có `const` vì hàm chỉ đọc chúng. Tên file log là tham số, nên cả ba task dùng chung một hàm, mỗi task truyền tên log của mình. |
| `FILE *log_file;` | Biến giữ luồng file. |
| `log_file = fopen (log_name, "a");` | Mở ở chế độ **ghi nối**. Một lần chạy có thể gặp nhiều lỗi (ví dụ ba dòng dữ liệu sai): mỗi lỗi gọi `log_error` một lần, và mỗi lần phải *thêm* một dòng, không xóa các dòng trước. |
| `if (log_file == NULL) return;` | Không mở được file log thì không còn nơi nào để báo lỗi (đề cấm in ra màn hình), nên chỉ còn cách thoát hàm. |
| `fprintf (log_file, "Error %02d: %s\n", code, description);` | Ghi đúng một dòng theo mẫu của đề. `%02d` làm mã 6 thành `06`. `\n` kết thúc dòng. |
| `fclose (log_file);` | Đóng ngay, để dòng vừa ghi chắc chắn nằm trên đĩa, kể cả khi chương trình dừng đột ngột ngay sau đó. |

### 8.4. Những câu có thể bị hỏi

- **"Vì sao mở rồi đóng file mỗi lần ghi, không mở một lần ở đầu chương trình?"** Đơn giản và an toàn: không phải truyền con trỏ file đi khắp nơi, không sợ quên đóng, và mỗi dòng log nằm trên đĩa ngay khi ghi. Số lỗi trong một lần chạy ít nên chi phí mở file không đáng kể.
- **"Log cũ của lần chạy trước thì sao?"** Chế độ `"a"` giữ lại log cũ. Theo bảng quyết định của nhóm, `main` sẽ xóa log cũ một lần ở đầu mỗi lần chạy, bằng cách mở file với `"w"` rồi đóng ngay. Việc này làm ở buổi 7.

---

## 9. Hàm split_csv_line

### 9.1. Bài toán

File CSV là file văn bản, mỗi dòng gồm các trường cách nhau bằng dấu phẩy:

```
2,2026:11:10 15:16:03,107.8,3
```

Muốn kiểm tra và đọc từng trường, trước hết phải tách dòng thành 4 chuỗi riêng: `"2"`, `"2026:11:10 15:16:03"`, `"107.8"`, `"3"`.

Đề liệt kê các dòng sai cần phát hiện, trong đó có **trường bị trống**:

```
2,2026:11:10 15:16:03,,3     ← công suất trống (hai dấu phẩy liền nhau)
2,2026:11:10 15:16:03,107.8, ← mã tải trống (dấu phẩy ở cuối)
, , ,                        ← mọi trường đều trống
```

Vậy hàm tách phải **giữ được trường rỗng**: dòng thứ nhất phải ra đúng 4 trường, trong đó trường thứ ba là chuỗi rỗng.

### 9.2. Vì sao không dùng strtok

C có sẵn hàm `strtok` để tách chuỗi, nhưng nó coi **nhiều dấu phân cách liền nhau là một**. Với `2,2026:11:10 15:16:03,,3`, `strtok` trả về 3 trường `"2"`, `"2026:11:10 15:16:03"`, `"3"`. Trường công suất trống biến mất, và `"3"` bị hiểu nhầm là công suất. Chương trình không thể biết dòng này sai.

Vì vậy nhóm tự viết hàm tách bằng `strchr`. **Đây là câu rất dễ bị hỏi khi vấn đáp.**

### 9.3. Ý tưởng

1. Giữ một con trỏ `start` ở **đầu trường hiện tại**.
2. Ghi lại `start` vào mảng `fields`: đó là nơi trường hiện tại bắt đầu.
3. Tìm dấu phẩy kế tiếp tính từ `start`.
4. Nếu không còn dấu phẩy: trường hiện tại là trường cuối, xong.
5. Nếu có: **ghi `'\0'` đè lên dấu phẩy**. Trường hiện tại kết thúc tại đó, vì mọi hàm chuỗi dừng ở `'\0'`. Trường kế tiếp bắt đầu ngay sau vị trí đó. Quay lại bước 2.

Hàm **không chép** chuỗi nào. Nó chỉ cắt dòng gốc tại chỗ và ghi lại địa chỉ đầu mỗi đoạn. Nhanh và không tốn bộ nhớ thêm.

### 9.4. Code

```c
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
```

### 9.5. Giải thích từng dòng

| Dòng | Giải thích |
|---|---|
| `int split_csv_line (char *line, char *fields[], int max_fields)` | `line`: dòng cần tách, sẽ bị sửa nên không có `const`. `fields`: mảng **các con trỏ**, mỗi phần tử sẽ trỏ vào đầu một trường. `max_fields`: mảng `fields` có bao nhiêu chỗ, để không ghi tràn. Trả về số trường. |
| `char *start = line;` | Trường đầu tiên bắt đầu ở đầu dòng. |
| `char *comma;` | Sẽ chứa vị trí dấu phẩy tìm được. |
| `int count = 0;` | Số trường đã đếm. |
| `line[strcspn (line, "\r\n")] = '\0';` | **Cắt ký tự xuống dòng ở cuối.** `fgets` (dùng để đọc từng dòng file ở buổi 3) giữ lại ký tự xuống dòng; file tạo trên Windows còn có thêm `'\r'`. Nếu không cắt, trường cuối của `1,TV,40,200,250\n` sẽ là `"250\n"`, và bước kiểm tra số sẽ coi nó không hợp lệ. `strcspn` trả vị trí của `'\r'` hoặc `'\n'` đầu tiên; ghi `'\0'` vào đó là cắt. Nếu dòng không có ký tự xuống dòng, vị trí trả về là cuối chuỗi (đã là `'\0'`), nên lệnh vẫn an toàn. |
| `for (;;)` | Vòng lặp không có điều kiện dừng sẵn; thoát bằng `break` bên trong. Viết như vậy vì điều kiện dừng ("không còn dấu phẩy") chỉ biết được ở giữa thân vòng lặp. |
| `if (count < max_fields) fields[count] = start;` | Ghi địa chỉ đầu trường hiện tại vào mảng, **chỉ khi mảng còn chỗ**. Không có điều kiện này, dòng có nhiều trường hơn mảng sẽ ghi ra ngoài mảng, làm hỏng bộ nhớ của biến khác: một lỗi rất khó tìm. |
| `count++;` | Vẫn đếm mọi trường, kể cả khi mảng đã đầy. Nhờ vậy nơi gọi biết dòng có **thừa** trường (ví dụ dòng dữ liệu có 5 trường thay vì 4) và báo dòng sai. |
| `comma = strchr (start, ',');` | Tìm dấu phẩy đầu tiên **tính từ đầu trường hiện tại**. `start` luôn nằm sau những dấu phẩy đã xử lý, nên `strchr` chỉ thấy các dấu phẩy còn lại phía sau. |
| `if (comma == NULL) break;` | Không còn dấu phẩy: trường hiện tại kéo dài tới hết dòng, là trường cuối. Thoát vòng lặp. |
| `*comma = '\0';` | Ghi `'\0'` vào đúng vị trí dấu phẩy. Trường hiện tại kết thúc tại đây. |
| `start = comma + 1;` | Trường kế tiếp bắt đầu ở ký tự ngay sau dấu phẩy cũ. Nếu ký tự đó cũng là dấu phẩy (hai dấu phẩy liền nhau), lần lặp sau sẽ ghi lại một trường **rỗng**: đúng điều ta muốn. |
| `return count;` | Trả số trường. |

### 9.6. Chạy tay từng bước

Dòng `2,T,,3` (viết gọn thời gian thành `T`). Trước khi tách, bộ nhớ của `line`:

```
chỉ số:   0    1    2    3    4    5    6
        ┌────┬────┬────┬────┬────┬────┬──────┐
        │'2' │',' │'T' │',' │',' │'3' │'\0'  │
        └────┴────┴────┴────┴────┴────┴──────┘
```

| Vòng | `start` trỏ vào | Ghi vào `fields` | `count` sau | `strchr` tìm thấy | Ghi `'\0'` vào | `start` mới |
|---|---|---|---|---|---|---|
| 1 | chỉ số 0 | `fields[0]` = chỉ số 0 | 1 | dấu phẩy ở chỉ số 1 | chỉ số 1 | chỉ số 2 |
| 2 | chỉ số 2 | `fields[1]` = chỉ số 2 | 2 | dấu phẩy ở chỉ số 3 | chỉ số 3 | chỉ số 4 |
| 3 | chỉ số 4 | `fields[2]` = chỉ số 4 | 3 | dấu phẩy ở chỉ số 4 | chỉ số 4 | chỉ số 5 |
| 4 | chỉ số 5 | `fields[3]` = chỉ số 5 | 4 | `NULL` | | thoát |

Sau khi tách:

```
chỉ số:   0     1     2     3     4     5    6
        ┌────┬──────┬────┬──────┬──────┬────┬──────┐
        │'2' │'\0'  │'T' │'\0'  │'\0'  │'3' │'\0'  │
        └────┴──────┴────┴──────┴──────┴────┴──────┘
          ▲            ▲            ▲      ▲
      fields[0]    fields[1]    fields[2] fields[3]
        "2"          "T"          ""       "3"
```

`fields[2]` trỏ vào chỉ số 4, nơi ký tự đầu tiên đã là `'\0'`: đó là **chuỗi rỗng**. Trường trống được giữ lại, và hàm trả về 4. Ở buổi 3, bước kiểm tra sẽ thấy công suất là chuỗi rỗng và báo `Error 06` cho dòng này.

### 9.7. Hai điều cần nhớ khi dùng hàm

1. **Hàm làm hỏng dòng gốc.** Sau khi tách, `line` chỉ còn đọc được tới dấu phẩy đầu tiên (`"2"`). Nếu sau đó còn cần cả dòng (ví dụ để so dòng trùng), phải chép dòng ra một mảng khác *trước khi* tách.
2. **`fields` chỉ chứa địa chỉ trỏ vào bên trong `line`.** Nếu `line` bị ghi đè (ví dụ lần `fgets` kế tiếp đọc dòng mới vào cùng mảng), mọi trường cũ cũng thay đổi theo. Phải dùng xong các trường, chép số ra biến hay struct, rồi mới đọc dòng tiếp.

---

## 10. Chương trình thử và cách chạy

`tests\test_buoi1.c` gọi hai hàm với các đầu vào đã biết trước kết quả, rồi tự so sánh và in `DAT` hoặc `SAI` cho từng trường hợp. File này **không nộp**: nó giúp bạn chắc chắn hàm đúng trước khi dùng ở Task 2, và mỗi lần sửa hàm thì chạy lại để chắc không làm hỏng gì.

**Cách chạy:** bấm đúp `chay_thu_buoi1.bat` trong thư mục `EE3490E`. File này:
1. biên dịch `test_buoi1.c` cùng `common.c` với `-Wall -Wextra`;
2. chạy chương trình thử;
3. in nội dung `test.log` mà `log_error` vừa ghi;
4. lưu toàn bộ kết quả vào `ket_qua_buoi1.txt`.

**Kết quả đúng** là 8 dòng `DAT` và dòng cuối `Tong so truong hop SAI: 0`:

| Trường hợp | Kiểm tra điều gì |
|---|---|
| `log_error` ghi 2 dòng | Mẫu `Error AB: ...`, `%02d` (1 → `01`), ghi nối chứ không ghi đè |
| Dòng file tải có `\n` ở cuối | Cắt ký tự xuống dòng; trường cuối là `250`, không phải `250\n` |
| Công suất bị trống | Giữ được trường rỗng ở giữa dòng |
| Dòng toàn dấu phẩy và dấu cách | Giữ được trường rỗng ở đầu và cuối; dấu cách là ký tự thường, không bị bỏ |
| Không có dấu phẩy | Cả dòng là một trường |
| Dòng rỗng | Một trường rỗng |
| Kết thúc kiểu Windows `\r\n` | Cắt cả `'\r'` |
| 10 trường, mảng chứa 8 | Không ghi tràn mảng, nhưng vẫn báo đủ 10 trường |

**Tự làm để hiểu sâu hơn** (không bắt buộc): mở `test_buoi1.c`, thêm một lời gọi `check_split` với dòng của riêng bạn, ví dụ `"a,,b,"`. Đoán trước kết quả (số trường và từng trường), rồi chạy lại xem có đúng không.

---

## 11. Câu hỏi tự kiểm tra

Tự trả lời thành lời trước, rồi mới mở đáp án.

**Câu 1.** Nếu xóa ba dòng include guard, rồi `#include "common.h"` hai lần trong cùng một file `.c`, chuyện gì xảy ra?

<details><summary>Xem đáp án</summary>

Nội dung `common.h` bị dán vào hai lần, nên ba struct (`LoadSpec`, `Sample`, `HourRecord`) bị định nghĩa hai lần. gcc báo lỗi kiểu `redefinition of 'LoadSpec'` (hoặc `conflicting types`) và không biên dịch được. Include guard làm lần include thứ hai bị bỏ qua.
</details>

**Câu 2.** Vì sao `Sample` lưu thời gian bằng `time_t` mà không lưu chuỗi?

<details><summary>Xem đáp án</summary>

`time_t` là một số (số giây từ năm 1970). Task 2 cần so sánh thời gian (phát hiện dòng trùng), sắp xếp theo thời gian, và gom các mẫu theo từng giờ. Trên số thì làm được bằng phép so sánh và phép tính thông thường; trên chuỗi thì rất khó và dễ sai. Chỉ khi ghi ra file mới đổi số thành chuỗi.
</details>

**Câu 3.** Vì sao `split_csv_line` nhận `char *line`, còn `parse_int` nhận `const char *text`?

<details><summary>Xem đáp án</summary>

`split_csv_line` sửa chuỗi: nó thay mỗi dấu phẩy và ký tự xuống dòng bằng `'\0'`. `parse_int` chỉ đọc chuỗi để đổi ra số. `const` là cam kết không sửa; nếu lỡ viết lệnh sửa, gcc báo lỗi.
</details>

**Câu 4.** Với lệnh `power_process load_specs.csv data.csv`, `argc` bằng mấy và `argv[1]` là gì?

<details><summary>Xem đáp án</summary>

`argc` bằng 3. `argv[0]` là tên chương trình, `argv[1]` là `"load_specs.csv"`, `argv[2]` là `"data.csv"`. Kết quả chạy `kiem_tra.bat` đã cho thấy đúng như vậy.
</details>

**Câu 5.** `log_error` mở file bằng `"a"`. Nếu đổi thành `"w"`, và một lần chạy gặp 3 dòng dữ liệu sai, file log chứa gì?

<details><summary>Xem đáp án</summary>

Chỉ còn dòng lỗi **cuối cùng**. Mỗi lần mở bằng `"w"` xóa sạch nội dung cũ, nên hai dòng lỗi trước bị mất.
</details>

**Câu 6.** `printf ("Error %02d: %s\n", 7, "x")` in ra gì? Còn với 12?

<details><summary>Xem đáp án</summary>

`Error 07: x` và `Error 12: x`. `%02d` in ít nhất 2 chữ số và thêm 0 ở đầu khi số chỉ có 1 chữ số.
</details>

**Câu 7.** `split_csv_line` với dòng `"a,,b,"` trả về bao nhiêu, và các trường là gì?

<details><summary>Xem đáp án</summary>

Trả về 4: `"a"`, `""`, `"b"`, `""`. Dấu phẩy ở cuối tạo ra một trường rỗng cuối cùng. Đây đúng là trường hợp "mã tải trống" trong đề.
</details>

**Câu 8.** Vì sao hàm vẫn tăng `count` khi mảng `fields` đã đầy, thay vì dừng lại?

<details><summary>Xem đáp án</summary>

Để nơi gọi biết chính xác dòng có bao nhiêu trường. Một dòng dữ liệu phải có đúng 4 trường; nếu có 5 thì là dòng sai. Nếu hàm dừng đếm khi mảng đầy, nơi gọi chỉ thấy "4" và tưởng dòng hợp lệ.
</details>

**Câu 9.** Gọi `split_csv_line (line, fields, 4)` xong, bạn `printf ("%s", line)`. In ra gì nếu dòng ban đầu là `2,T,107.8,3`?

<details><summary>Xem đáp án</summary>

Chỉ in `2`. Dấu phẩy đầu tiên đã bị thay bằng `'\0'`, nên `printf` dừng ở đó. Đây là lý do phải chép dòng ra trước nếu còn cần cả dòng.
</details>

**Câu 10.** Tại sao `strtok` không dùng được cho bài này? Nêu một dòng cụ thể trong đề làm nó sai.

<details><summary>Xem đáp án</summary>

`strtok` coi nhiều dấu phẩy liền nhau là một nên bỏ mất trường rỗng. Với dòng `2,2026:11:10 15:16:03,,3` (công suất trống), `strtok` chỉ trả 3 trường và `"3"` bị hiểu là công suất, nên chương trình không phát hiện được dòng sai mà đề yêu cầu báo `Error 06`.
</details>

---

## 12. Thuật ngữ cần nói được khi vấn đáp

| Thuật ngữ | Nói ngắn gọn |
|---|---|
| Header (`.h`) | File chứa khai báo: hằng số, struct, prototype. Được `#include` vào các file `.c`. |
| Prototype | Dòng khai báo hàm (tên, tham số, kiểu trả về) không có thân, để gcc kiểm tra lời gọi hàm. |
| Include guard | Ba dòng `#ifndef / #define / #endif` ngăn một header bị đưa vào hai lần. |
| Tiền xử lý, biên dịch, liên kết | Ba bước của gcc: xử lý các dòng `#`, dịch từng file `.c`, ghép các file lại thành file chạy được. |
| Con trỏ | Biến chứa địa chỉ của một biến khác. `&` lấy địa chỉ, `*` truy cập giá trị tại địa chỉ. |
| Truyền tham trị | C chép giá trị đối số vào tham số; muốn hàm sửa biến bên ngoài thì truyền địa chỉ. |
| Chuỗi kết thúc bằng `'\0'` | Chuỗi C là mảng `char`, ký tự `'\0'` đánh dấu chỗ kết thúc. |
| `const` | Cam kết không sửa dữ liệu mà con trỏ trỏ tới. |
| Luồng file `FILE *` | Đối tượng đại diện cho một file đang mở; `fopen` mở, `fclose` đóng. |
| Chế độ ghi nối `"a"` | Giữ nội dung cũ, ghi thêm vào cuối file. |
| Hard-coding | Viết thẳng số hay chuỗi vào code thay vì đặt tên bằng `#define`. Đề trừ điểm. |

---

**Buổi 2** sẽ viết bốn hàm còn lại: `parse_int`, `parse_double` (kiểm tra một chuỗi có phải là số), `parse_time` và `format_time` (đổi qua lại giữa chuỗi thời gian và `time_t`).
