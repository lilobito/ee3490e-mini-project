# Buổi 7 – Task 2.3 và hàm main

Buổi này hoàn thành phần tính toán cuối cùng (Task 2.3: tổng công suất theo loại tải và giờ trong ngày) và ghép mọi phần lại trong `main`. Hết buổi này, `power_process` chạy đúng với dữ liệu bình thường: đó là mốc 4/12 trong kế hoạch.

Code liên quan trong `task2.c`: `load_index`, `contains`, `summarize_by_load`, `clear_log`, `main`.

> Mở file này trong VS Code rồi bấm `Ctrl + Shift + V` để xem dạng đã định dạng.

---

## Mục lục

1. [Đề yêu cầu gì ở Task 2.3](#1-đề-yêu-cầu-gì-ở-task-23)
2. [Kiến thức nền: mảng hai chiều](#2-kiến-thức-nền-mảng-hai-chiều)
3. [Hai hàm phụ: load_index và contains](#3-hai-hàm-phụ-load_index-và-contains)
4. [Hàm summarize_by_load](#4-hàm-summarize_by_load)
5. [Hàm main: ghép mọi phần lại](#5-hàm-main-ghép-mọi-phần-lại)
6. [Tự chạy power_process bằng dòng lệnh](#6-tự-chạy-power_process-bằng-dòng-lệnh)
7. [Câu hỏi tự kiểm tra](#7-câu-hỏi-tự-kiểm-tra)

---

## 1. Đề yêu cầu gì ở Task 2.3

> Tính toán tổng công suất tiêu thụ của tất cả các đồng hồ đo cùng loại tải, tức là cùng mã tải, cho mỗi giờ trong ngày trong suốt khoảng thời gian mô phỏng. [...] Thông tin bao gồm mã tải, thời gian trong ngày (từ 0 đến 23), số lượng đồng hồ đo cùng mã tải và tổng công suất của tất cả các đồng hồ đó tại cùng một giờ trong ngày. [...] Các giá trị công suất tiêu thụ này là công suất trung bình mỗi giờ thu được trong Tác vụ 2.2.

File `power_summary.csv`:

```
load,time,meters,power
1,0,2,79.3
1,1,2,9.0
...
1,23,2,0.0
2,0,5,242.3
...
```

| Cột | Ý nghĩa |
|---|---|
| `load` | Mã tải |
| `time` | **Giờ trong ngày**, từ 0 đến 23 (không phải ngày giờ đầy đủ) |
| `meters` | Số đồng hồ (cảm biến) **khác nhau** mang mã tải đó |
| `power` | Tổng các **công suất trung bình giờ** (lấy từ Task 2.2) của các cảm biến mang mã tải đó, tại giờ đó |

Ví dụ dòng `1,0,2,79.3`: loại tải 1 có 2 cảm biến; cộng công suất trung bình lúc 0 giờ của cả hai được 79.3.

**Đầu vào của Task 2.3 là kết quả của Task 2.2**, không phải các mẫu gốc. Đó là lý do `summarize_by_load` nhận mảng `HourRecord`.

### Nếu mô phỏng dài hơn một ngày

"Giờ trong ngày" là giờ trên đồng hồ, bất kể ngày nào. Nếu dữ liệu kéo dài 48 giờ, giờ 8 của hôm qua và giờ 8 của hôm nay **cộng vào cùng một ô**. Đề viết "trong suốt khoảng thời gian mô phỏng", nên nhóm cộng tất cả các ngày. Ghi lựa chọn này vào bảng quyết định.

---

## 2. Kiến thức nền: mảng hai chiều

Bảng tổng có một hàng cho mỗi loại tải và 24 cột cho 24 giờ. Mảng hai chiều có đúng hình dạng đó:

```c
double totals[MAX_LOADS][HOURS_PER_DAY];
```

```
                 giờ 0   giờ 1   ...   giờ 8   giờ 9  ...  giờ 23
                ┌──────┬──────┬─────┬──────┬──────┬─────┬──────┐
totals[0] (TV)  │      │      │     │      │      │     │      │
totals[1] (Lap) │      │      │     │      │      │     │      │
totals[2] (Des) │      │      │     │150.0 │ 40.0 │     │      │
...             │      │      │     │      │      │     │      │
                └──────┴──────┴─────┴──────┴──────┴─────┴──────┘
```

- `totals[2][8]` là ô của loại tải ở vị trí 2, giờ 8.
- Trong bộ nhớ, các hàng nằm nối tiếp nhau: toàn bộ hàng 0, rồi toàn bộ hàng 1...
- `MAX_LOADS` × `HOURS_PER_DAY` = 100 × 24 = 2400 số `double`, khoảng 19 KB.

**Chỉ số hàng là vị trí của loại tải trong mảng `specs`, không phải mã tải.** Mã tải có thể là 1, 5, 40 hay 999; dùng mã làm chỉ số thì mảng phải rất lớn và đa số ô bỏ trống. Hàm `load_index` đổi mã tải thành vị trí.

**Đặt mọi ô về 0 bằng `memset`:** `memset (totals, 0, sizeof totals);` ghi byte 0 vào toàn bộ mảng. Với `double`, toàn bộ byte bằng 0 chính là giá trị 0.0. `sizeof totals` của một mảng là kích thước **cả mảng**.

---

## 3. Hai hàm phụ: load_index và contains

```c
static int
load_index (const LoadSpec specs[], int count, int code)
{
  int i;

  for (i = 0; i < count; i++)
    if (specs[i].code == code)
      return i;
  return -1;
}
```

Giống `find_load` của buổi 5, nhưng trả về **vị trí** (để làm chỉ số hàng của bảng) thay vì con trỏ. Không tìm thấy thì trả −1.

```c
static int
contains (const int ids[], int count, int id)
{
  int i;

  for (i = 0; i < count; i++)
    if (ids[i] == id)
      return 1;
  return 0;
}
```

Cho biết `id` đã có trong danh sách `ids` chưa. Dùng để đếm số cảm biến **khác nhau**: mỗi cảm biến có nhiều bản ghi (mỗi giờ một bản ghi), nhưng chỉ được đếm một lần.

---

## 4. Hàm summarize_by_load

### 4.1. Code

```c
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
```

### 4.2. Phần 1: cộng dồn vào bảng

| Phần | Giải thích |
|---|---|
| `static double totals[...][...]` | Bảng tổng (mục 2). Khai báo `static` cho chắc không nằm trên ngăn xếp. Vì `static` giữ giá trị giữa các lần gọi, phải `memset` về 0 ở đầu hàm. |
| `int meters[MAX_LOADS]` | Số cảm biến của mỗi loại tải. |
| `counted`, mảng `counted_ids` | Danh sách các ID cảm biến đã đếm. `counted_ids` là mảng `static` khai báo ở đầu file, vì có thể có tới 10000 phần tử. |
| `index = load_index (...)`, `index < 0` thì `continue` | Bỏ qua bản ghi có mã tải không có trong file tải (trường hợp `Error 04` ở buổi 8). Không có hàng nào để cộng vào. |
| `hour = localtime (&records[i].time)->tm_hour;` | Lấy **giờ trong ngày** của bản ghi: đổi `time_t` thành `struct tm` rồi lấy trường `tm_hour` (0 đến 23). Mũi tên `->` vì `localtime` trả về con trỏ. |
| `totals[index][hour] += records[i].power;` | Cộng công suất trung bình giờ của bản ghi vào đúng ô. |
| Khối `if (!contains (...))` | Gặp một ID lần đầu: thêm vào danh sách, tăng số cảm biến của loại tải đó. Các lần sau gặp lại ID này thì bỏ qua. |

**Vì sao mỗi cảm biến chỉ tính cho đúng một loại tải?** Đề nói mã tải của mỗi cảm biến được chọn một lần và cố định suốt mô phỏng. Vì vậy lần đầu gặp một ID cũng là lần duy nhất cần đếm.

### 4.3. Phần 2: ghi file

| Phần | Giải thích |
|---|---|
| `fopen (SUMMARY_NAME, "w")` | Không mở được thì trả 0, `main` ghi `Error 08`. |
| `SUMMARY_HEADER` | `load,time,meters,power` |
| Vòng ngoài theo `i` | Duyệt các loại tải **theo thứ tự trong file tải**. |
| `if (meters[i] == 0) continue;` | Loại tải không có cảm biến nào thì không in. Ghi vào bảng quyết định. |
| Vòng trong theo `hour` từ 0 tới 23 | In đủ **24 dòng** cho mỗi loại tải, kể cả giờ có tổng bằng 0 (in `0.0`), giống ví dụ `1,23,2,0.0` của đề. |

### 4.4. Ví dụ với file mẫu

Từ 6 bản ghi của Task 2.2 (buổi 6):

| Bản ghi | Mã tải | Vị trí trong `specs` | Giờ | Cộng vào |
|---|---|---|---|---|
| ID 1, 08:00, 150.0 | 3 | 2 | 8 | `totals[2][8]` = 150.0 |
| ID 2, 08:00, 11.0 | 5 | 4 | 8 | `totals[4][8]` = 11.0 |
| ID 1, 09:00, 40.0 | 3 | 2 | 9 | `totals[2][9]` = 40.0 |
| ID 2, 09:00, 17.0 | 5 | 4 | 9 | `totals[4][9]` = 17.0 |
| ID 1, 10:00, 0.0 | 3 | 2 | 10 | `totals[2][10]` = 0.0 |
| ID 2, 10:00, 20.0 | 5 | 4 | 10 | `totals[4][10]` = 20.0 |

`meters[2]` = 1 (chỉ cảm biến 1), `meters[4]` = 1 (chỉ cảm biến 2). File ra có 48 dòng dữ liệu (2 loại tải × 24 giờ), trong đó các dòng khác 0 là:

```
3,8,1,150.0
3,9,1,40.0
5,8,1,11.0
5,9,1,17.0
5,10,1,20.0
```

---

## 5. Hàm main: ghép mọi phần lại

### 5.1. Luồng chương trình

```
xóa log cũ
    │
đọc tham số ──── sai ──► Error 05, dừng
    │
đọc file tải ─── không mở được ──► Error 10, dừng
    │            sai định dạng  ──► Error 03, dừng
kiểm tra file tải ── vô lý ──► Error 04, dừng
    │
đọc file dữ liệu ── không mở được ──► Error 01, dừng
    │               sai tiêu đề    ──► Error 02, dừng
    │   (trong lúc đọc: dòng sai Error 06, dòng trùng Error 07,
    │    quá 10000 dòng Error 09; vẫn chạy tiếp)
mã tải lạ? ── có ──► Error 04, vẫn chạy tiếp
    │
Task 2.1 ── không ghi được file ──► Error 08, dừng
    │
Task 2.2 ┐
Task 2.3 ┴─ không ghi được file ──► Error 08, dừng
    │
kết thúc thành công
```

### 5.2. Code

```c
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
    { ... Error 10 ... }
  if (result == LOAD_INVALID)
    { ... Error 03 ... }
  if (!check_load_specs (specs, spec_count))
    { ... Error 04 ... }

  result = read_samples (data_path, samples, sample_lines, MAX_SAMPLES,
                         &sample_count);
  if (result == READ_NOT_FOUND)
    { ... Error 01 ... }
  if (result == READ_BAD_FORMAT)
    { ... Error 02 ... }
  if (!all_loads_known (samples, sample_count, specs, spec_count))
    log_error (LOG_NAME, ERR_INCOMPATIBLE_LOAD, MSG_INVALID_LOAD);

  if (!filter_outliers (samples, sample_count, valid, &valid_count))
    { ... Error 08 ... }

  hour_count = compute_hourly (valid, valid_count, specs, spec_count, hours);
  if (!write_load_condition (hours, hour_count)
      || !summarize_by_load (hours, hour_count, specs, spec_count))
    { ... Error 08 ... }
  return EXIT_SUCCESS;
}
```

(Các dấu `...` là các khối `log_error (...); return EXIT_FAILURE;` giống khối đầu tiên; xem đầy đủ trong `task2.c`.)

### 5.3. Những điểm cần hiểu

| Điểm | Giải thích |
|---|---|
| `main` chỉ điều phối | Mọi việc thật nằm trong các hàm con. Đọc `main` là thấy toàn bộ luồng; đây là ý chính của thiết kế top-down mà đề yêu cầu. |
| `clear_log ()` ở đầu | Mở `task2.log` bằng `"w"` rồi đóng ngay, để file log chỉ chứa lỗi của lần chạy này. `log_error` sau đó ghi nối (buổi 1). |
| `LoadSpec specs[MAX_LOADS]` là biến cục bộ | 100 phần tử × khoảng 64 byte, khoảng 6 KB: đủ nhỏ để nằm trên ngăn xếp. Các mảng 10000 phần tử thì phải `static` (buổi 3). |
| `&load_path`, `&spec_count`... | Truyền địa chỉ để hàm con ghi kết quả vào biến của `main` (buổi 1). |
| `EXIT_SUCCESS`, `EXIT_FAILURE` | Hai hằng trong `<stdlib.h>` cho giá trị trả về của chương trình. Không in gì ra màn hình: đề yêu cầu mọi lỗi chỉ ghi vào log. |
| Lỗi nào dừng, lỗi nào chạy tiếp | Lỗi làm **không thể** làm tiếp (không có file, file sai định dạng, không ghi được file) thì dừng. Lỗi chỉ ảnh hưởng **một phần** dữ liệu (một dòng sai, một dòng trùng, quá 10000 dòng, mã tải lạ) thì ghi log và chạy tiếp với phần còn lại. |
| `\|\|` khi ghi hai file cuối | Nếu `write_load_condition` thất bại (trả 0, `!0` là đúng), `\|\|` dừng ngay, không gọi `summarize_by_load`. Chỉ ghi một dòng `Error 08`. |

### 5.4. Hàm clear_log

```c
static void
clear_log (void)
{
  FILE *file = fopen (LOG_NAME, "w");

  if (file != NULL)
    fclose (file);
}
```

Chế độ `"w"` xóa sạch nội dung file (hoặc tạo file rỗng nếu chưa có). Không ghi gì cả, chỉ mở rồi đóng. Đề không nói log ghi đè hay ghi nối giữa các lần chạy; nhóm chọn xóa ở đầu mỗi lần chạy để log dễ đọc, và ghi vào bảng quyết định.

---

## 6. Tự chạy power_process bằng dòng lệnh

`chay_thu_task2.bat` biên dịch chương trình thành `tests\task2\power_process.exe`. Để tự chạy với dữ liệu của bạn, trong VS Code mở Terminal (`Ctrl + ` `), rồi:

```
cd tests\task2\14_du_lieu_that
..\power_process.exe load_specs.csv data.csv
type load_condition.csv
type power_summary.csv
type task2.log
```

Hoặc tự biên dịch ở bất kỳ đâu:

```
gcc -Wall -Wextra -o power_process MSSV1_MSSV2_mini_project_20261\task_2\task2.c MSSV1_MSSV2_mini_project_20261\task_2\common.c
```

Khi A gửi file `power_sensor.csv` thật từ Task 1, đặt nó cùng thư mục với `load_specs.csv` và chạy `power_process` không tham số: chương trình dùng hai tên mặc định.

**Liên quan tới buổi này trong bộ test:**

| Trường hợp | Kiểm tra |
|---|---|
| `01_mau_chuan` | `power_summary.csv`: 2 loại tải × 24 giờ, đúng 5 giá trị khác 0 như mục 4.4 |
| `09_ma_tai_la` | Cảm biến mang mã tải 9 không có trong file tải: không xuất hiện trong `power_summary.csv` |
| `13_qua_lon` | Dữ liệu trải trên 3 ngày (8/11 đến 10/11): mỗi giờ trong ngày cộng dồn qua các ngày |
| `14_du_lieu_that` | 4 loại tải, mỗi loại 1 cảm biến |

---

## 7. Câu hỏi tự kiểm tra

**Câu 1.** Cột `time` của `power_summary.csv` khác cột `time` của `load_condition.csv` thế nào?

<details><summary>Xem đáp án</summary>

Trong `load_condition.csv` là ngày giờ đầy đủ của đầu giờ, ví dụ `2026:11:10 16:00:00`. Trong `power_summary.csv` chỉ là giờ trong ngày, một số từ 0 đến 23.
</details>

**Câu 2.** Task 2.3 cộng các giá trị gì: công suất gốc của từng mẫu hay thứ khác?

<details><summary>Xem đáp án</summary>

Cộng các công suất trung bình giờ đã tính ở Task 2.2 (mảng `HourRecord`), đúng như đề viết. Không dùng lại các mẫu gốc.
</details>

**Câu 3.** Vì sao chỉ số hàng của bảng `totals` là vị trí của loại tải trong `specs` chứ không phải mã tải?

<details><summary>Xem đáp án</summary>

Mã tải có thể là một số bất kỳ (1, 40, 999...). Dùng mã làm chỉ số thì mảng phải lớn bằng mã lớn nhất và hầu hết các ô bỏ trống, thậm chí tràn mảng. Vị trí trong `specs` luôn nằm trong khoảng 0 đến số loại tải trừ 1.
</details>

**Câu 4.** Cột `meters` được đếm thế nào? Vì sao cần hàm `contains`?

<details><summary>Xem đáp án</summary>

`meters` là số cảm biến khác nhau mang mã tải đó. Mỗi cảm biến có nhiều bản ghi (mỗi giờ một bản ghi), nên phải nhớ các ID đã đếm và chỉ tăng `meters` khi gặp một ID lần đầu. `contains` kiểm tra một ID đã có trong danh sách chưa.
</details>

**Câu 5.** Vì sao một loại tải có cảm biến luôn được in đủ 24 dòng, kể cả các giờ không có dữ liệu?

<details><summary>Xem đáp án</summary>

Đề yêu cầu mỗi giờ trong ngày từ 0 tới 23, và ví dụ có dòng `1,23,2,0.0`. Giờ không có dữ liệu có tổng 0.0.
</details>

**Câu 6.** Bảng `totals` được khai báo `static` trong hàm. Vì sao phải `memset` về 0 ở đầu hàm?

<details><summary>Xem đáp án</summary>

Biến `static` tồn tại suốt chương trình và giữ giá trị giữa các lần gọi hàm. Nó tự bằng 0 lúc chương trình bắt đầu, nhưng nếu hàm được gọi lần thứ hai thì còn giá trị của lần trước. `memset` bảo đảm mỗi lần gọi bắt đầu từ bảng sạch.
</details>

**Câu 7.** Kể tên các lỗi làm `power_process` dừng ngay, và các lỗi mà chương trình ghi log rồi chạy tiếp.

<details><summary>Xem đáp án</summary>

Dừng: Error 05 (sai lệnh), 10 (không mở được file tải), 03 (file tải sai định dạng), 04 khi thông số tải vô lý, 01 (không mở được file dữ liệu), 02 (file dữ liệu sai tiêu đề), 08 (không ghi được file kết quả). Chạy tiếp: 06 (dòng sai), 07 (dòng trùng), 09 (quá 10000 dòng, chỉ xử lý 10000 dòng đầu), 04 khi dữ liệu có mã tải lạ.
</details>

**Câu 8.** Vì sao `main` gọi `clear_log ()` đầu tiên?

<details><summary>Xem đáp án</summary>

`log_error` ghi nối vào cuối file. Nếu không xóa trước, lỗi của các lần chạy cũ vẫn còn trong `task2.log`. Xóa ở đầu mỗi lần chạy để log chỉ chứa lỗi của lần chạy hiện tại.
</details>

**Câu 9.** `if (!write_load_condition (...) || !summarize_by_load (...))`: nếu hàm đầu thất bại, hàm sau có được gọi không?

<details><summary>Xem đáp án</summary>

Không. Hàm đầu trả 0 nên `!0` là đúng; `||` dừng ngay khi vế trái đúng, nên `summarize_by_load` không được gọi. Chương trình ghi một dòng `Error 08` rồi dừng.
</details>
