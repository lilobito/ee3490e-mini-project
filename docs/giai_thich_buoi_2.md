# Buổi 2 – Giải thích chi tiết

Buổi này hoàn thành bốn hàm còn lại trong `common.c`:

| Hàm | Việc làm | Task 2 dùng ở đâu |
|---|---|---|
| `parse_int` | Kiểm tra một chuỗi có đúng là số nguyên không, nếu đúng thì đổi ra số | Đọc cột `id` và `load` của file dữ liệu, cột `code` của file tải |
| `parse_double` | Tương tự cho số thực | Đọc cột `power`; cột `min`, `max`, `failure` của file tải |
| `parse_time` | Đổi chuỗi `2026:11:10 16:00:00` thành `time_t`, từ chối mọi chuỗi sai mẫu hay ngày không tồn tại | Đọc cột `time` |
| `format_time` | Đổi ngược `time_t` thành chuỗi | Ghi cột `time` vào mọi file kết quả |

Đọc buổi 1 trước nếu chưa đọc: buổi này dùng lại con trỏ, chuỗi và quy ước "trả 1 nếu hợp lệ, kết quả qua con trỏ".

> Mở file này trong VS Code rồi bấm `Ctrl + Shift + V` để xem dạng đã định dạng.

---

## Mục lục

1. [Vì sao cần kiểm tra chuỗi trước khi đổi ra số](#1-vì-sao-cần-kiểm-tra-chuỗi-trước-khi-đổi-ra-số)
2. [Kiến thức nền: các kiểu số trong C và giới hạn của chúng](#2-kiến-thức-nền-các-kiểu-số-trong-c)
3. [Kiến thức nền: vì sao không dùng atoi và atof](#3-kiến-thức-nền-vì-sao-không-dùng-atoi-và-atof)
4. [Kiến thức nền: strtol, strtod và con trỏ end](#4-kiến-thức-nền-strtol-strtod-và-con-trỏ-end)
5. [Kiến thức nền: errno, isspace, isdigit](#5-kiến-thức-nền-errno-isspace-isdigit)
6. [Hàm parse_int](#6-hàm-parse_int)
7. [Hàm parse_double](#7-hàm-parse_double)
8. [Kiến thức nền: thời gian trong C](#8-kiến-thức-nền-thời-gian-trong-c)
9. [Hàm parse_time](#9-hàm-parse_time)
10. [Hàm format_time](#10-hàm-format_time)
11. [Chương trình thử và cách chạy](#11-chương-trình-thử-và-cách-chạy)
12. [Câu hỏi tự kiểm tra](#12-câu-hỏi-tự-kiểm-tra)
13. [Thuật ngữ cần nói được khi vấn đáp](#13-thuật-ngữ-cần-nói-được-khi-vấn-đáp)

---

## 1. Vì sao cần kiểm tra chuỗi trước khi đổi ra số

Sau khi `split_csv_line` tách dòng, mọi trường vẫn là **chuỗi**: `"2"`, `"2026:11:10 15:16:03"`, `"107.8"`, `"3"`. Muốn so sánh, cộng, tính trung bình thì phải đổi chúng ra số.

Đề liệt kê các dòng dữ liệu sai mà chương trình phải phát hiện và báo `Error 06: invalid data at line X`:

| Dòng trong đề | Trường sai | Hàm phát hiện |
|---|---|---|
| `, , ,` | Mọi trường trống | `parse_int` từ chối `""` và `" "` |
| `-2,2026:11:10 15:16:03,107.8,3` | ID âm | `parse_int` đọc được −2; nơi gọi kiểm tra ID phải lớn hơn 0 |
| `2,2026:11:10 15:16:,107.8,3` | Thời gian thiếu giây | `parse_time` từ chối |
| `2,2026:11:10 15:16:03,,3` | Công suất trống | `parse_double` từ chối `""` |
| `2,2026:11:10 15:16:03,107.8,` | Mã tải trống | `parse_int` từ chối `""` |

Vậy mỗi hàm phải làm **hai việc cùng lúc**: cho biết chuỗi có hợp lệ không, và nếu hợp lệ thì đưa ra giá trị. Đây là lý do có quy ước: **trả về 1 hoặc 0 cho biết hợp lệ; giá trị đi qua tham số con trỏ.**

```c
int id;

if (!parse_int (fields[0], &id) || id <= 0)
  {
    /* dòng sai: ghi Error 06 và bỏ qua dòng này */
  }
```

Lưu ý sự phân công: `parse_int` chỉ trả lời câu hỏi **"đây có phải là một số nguyên không?"**. Câu hỏi **"số đó có hợp lý không?"** (ID phải dương, công suất phải nằm trong dải đo...) do nơi gọi trả lời, vì mỗi trường có quy tắc riêng.

---

## 2. Kiến thức nền: các kiểu số trong C

| Kiểu | Kích thước trên Windows (MinGW) | Giá trị chứa được | Dùng trong dự án |
|---|---|---|---|
| `int` | 4 byte | khoảng −2,1 tỷ đến 2,1 tỷ (`INT_MIN` đến `INT_MAX`) | ID, mã tải, mã lỗi, số đếm |
| `long` | 4 byte trên Windows, 8 byte trên Linux | ít nhất bằng `int` | Kết quả tạm của `strtol` |
| `double` | 8 byte | khoảng ±1,8 × 10³⁰⁸, khoảng 15–16 chữ số có nghĩa | Công suất, năng lượng |
| `time_t` | 8 byte | số giây, đủ cho hàng tỷ năm | Thời gian |

`INT_MIN`, `INT_MAX`, `LONG_MAX` là các hằng có sẵn trong `<limits.h>`.

**Tràn số:** một kiểu chỉ chứa được một khoảng giá trị. Chuỗi `"99999999999"` (gần 100 tỷ) không vừa `int`. Nếu đổi bừa, kết quả là một số vô nghĩa. Hàm đổi số tử tế phải phát hiện và từ chối trường hợp này.

**Số thực không chính xác tuyệt đối:** `double` lưu số ở hệ nhị phân, nên 25.4 thực ra được lưu là 25,399999999999998... In bằng `%.1f` vẫn ra `25.4`, nên với bài này không ảnh hưởng. Điều cần nhớ là không nên so sánh hai số thực *tính toán ra* bằng `==`; còn so với 0 hoặc so với một giá trị vừa đọc từ chuỗi giống hệt thì an toàn.

---

## 3. Kiến thức nền: vì sao không dùng atoi và atof

C có sẵn `atoi` (chuỗi sang `int`) và `atof` (chuỗi sang `double`). Chúng gọn nhưng **không báo lỗi**:

| Lời gọi | Kết quả | Vấn đề |
|---|---|---|
| `atoi ("12")` | 12 | Đúng |
| `atoi ("0")` | 0 | Đúng |
| `atoi ("")` | 0 | Chuỗi rỗng cũng ra 0 |
| `atoi ("abc")` | 0 | Chữ cũng ra 0 |
| `atoi ("12a")` | 12 | Bỏ qua phần thừa, không báo gì |

Nhìn kết quả 0, không thể biết người dùng viết `"0"` hay viết bậy. Với công suất, `"0"` (tải tắt) là hợp lệ, còn `""` (trống) là dòng sai: `atof` trả 0.0 cho cả hai. **Đây là câu hay bị hỏi khi vấn đáp.**

---

## 4. Kiến thức nền: strtol, strtod và con trỏ end

`strtol` ("string to long") và `strtod` ("string to double") trong `<stdlib.h>` làm cùng việc, nhưng cho biết thêm **chúng dừng đọc ở đâu**:

```c
long strtol (const char *text, char **end, int base);
double strtod (const char *text, char **end);
```

- `text`: chuỗi cần đổi.
- `end`: **địa chỉ** của một biến con trỏ. Hàm ghi vào biến đó vị trí ký tự đầu tiên mà nó *không* đọc được.
- `base`: cơ số, ở đây là 10 (hệ thập phân). Buổi sau, Task 3 của A dùng cơ số 16 để đọc số hex.

Vì sao tham số là `char **end` (hai dấu sao)? Hàm cần *sửa* một biến con trỏ của nơi gọi. Theo buổi 1, muốn hàm sửa biến nào thì truyền địa chỉ biến đó. Biến là `char *end`, nên địa chỉ của nó có kiểu `char **`, và nơi gọi viết `&end`.

### Minh họa

```c
char *end;
long n = strtol (text, &end, 10);
```

Với `text = "12a"`:

```
chỉ số:   0     1     2     3
        ┌─────┬─────┬─────┬──────┐
        │ '1' │ '2' │ 'a' │ '\0' │
        └─────┴─────┴─────┴──────┘
          ▲           ▲
         text        end        n = 12
```

`strtol` đọc `1`, `2`, gặp `a` không phải chữ số nên dừng. `end` trỏ vào `a`.

### Ba trường hợp, phân biệt bằng end

| Chuỗi | `end` trỏ vào | Kết luận |
|---|---|---|
| `"12"` | `'\0'` ở cuối | **Hợp lệ**: đọc hết chuỗi |
| `"12a"` | `'a'` | Không hợp lệ: còn ký tự thừa |
| `""` hoặc `"abc"` | Đầu chuỗi (`end == text`) | Không hợp lệ: không đọc được chữ số nào |

Từ đó ra quy tắc: **chuỗi là một số trọn vẹn khi `end != text` (đọc được ít nhất một ký tự) và `*end == '\0'` (dừng đúng ở cuối chuỗi).**

---

## 5. Kiến thức nền: errno, isspace, isdigit

### errno

`errno` (trong `<errno.h>`) là một biến toàn cục mà các hàm thư viện ghi mã lỗi vào. Khi số quá lớn để vừa kiểu trả về, `strtol` và `strtod` đặt `errno` bằng `ERANGE`.

Cách dùng đúng: **đặt `errno = 0` ngay trước khi gọi**, rồi kiểm tra ngay sau. Thư viện không bao giờ tự đặt `errno` về 0, nên nếu không xóa trước, một lỗi cũ từ lần gọi khác có thể còn nằm đó và bị hiểu nhầm.

### isspace, isdigit

Trong `<ctype.h>`:
- `isspace (c)`: khác 0 nếu `c` là ký tự trắng (dấu cách, tab, xuống dòng...).
- `isdigit (c)`: khác 0 nếu `c` là chữ số `'0'` đến `'9'`.

Luôn ép ký tự sang `unsigned char` trước khi truyền vào: `isdigit ((unsigned char) text[i])`. Lý do: trên Windows, `char` có dấu, nên một ký tự có mã lớn hơn 127 (ví dụ chữ có dấu tiếng Việt trong file) thành số âm. Truyền số âm vào các hàm này là hành vi không xác định, có thể làm chương trình sập.

### Vì sao từ chối dấu cách ở đầu

`strtol` và `strtod` tự **bỏ qua** dấu cách ở đầu chuỗi, nên `" 5"` sẽ được đọc thành 5. Nhưng dấu cách ở cuối (`"5 "`) thì bị từ chối vì `*end` là dấu cách. Để nhất quán ("một số trọn vẹn, không có gì khác"), hai hàm của nhóm từ chối cả dấu cách ở đầu. File do Task 1 sinh ra không bao giờ có dấu cách, nên dấu cách xuất hiện nghĩa là file đã bị sửa tay hoặc sai định dạng.

---

## 6. Hàm parse_int

### 6.1. Code

```c
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
  if (number < INT_MIN || number > INT_MAX)
    return 0;
#endif
  *value = (int) number;
  return 1;
}
```

### 6.2. Giải thích từng dòng

| Dòng | Giải thích |
|---|---|
| `int parse_int (const char *text, int *value)` | `text` chỉ được đọc nên có `const`. `value` là địa chỉ nơi ghi kết quả. Trả về `int` làm cờ 1/0. |
| `char *end;` | Biến sẽ nhận vị trí `strtol` dừng đọc. |
| `long number;` | `strtol` trả `long`, nên nhận vào biến `long` trước, kiểm tra xong mới chuyển sang `int`. |
| `if (isspace ((unsigned char) text[0])) return 0;` | Từ chối chuỗi bắt đầu bằng ký tự trắng (mục 5). Với chuỗi rỗng, `text[0]` là `'\0'`, không phải ký tự trắng, nên đi tiếp và bị chặn ở dòng sau. |
| `errno = 0;` | Xóa mã lỗi cũ trước khi gọi `strtol` (mục 5). |
| `number = strtol (text, &end, 10);` | Đổi chuỗi theo cơ số 10. Truyền `&end` để `strtol` ghi vị trí dừng vào `end`. |
| `if (end == text \|\| *end != '\0' \|\| errno == ERANGE) return 0;` | Ba điều kiện từ chối: không đọc được chữ số nào; còn ký tự thừa sau số; số quá lớn so với `long`. Dấu `\|\|` là "hoặc": chỉ cần một điều kiện đúng là từ chối. |
| `#if LONG_MAX > INT_MAX` ... `#endif` | Chỉ biên dịch đoạn bên trong khi `long` rộng hơn `int` (ví dụ Linux, `long` 8 byte). Khi đó một số vừa `long` nhưng không vừa `int`, như 99999999999, phải bị chặn ở đây. Trên Windows, `long` và `int` cùng 4 byte, `strtol` đã báo `ERANGE` rồi, và đoạn này không cần; để nó lại còn làm gcc cảnh báo "phép so sánh luôn sai". |
| `*value = (int) number;` | Đã chắc chắn hợp lệ: ghi kết quả vào biến của nơi gọi. `(int)` là ép kiểu từ `long` sang `int`, an toàn vì đã kiểm tra khoảng. |
| `return 1;` | Báo thành công. |

**Điều quan trọng:** khi trả 0, hàm **không động vào** `*value`. Mọi lệnh `return 0` đều nằm trước dòng ghi kết quả.

---

## 7. Hàm parse_double

### 7.1. Code

```c
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
```

### 7.2. Khác gì parse_int

Cấu trúc giống hệt `parse_int`, chỉ khác ba điểm:

1. Dùng `strtod` (không có tham số cơ số), kết quả là `double`.
2. Không cần kiểm tra khoảng `int`.
3. Thêm `if (!isfinite (number)) return 0;`. `strtod` chấp nhận cả chuỗi `"inf"` (vô cực) và `"nan"` ("not a number", không phải số). Với công suất, cả hai đều vô nghĩa. `isfinite` (trong `<math.h>`) trả khác 0 khi số là một giá trị hữu hạn bình thường; dấu `!` đảo lại thành "không hữu hạn thì từ chối".

`errno == ERANGE` với `strtod` xảy ra khi số quá lớn (ví dụ `"1e400"`) hoặc quá gần 0 để biểu diễn (ví dụ `"1e-400"`). Cả hai đều không thể là công suất thật, nên từ chối luôn.

### 7.3. Các trường hợp

| Chuỗi | Kết quả | Lý do |
|---|---|---|
| `"25.4"` | 1, giá trị 25.4 | |
| `"-10.8"` | 1, giá trị −10.8 | Đọc được; Task 2.1 sẽ xếp nó vào ngoại lệ |
| `"3"` | 1, giá trị 3.0 | Số nguyên cũng là số thực hợp lệ |
| `""`, `" "` | 0 | Trống |
| `"1.2.3"` | 0 | `strtod` đọc `1.2`, dừng ở dấu chấm thứ hai |
| `"25.4W"` | 0 | Thừa chữ `W` |
| `"inf"`, `"nan"` | 0 | Không hữu hạn |

**Phân biệt quan trọng cho Task 2.1:** `"-10.8"` và `"3448.8"` là **số hợp lệ**, nên `parse_double` trả 1. Chúng không phải dòng sai (không ghi `Error 06`), mà là **ngoại lệ**: dòng đọc được nhưng công suất nằm ngoài dải đo, được ghi vào `power_outlier.csv`. Việc phân loại ngoại lệ làm ở buổi 4.

---

## 8. Kiến thức nền: thời gian trong C

### 8.1. time_t: thời gian là một con số

`time_t` là **số giây tính từ 00:00:00 ngày 1/1/1970 theo giờ UTC** (giờ quốc tế, gọi là "Unix epoch"). Ví dụ:

| Thời điểm (giờ Việt Nam) | Cùng thời điểm theo UTC | `time_t` |
|---|---|---|
| 2026:11:10 16:00:00 | 2026:11:10 09:00:00 | 1794301200 |
| 2026:11:10 17:00:00 | 2026:11:10 10:00:00 | 1794304800 |

Hai thời điểm cách nhau đúng 1 giờ thì `time_t` chênh đúng 3600. Đây là lý do lưu thời gian bằng `time_t`:
- So sánh: `t1 < t2` nghĩa là `t1` sớm hơn.
- Cộng trừ: lùi 24 giờ là `t - 24 * 3600`.
- Phát hiện dòng trùng: hai mẫu cùng ID và `time_t` bằng nhau.
- Gom theo giờ, sắp xếp theo thời gian (buổi 5).

Số 1794301200 chính là 4 byte `6A F2 DD 10` trong gói tin ví dụ của Task 3. Chương trình thử kiểm tra con số này.

### 8.2. struct tm: thời gian dạng từng phần

Con người đọc thời gian theo năm, tháng, ngày, giờ... Thư viện C biểu diễn dạng này bằng `struct tm` (trong `<time.h>`):

| Trường | Ý nghĩa | Khoảng giá trị | Cẩn thận |
|---|---|---|---|
| `tm_year` | Năm | | **Năm trừ đi 1900**: năm 2026 lưu là 126 |
| `tm_mon` | Tháng | 0 đến 11 | **Tháng bắt đầu từ 0**: tháng 11 lưu là 10 |
| `tm_mday` | Ngày trong tháng | 1 đến 31 | Bắt đầu từ 1 như bình thường |
| `tm_hour` | Giờ | 0 đến 23 | |
| `tm_min` | Phút | 0 đến 59 | |
| `tm_sec` | Giây | 0 đến 60 | |
| `tm_isdst` | Có đang dùng giờ mùa hè không | | Đặt −1 để thư viện tự xác định |

Hai cái bẫy năm trừ 1900 và tháng bắt đầu từ 0 là lỗi phổ biến nhất khi làm việc với thời gian trong C.

### 8.3. Bốn hàm chuyển đổi

```
                    mktime
   struct tm   ─────────────►   time_t
  (từng phần)  ◄─────────────  (số giây)
                   localtime
       │
       │ strftime
       ▼
     chuỗi "2026:11:10 16:00:00"
```

| Hàm | Đổi từ | Sang | Ghi chú |
|---|---|---|---|
| `mktime (&tm)` | `struct tm` (giờ địa phương) | `time_t` | Trả −1 nếu không đổi được. **Tự chuẩn hóa** các giá trị vượt khoảng (mục 8.4). |
| `localtime (&t)` | `time_t` | `struct tm` (giờ địa phương) | Trả con trỏ tới một vùng nhớ dùng chung; lần gọi sau ghi đè lần trước. |
| `strftime (buf, size, mau, &tm)` | `struct tm` | chuỗi theo mẫu | Trả số ký tự đã ghi, 0 nếu bộ đệm không đủ. |
| `time (NULL)` | | `time_t` của thời điểm hiện tại | Task 1 và Task 3 dùng |

"Giờ địa phương" là múi giờ đặt trên máy. Đề nói dùng múi giờ mặc định, không cần đổi. Máy ở Việt Nam là UTC+7, khớp với ví dụ của đề.

### 8.4. mktime tự chuẩn hóa, không báo lỗi

`mktime` không từ chối giá trị vượt khoảng mà **tự dồn sang đơn vị lớn hơn**:

| Đưa vào | mktime hiểu thành |
|---|---|
| ngày 31 tháng 11 | ngày 1 tháng 12 |
| tháng 13 (tức `tm_mon = 12`) | tháng 1 năm sau |
| ngày 29/2/2027 (năm thường) | ngày 1/3/2027 |

Tính năng này tiện khi cộng ngày ("ngày 28 cộng 5 ngày"), nhưng nghĩa là **`mktime` không giúp phát hiện ngày sai**. `parse_time` phải tự kiểm tra (mục 9).

### 8.5. Mẫu định dạng thời gian

`TIME_FORMAT` trong `common.h` là `"%Y:%m:%d %H:%M:%S"`:

| Ký hiệu | Nghĩa | Ví dụ |
|---|---|---|
| `%Y` | Năm 4 chữ số | 2026 |
| `%m` | Tháng 2 chữ số | 11 |
| `%d` | Ngày 2 chữ số | 10 |
| `%H` | Giờ 2 chữ số (00–23) | 16 |
| `%M` | Phút 2 chữ số | 00 |
| `%S` | Giây 2 chữ số | 00 |

Các ký tự khác (dấu `:`, dấu cách) được in nguyên văn. Kết quả: `2026:11:10 16:00:00`, đúng mẫu `YYYY:MM:DD hh:mm:ss` của đề.

### 8.6. sscanf

`sscanf (chuoi, mau, &a, &b, ...)` đọc các giá trị từ một chuỗi theo mẫu, giống `scanf` nhưng đọc từ chuỗi thay vì bàn phím. `%4d` nghĩa là "đọc một số nguyên, tối đa 4 chữ số". Mỗi giá trị đọc được ghi vào biến tương ứng, nên phải truyền **địa chỉ** các biến (`&year`...).

---

## 9. Hàm parse_time

### 9.1. Chiến lược ba lớp

Một chuỗi thời gian hợp lệ phải qua ba lớp kiểm tra:

1. **Đúng hình dạng:** dài đúng 19 ký tự; chữ số ở đúng vị trí chữ số, dấu `:` và dấu cách ở đúng vị trí của chúng.
2. **Đúng khoảng:** tháng 1–12, ngày 1–31, giờ 0–23, phút 0–59, giây 0–59.
3. **Ngày có thật:** loại các ngày như 31/11 hay 29/2 năm thường, bằng cách xem `mktime` có phải chuẩn hóa không.

### 9.2. Code

```c
int
parse_time (const char *text, time_t *value)
{
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

  if (result == (time_t) -1 || parts.tm_mday != day
      || parts.tm_mon != month - 1)
    return 0;
  *value = result;
  return 1;
}
```

### 9.3. Lớp 1: kiểm tra hình dạng bằng một chuỗi mẫu

```c
static const char pattern[] = "dddd:dd:dd dd:dd:dd";
```

Đây là "khuôn" của chuỗi thời gian. Chữ `d` nghĩa là "ở vị trí này phải có một chữ số"; mọi ký tự khác phải giống hệt.

```
pattern:  d d d d : d d : d d   d d : d d : d d
text:     2 0 2 6 : 1 1 : 1 0   1 6 : 0 0 : 0 0
chỉ số:   0 1 2 3 4 5 6 7 8 9 10 ...           18
```

| Dòng | Giải thích |
|---|---|
| `static const char pattern[]` | `static`: mảng chỉ tạo một lần cho cả chương trình, không tạo lại mỗi lần gọi hàm. `const`: không được sửa. |
| `const size_t length = sizeof pattern - 1;` | `sizeof pattern` là số byte của mảng, gồm cả `'\0'` ở cuối, tức 20. Trừ 1 ra độ dài chuỗi là 19. Tính từ mẫu thay vì viết số 19, nên không bị "hard-coding". |
| `if (strlen (text) != length) return 0;` | Sai độ dài thì chắc chắn sai: thiếu giây (`15:16:`), thừa ký tự, thiếu số 0 đầu (`9:00:00`). |
| Vòng `for` | Duyệt từng vị trí. Nếu khuôn là `d` thì ký tự phải là chữ số; nếu không thì ký tự phải đúng bằng ký tự của khuôn (`:` hoặc dấu cách). |

Sau lớp này, chuỗi chắc chắn có dạng `NNNN:NN:NN NN:NN:NN` với N là chữ số. Ví dụ `2026-11-10 16:00:00` bị loại ở vị trí 4 (`-` khác `:`).

### 9.4. Lớp 2: đọc sáu số và kiểm tra khoảng

| Dòng | Giải thích |
|---|---|
| `sscanf (text, "%4d:%2d:%2d %2d:%2d:%2d", &year, ...)` | Đọc sáu số. Lớp 1 đã bảo đảm hình dạng nên `sscanf` chắc chắn đọc đủ sáu số. |
| `if (month < 1 \|\| month > 12 \|\| ...)` | Loại tháng 0, tháng 13, ngày 0, ngày 32 trở lên, giờ 24 trở lên, phút và giây 60 trở lên. Không cần kiểm tra giờ, phút, giây âm: lớp 1 chỉ cho phép chữ số, không có dấu trừ. |

### 9.5. Lớp 3: dùng mktime để đổi và để bắt ngày không có thật

| Dòng | Giải thích |
|---|---|
| `memset (&parts, 0, sizeof parts);` | Ghi 0 vào mọi byte của `parts`. `struct tm` có thể có thêm trường ngoài các trường ta gán; xóa sạch trước để không còn giá trị rác. |
| `parts.tm_year = year - 1900;` | Năm trừ 1900 (mục 8.2). |
| `parts.tm_mon = month - 1;` | Tháng bắt đầu từ 0. |
| `parts.tm_mday = day;` ... `parts.tm_sec = second;` | Gán các phần còn lại, giữ nguyên. |
| `parts.tm_isdst = -1;` | Để thư viện tự xác định giờ mùa hè. Việt Nam không dùng giờ mùa hè nên không ảnh hưởng, nhưng đặt −1 là cách đúng chuẩn. |
| `result = mktime (&parts);` | Đổi sang `time_t`. **Đồng thời** `mktime` sửa lại các trường của `parts` về giá trị đã chuẩn hóa. |
| `if (result == (time_t) -1 \|\| parts.tm_mday != day \|\| parts.tm_mon != month - 1)` | `mktime` trả −1 là không đổi được. Nếu ngày hoặc tháng **bị đổi** sau khi gọi, nghĩa là ngày ban đầu không tồn tại. |

Ví dụ với `2026:11:31 00:00:00`:

```
trước mktime:  tm_mon = 10 (tháng 11), tm_mday = 31
sau mktime:    tm_mon = 11 (tháng 12), tm_mday = 1    ← đã bị dồn
so sánh:       tm_mday (1) != day (31)                → trả 0
```

| Dòng | Giải thích |
|---|---|
| `*value = result; return 1;` | Qua cả ba lớp: ghi kết quả, báo thành công. |

---

## 10. Hàm format_time

### 10.1. Code

```c
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
```

### 10.2. Giải thích từng dòng

| Dòng | Giải thích |
|---|---|
| `void format_time (time_t value, char *buffer, size_t size)` | `value`: thời gian cần in. `buffer`: mảng `char` của nơi gọi, nơi ghi chuỗi kết quả; không có `const` vì hàm ghi vào. `size`: số phần tử của mảng, để không ghi tràn. |
| `struct tm *parts;` | Con trỏ sẽ nhận kết quả của `localtime`. |
| `if (size == 0) return;` | Bộ đệm không có chỗ nào thì không làm gì, kể cả ghi `'\0'`. |
| `parts = localtime (&value);` | Đổi số giây thành năm, tháng, ngày... theo giờ của máy. `localtime` nhận **địa chỉ** của `time_t`. |
| `if (parts == NULL \|\| strftime (...) == 0) buffer[0] = '\0';` | `localtime` có thể trả `NULL` với thời gian quá xa. `strftime` trả 0 khi bộ đệm quá nhỏ. Trong cả hai trường hợp, đặt bộ đệm thành chuỗi rỗng để nơi gọi không in ra rác. Thứ tự quan trọng: `\|\|` dừng ngay khi vế trái đúng, nên `strftime` chỉ được gọi khi `parts` khác `NULL`. |

### 10.3. Cách gọi đúng

```c
char text[TIME_TEXT_SIZE];

format_time (sample.time, text, sizeof text);
fprintf (output, "%d,%s,%.1f,%d\n", sample.id, text, sample.power, sample.load);
```

`sizeof text` là kích thước của mảng (20), truyền vào để hàm biết giới hạn. `parse_time` và `format_time` là cặp ngược nhau: chuỗi → `parse_time` → `format_time` cho lại đúng chuỗi ban đầu. Chương trình thử kiểm tra đúng tính chất này.

---

## 11. Chương trình thử và cách chạy

`tests\test_buoi2.c` có 37 trường hợp: 11 cho `parse_int`, 10 cho `parse_double`, 14 cho `parse_time` kèm `format_time`, và 2 phép tính trên `time_t` (16:00 đến 17:00 là 3600 giây; Unix time của `2026:11:10 16:00:00` bằng 1794301200 như ví dụ Task 3).

**Cách chạy, chọn một trong hai:**
- Bấm đúp `chay_thu_buoi2.bat`. File này chạy **cả** chương trình thử của buổi 1 lẫn buổi 2. Sửa `common.c` xong, chạy lại cả hai là cách chắc chắn không làm hỏng hàm cũ. Kết quả lưu vào `ket_qua_buoi2.txt`.
- Trong VS Code (đã cập nhật cấu hình theo hướng dẫn trong chat): mở `tests\test_buoi2.c`, bấm `Ctrl + Shift + B`.

**Kết quả đúng:** mọi dòng là `DAT`, dòng cuối của mỗi chương trình là `Tong so truong hop SAI: 0`.

Nếu thấy dòng `CANH BAO Unix time khac vi du Task 3`: múi giờ của máy không phải UTC+7. Vào Settings → Time & language → Date & time, kiểm tra múi giờ là `(UTC+07:00) Bangkok, Hanoi, Jakarta`.

---

## 12. Câu hỏi tự kiểm tra

**Câu 1.** Vì sao không dùng `atoi` để đọc cột ID?

<details><summary>Xem đáp án</summary>

`atoi` không báo lỗi: chuỗi rỗng, chữ, và `"0"` đều cho 0; `"12a"` cho 12. Chương trình không phân biệt được trường trống hay sai với số hợp lệ, nên không phát hiện được các dòng sai mà đề yêu cầu báo `Error 06`.
</details>

**Câu 2.** Sau `strtol (text, &end, 10)`, làm sao biết chuỗi là một số trọn vẹn?

<details><summary>Xem đáp án</summary>

`end != text` (đọc được ít nhất một chữ số) và `*end == '\0'` (dừng đúng ở cuối chuỗi). Thêm `errno` khác `ERANGE` để chắc số không quá lớn.
</details>

**Câu 3.** Vì sao tham số `end` của `strtol` có kiểu `char **`?

<details><summary>Xem đáp án</summary>

`strtol` cần ghi vị trí dừng vào biến `end` của nơi gọi. Muốn hàm sửa một biến thì truyền địa chỉ biến đó. `end` có kiểu `char *`, nên địa chỉ của nó có kiểu `char **`, và nơi gọi viết `&end`.
</details>

**Câu 4.** Vì sao phải viết `errno = 0;` trước khi gọi `strtol`?

<details><summary>Xem đáp án</summary>

Thư viện chỉ ghi `errno` khi có lỗi, không bao giờ tự xóa về 0. Nếu không xóa trước, một giá trị `ERANGE` còn sót từ lần gọi khác sẽ làm một số hợp lệ bị từ chối nhầm.
</details>

**Câu 5.** `parse_int ("-2", &id)` trả gì? Vậy dòng có ID âm bị phát hiện ở đâu?

<details><summary>Xem đáp án</summary>

Trả 1 và `id` bằng −2, vì −2 là một số nguyên hợp lệ. Nơi gọi (hàm đọc dòng dữ liệu ở buổi 3) kiểm tra thêm `id > 0`; không thỏa thì dòng sai, ghi `Error 06`. `parse_int` chỉ trả lời "có phải số nguyên không", còn "có hợp lý không" là việc của nơi gọi.
</details>

**Câu 6.** `parse_double ("-10.8", &p)` trả gì? Dòng có công suất −10.8 là dòng sai hay ngoại lệ?

<details><summary>Xem đáp án</summary>

Trả 1, `p` bằng −10.8. Dòng đọc được nên **không** phải dòng sai. Công suất nằm ngoài dải đo (không bằng 0, không nằm trong 1–3000) nên đó là **ngoại lệ**, được ghi vào `power_outlier.csv` ở Task 2.1, không ghi log.
</details>

**Câu 7.** Trong `struct tm`, ngày 10/11/2026 được lưu với `tm_year`, `tm_mon`, `tm_mday` bằng bao nhiêu?

<details><summary>Xem đáp án</summary>

`tm_year` = 126 (2026 − 1900), `tm_mon` = 10 (tháng bắt đầu từ 0), `tm_mday` = 10.
</details>

**Câu 8.** `mktime` với ngày 31/11 cho kết quả gì, và `parse_time` dựa vào đó thế nào?

<details><summary>Xem đáp án</summary>

`mktime` không báo lỗi mà tự chuẩn hóa thành ngày 1/12, đồng thời sửa `tm_mday` thành 1 và `tm_mon` thành 11. `parse_time` so sánh lại: `tm_mday` (1) khác ngày ban đầu (31) nên kết luận ngày không tồn tại và trả 0.
</details>

**Câu 9.** Vì sao `parse_time` từ chối `2026:11:10 9:00:00` dù đây là một thời điểm có thật?

<details><summary>Xem đáp án</summary>

Chuỗi chỉ dài 18 ký tự và giờ chỉ có 1 chữ số, không đúng mẫu `YYYY:MM:DD hh:mm:ss` của đề (mỗi phần phải đủ chữ số). Lớp kiểm tra hình dạng loại ngay từ bước so độ dài.
</details>

**Câu 10.** `2026:11:10 16:00:00` và `2026:11:10 17:00:00` sau khi `parse_time` chênh nhau bao nhiêu? Vì sao điều này quan trọng cho Task 2?

<details><summary>Xem đáp án</summary>

Chênh đúng 3600 (giây). Vì thời gian là số giây, Task 2 so sánh, sắp xếp và gom các mẫu theo giờ bằng phép tính số học bình thường, thay vì phải xử lý chuỗi.
</details>

**Câu 11.** `strftime` trả 0 trong trường hợp nào, và `format_time` xử lý thế nào?

<details><summary>Xem đáp án</summary>

Khi bộ đệm không đủ chỗ cho cả chuỗi kết quả và `'\0'`. `format_time` khi đó ghi `'\0'` vào đầu bộ đệm, để nơi gọi nhận chuỗi rỗng thay vì rác.
</details>

---

## 13. Thuật ngữ cần nói được khi vấn đáp

| Thuật ngữ | Nói ngắn gọn |
|---|---|
| `strtol`, `strtod` | Đổi chuỗi sang `long` / `double` và cho biết vị trí dừng đọc qua con trỏ `end`. |
| Con trỏ `end` | Trỏ vào ký tự đầu tiên không thuộc về số; dùng để biết chuỗi có phải số trọn vẹn không. |
| `errno`, `ERANGE` | Biến toàn cục chứa mã lỗi của thư viện; `ERANGE` nghĩa là số vượt khoảng của kiểu. |
| Tràn số | Giá trị lớn hơn khoảng mà kiểu dữ liệu chứa được. |
| `time_t`, Unix time | Số giây tính từ 00:00:00 1/1/1970 UTC. |
| `struct tm` | Thời gian dạng từng phần; năm trừ 1900, tháng bắt đầu từ 0. |
| `mktime` | `struct tm` (giờ địa phương) sang `time_t`; tự chuẩn hóa giá trị vượt khoảng. |
| `localtime` | `time_t` sang `struct tm` theo giờ địa phương. |
| `strftime` | In `struct tm` thành chuỗi theo mẫu, ví dụ `%Y:%m:%d %H:%M:%S`. |
| Chuẩn hóa (normalize) | `mktime` tự dồn giá trị vượt khoảng: 31/11 thành 1/12. |
| Giờ địa phương và UTC | Việt Nam là UTC+7: 16:00 giờ Việt Nam là 09:00 UTC. |

---

**Buổi 3** sẽ viết `read_load_specs` (hàm cuối của `common.c`, đọc file thông số tải) và bắt đầu Task 2: đọc tham số dòng lệnh và đọc file dữ liệu vào mảng `Sample`. Từ buổi 3, các hàm của buổi 1 và 2 được dùng thật.
