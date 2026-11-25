# Buổi 3 – Đọc file thông số tải và file dữ liệu

Buổi này hoàn thành hàm cuối của `common.c` (`read_load_specs`) và phần đầu của chương trình `power_process`: nhận tham số dòng lệnh và đọc file dữ liệu vào bộ nhớ.

Từ buổi này, code nằm trong hai file:
- `task_2\common.c`: thêm `read_load_specs`.
- `task_2\task2.c`: **toàn bộ** chương trình `power_process`. File đã viết đủ; các buổi 3 đến 8 lần lượt giải thích từng phần của nó.

> Mở file này trong VS Code rồi bấm `Ctrl + Shift + V` để xem dạng đã định dạng. Mở `task2.c` song song (kéo tab sang nửa phải màn hình) để vừa đọc vừa nhìn code.

---

## Mục lục

1. [Bản đồ chương trình power_process](#1-bản-đồ-chương-trình-power_process)
2. [Kiến thức nền: đọc file từng dòng bằng fgets](#2-kiến-thức-nền-đọc-file-từng-dòng-bằng-fgets)
3. [Hàm read_load_specs](#3-hàm-read_load_specs)
4. [Kiến thức nền: static và mảng lớn](#4-kiến-thức-nền-static-và-mảng-lớn)
5. [Phần đầu của task2.c: hằng số và mảng](#5-phần-đầu-của-task2c-hằng-số-và-mảng)
6. [Hàm parse_arguments](#6-hàm-parse_arguments)
7. [Hàm parse_sample_line](#7-hàm-parse_sample_line)
8. [Hàm read_samples](#8-hàm-read_samples)
9. [Chạy thử](#9-chạy-thử)
10. [Câu hỏi tự kiểm tra](#10-câu-hỏi-tự-kiểm-tra)

---

## 1. Bản đồ chương trình power_process

### 1.1. Chương trình làm gì

```
                     ┌─────────────────┐
 load_specs.csv ───► │                 │ ───► power_outlier.csv   (Task 2.1)
                     │  power_process  │ ───► power_valid.csv     (Task 2.1)
 power_sensor.csv ─► │                 │ ───► load_condition.csv  (Task 2.2)
                     │                 │ ───► power_summary.csv   (Task 2.3)
                     └─────────────────┘ ───► task2.log           (lỗi)
```

Lệnh chạy: `power_process [file_tai] [file_du_lieu]`. Thiếu tên nào thì dùng tên mặc định `load_specs.csv`, `power_sensor.csv`.

### 1.2. Sơ đồ top-down (đề yêu cầu vẽ trong báo cáo)

```
main
├── clear_log                    xóa log của lần chạy trước
├── parse_arguments              buổi 3
├── read_load_specs   (common.c) buổi 3
├── check_load_specs             buổi 8
├── read_samples                 buổi 3
│   ├── parse_sample_line        buổi 3
│   │   └── split_csv_line, parse_int, parse_time, parse_double (common.c)
│   ├── find_duplicate           buổi 8
│   └── log_line_error           buổi 8
├── all_loads_known              buổi 8
│   └── find_load
├── filter_outliers              buổi 4   (Task 2.1)
│   ├── is_valid_power
│   └── write_samples
├── compute_hourly               buổi 5, 6 (Task 2.2)
│   ├── compare_samples, hour_start, find_load, classify_condition
│   └── compare_hours
├── write_load_condition         buổi 6
├── summarize_by_load            buổi 7   (Task 2.3)
│   ├── load_index, contains
└── log_error         (common.c) mọi chỗ báo lỗi
```

**Lý do tổ chức như vậy** (viết vào báo cáo):
- `main` chỉ điều phối: đọc `main` là thấy toàn bộ luồng, từ đọc file tới ba phần 2.1, 2.2, 2.3, đúng yêu cầu "tất cả nhiệm vụ con của Task 2 trong cùng một chương trình".
- Mỗi hàm làm một việc và trả kết quả rõ ràng (1/0 hoặc một mã), nên kiểm tra riêng được từng hàm.
- Phần đọc và kiểm tra chuỗi nằm trong `common.c`, dùng chung với Task 1 và Task 3.

### 1.3. Dữ liệu chảy qua các mảng thế nào

```
power_sensor.csv ──read_samples──► samples[]  ──filter_outliers──► valid[]
                                   (mọi dòng                       (chỉ công suất
                                    đọc được)                       hợp lệ)
                                                                      │
                                                              compute_hourly
                                                                      ▼
                                   power_summary.csv ◄──────────── hours[]
                                   (summarize_by_load)              (một bản ghi cho
                                                                     mỗi cảm biến, mỗi giờ)
```

---

## 2. Kiến thức nền: đọc file từng dòng bằng fgets

```c
char line[LINE_SIZE];

while (fgets (line, sizeof line, file) != NULL)
  {
    line[strcspn (line, "\r\n")] = '\0';
    /* xử lý một dòng */
  }
```

| Điểm cần hiểu | Giải thích |
|---|---|
| `fgets (line, sizeof line, file)` | Đọc **một dòng** (tới ký tự xuống dòng, và giữ lại ký tự đó) vào `line`, nhiều nhất `sizeof line - 1` ký tự, rồi thêm `'\0'`. |
| Trả về `NULL` | Khi hết file (hoặc lỗi đọc). Đó là điều kiện dừng vòng lặp. |
| Vì sao cắt `"\r\n"` | `fgets` giữ ký tự xuống dòng. File tạo trên Windows kết thúc dòng bằng `\r\n`. Buổi 1 đã giải thích dòng `strcspn`. |
| Dòng dài hơn bộ đệm | `fgets` cắt dòng thành nhiều lần đọc. `LINE_SIZE` là 256, trong khi một dòng dữ liệu hợp lệ dài khoảng 30 ký tự, nên chỉ dòng rác mới dài như vậy. |

**Đọc dòng tiêu đề riêng:** dòng đầu tiên được đọc bằng một lệnh `fgets` trước vòng lặp, rồi so với tiêu đề chuẩn bằng `strcmp`. Không khớp là file sai định dạng.

---

## 3. Hàm read_load_specs

### 3.1. Đề yêu cầu gì

File thông số tải có dạng:

```
code,name,min,max,failure
1,TV,40,200,250
...
```

`read_load_specs` đọc file này vào mảng `LoadSpec`. Cả Task 1 và Task 2 đều dùng, nhưng mỗi task báo lỗi bằng mã khác nhau (Task 1: Error 03 và 04; Task 2: Error 10 tự đề xuất và Error 03). Vì vậy hàm **không ghi log**, chỉ trả về một trong ba kết quả `LOAD_OK`, `LOAD_NOT_FOUND`, `LOAD_INVALID`, và mỗi chương trình tự đổi kết quả thành mã lỗi của mình.

### 3.2. Code

```c
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
```

Ở đầu `common.c` có thêm hai hằng: `LOAD_HEADER` là `"code,name,min,max,failure"`, `LOAD_FIELDS` là 5.

### 3.3. Giải thích từng phần

**Tham số:** `path` là tên file; `specs` là mảng của nơi gọi, nơi ghi các loại tải; `max_specs` là số chỗ trong mảng; `count` là địa chỉ biến nhận số loại tải đọc được.

**Mẫu "một biến `result`, một chỗ `fclose`":** thay vì `return` ở nhiều chỗ (mỗi chỗ phải nhớ `fclose`), hàm dùng biến `result`. Gặp lỗi thì gán `result = LOAD_INVALID` và thoát vòng lặp; cuối hàm chỉ có **một** lệnh `fclose`. Không thể quên đóng file. Riêng trường hợp `fopen` thất bại thì `return` ngay, vì chưa có gì để đóng.

| Đoạn | Giải thích |
|---|---|
| `*count = 0;` | Đặt số loại tải về 0 trước, để nơi gọi không bao giờ thấy một giá trị rác. |
| `fopen (path, "r")`, `NULL` thì `return LOAD_NOT_FOUND` | Không mở được: file không tồn tại hoặc không có quyền đọc. |
| `fgets` dòng đầu, `NULL` thì `LOAD_INVALID` | File rỗng (0 byte). |
| `strcmp (line, LOAD_HEADER) != 0` | Dòng đầu không đúng tiêu đề chuẩn: ví dụ thiếu tiêu đề, hoặc dùng dấu chấm phẩy. |
| `while (result == LOAD_OK && fgets (...) != NULL)` | Đọc tiếp khi chưa có lỗi và chưa hết file. Thứ tự hai vế quan trọng: `&&` dừng ngay khi vế trái sai, nên khi đã lỗi thì không đọc thêm dòng nào. |
| `if (line[0] == '\0') continue;` | Bỏ qua dòng trống (thường là dòng trống thừa ở cuối file). |
| Chuỗi điều kiện dài nối bằng `\|\|` | Một dòng hợp lệ phải: có đúng 5 trường; mã tải là số nguyên dương; tên không rỗng và vừa mảng `name` (dưới 32 ký tự); ba công suất là số; mảng còn chỗ. Sai bất kỳ điều nào thì cả file không hợp lệ. Nhờ `\|\|` dừng ở vế đúng đầu tiên, các hàm `parse_*` phía sau chỉ được gọi khi các trường phía trước đã đúng. |
| `strcpy (spec.name, fields[1]);` | Chép tên vào struct. An toàn vì đã kiểm tra `strlen (fields[1]) < NAME_SIZE`. Phải **chép** chứ không giữ con trỏ `fields[1]`, vì `fields[1]` trỏ vào `line`, mà `line` bị ghi đè ở lần `fgets` sau. |
| `specs[*count] = spec; (*count)++;` | Gán cả struct một lần (C cho phép gán struct bằng `=`) rồi tăng số đếm. Phải viết `(*count)++`, có ngoặc: `*count++` sẽ tăng con trỏ chứ không tăng giá trị. |
| `*count == 0` thì `LOAD_INVALID` | File chỉ có tiêu đề, không có loại tải nào. |
| `result != LOAD_OK` thì `*count = 0` | Khi file sai, không để lại số đếm dở dang. |

**Hàm này chỉ kiểm tra định dạng.** Các lỗi về ý nghĩa (min lớn hơn max, hai dòng trùng mã tải...) do `check_load_specs` trong `task2.c` kiểm tra ở buổi 8, vì đó là quy tắc riêng của Task 2.

---

## 4. Kiến thức nền: static và mảng lớn

### 4.1. Ba vùng nhớ của một chương trình

| Vùng | Chứa gì | Đặc điểm |
|---|---|---|
| **Ngăn xếp (stack)** | Biến cục bộ khai báo bình thường trong hàm | Tự tạo khi vào hàm, tự mất khi ra khỏi hàm. **Nhỏ**: mặc định khoảng 1 MB trên Windows. |
| **Vùng dữ liệu tĩnh** | Biến toàn cục và biến `static` | Tồn tại suốt chương trình, tự khởi tạo bằng 0. Không bị giới hạn 1 MB. |
| **Heap** | Vùng nhớ xin bằng `malloc` | Kích thước tùy ý, phải tự `free`. Task 2 không cần. |

### 4.2. Vì sao Task 2 dùng mảng static

Đề yêu cầu xử lý ít nhất 10000 dòng. Mỗi `Sample` dài 32 byte, nên mảng 10000 mẫu dài khoảng 320 KB; mảng `HourRecord` (40 byte mỗi phần tử) khoảng 400 KB. Bốn mảng như vậy cộng lại hơn 1 MB. Nếu khai báo chúng là biến cục bộ trong `main`, chương trình **sập ngay khi chạy** vì tràn ngăn xếp (stack overflow).

Khai báo `static` ở đầu file thì các mảng nằm ở vùng dữ liệu tĩnh, không bị giới hạn đó:

```c
static Sample samples[MAX_SAMPLES];
```

Từ khóa `static` ở phạm vi file còn có nghĩa thứ hai: **biến hoặc hàm chỉ dùng được trong file này**. Vì vậy mọi hàm của `task2.c` cũng khai báo `static`: chúng là chi tiết nội bộ của chương trình, không cần lộ ra ngoài.

### 4.3. Vì sao giới hạn đúng 10000

Đề cho hai lựa chọn khi file có hơn 10000 dòng dữ liệu: xử lý hết, hoặc chỉ xử lý 10000 dòng đầu và ghi `Error 09: input file is too large`. Nhóm chọn cách thứ hai: mảng có kích thước cố định, đơn giản và đúng yêu cầu. Cách thứ nhất cần mảng tự lớn lên bằng `realloc`.

---

## 5. Phần đầu của task2.c: hằng số và mảng

Đầu `task2.c` gồm các nhóm `#define`:

| Nhóm | Ví dụ | Dùng để |
|---|---|---|
| Tên file | `LOG_NAME "task2.log"`, `DEFAULT_LOAD_NAME`, `OUTLIER_NAME`... | Tên file mặc định và các file kết quả theo đề |
| Dòng tiêu đề | `DATA_HEADER "id,time,power,load"`... | Kiểm tra file vào và ghi file ra |
| Kích thước | `DATA_FIELDS 4`, `MAX_ARGUMENTS 3`, `MAX_SAMPLES 10000`, `HOURS_PER_DAY 24` | Không viết cứng số |
| Mã lỗi | `ERR_INPUT_NOT_FOUND 1` ... `ERR_LOAD_NOT_FOUND 10` | Buổi 8 |
| Mô tả lỗi | `MSG_INVALID_LINE "invalid data at line %d"`... | Buổi 8 |
| Kết quả đọc file | `READ_OK`, `READ_NOT_FOUND`, `READ_BAD_FORMAT` | Kết quả của `read_samples` |
| Mã vận hành | `CONDITION_OFF 1` ... `CONDITION_FAILURE 5` | Bảng 1 của đề, buổi 5 |

Tiếp theo là năm mảng `static` (mục 4) và mảng tên điều kiện:

```c
static const char *const condition_names[] = {
  "unknown", "off", "inefficient", "good", "overloaded", "failure"
};
```

Chỉ số của mảng **chính là mã vận hành**: `condition_names[3]` là `"good"`. Đổi mã thành chữ chỉ cần một phép lấy phần tử.

---

## 6. Hàm parse_arguments

```c
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
```

| Lệnh | `argc` | File tải | File dữ liệu | Trả về |
|---|---|---|---|---|
| `power_process` | 1 | `load_specs.csv` | `power_sensor.csv` | 1 |
| `power_process a.csv` | 2 | `a.csv` | `power_sensor.csv` | 1 |
| `power_process a.csv b.csv` | 3 | `a.csv` | `b.csv` | 1 |
| `power_process a.csv b.csv c.csv` | 4 | | | 0 → `Error 05` |

**Vì sao tham số là `const char **`?** Hàm cần sửa hai biến `const char *` của `main` (cho chúng trỏ vào tên file), nên nhận **địa chỉ** của chúng: kiểu `const char **`, và `main` truyền `&load_path`. Đây là đúng quy tắc của buổi 1: muốn hàm sửa biến nào thì truyền địa chỉ biến đó.

**Hàm không chép chuỗi:** `*load_path = argv[1]` chỉ cho con trỏ trỏ vào chuỗi mà hệ điều hành đã cấp trong `argv`. Chuỗi đó tồn tại suốt chương trình nên an toàn.

**Một tham số là file nào?** Đề không nói rõ. Nhóm hiểu theo thứ tự trong cú pháp lệnh `power_process [load_type.csv] [data_filename.csv]`: tham số đầu là file tải. Ghi vào bảng quyết định.

---

## 7. Hàm parse_sample_line

```c
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
```

Hàm biến một dòng `2,2026:11:10 15:16:03,107.8,3` thành một `Sample`. Mỗi `if` kiểm tra một trường bằng các hàm của buổi 1 và 2:

| Trường | Điều kiện hợp lệ | Dòng sai trong đề bị bắt ở đây |
|---|---|---|
| Số trường | Đúng 4 | Dòng thiếu hoặc thừa dấu phẩy |
| `id` | Số nguyên **dương** | `-2,...` (ID âm), `, , ,` (trống) |
| `time` | Đúng mẫu, ngày có thật | `...15:16:,...` (thiếu giây) |
| `power` | Là một số | `...,,3` (công suất trống) |
| `load` | Số nguyên dương | `...,107.8,` (mã tải trống) |

**Hai điểm cần nói được khi vấn đáp:**

1. **Đọc vào biến tạm `result`, cuối cùng mới gán `*sample = result`.** Nếu một trường giữa chừng sai, `*sample` của nơi gọi không bị ghi dở.
2. **Không kiểm tra khoảng của công suất.** Công suất −10.8 hay 3448.8 vẫn là số hợp lệ, nên dòng đó **đọc được**. Nó không phải dòng sai (không ghi `Error 06`) mà là **ngoại lệ** của Task 2.1 (buổi 4). Đề tách rõ hai loại: dòng sai bị bỏ qua và ghi log; ngoại lệ được giữ lại và ghi vào `power_outlier.csv`.

---

## 8. Hàm read_samples

### 8.1. Code

```c
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
```

### 8.2. Ba kết quả

| Kết quả | Khi nào | `main` ghi |
|---|---|---|
| `READ_NOT_FOUND` | Không mở được file | `Error 01: input file not found or not accessible` |
| `READ_BAD_FORMAT` | File rỗng hoặc dòng đầu không phải `id,time,power,load` | `Error 02: invalid input data file` |
| `READ_OK` | Đọc xong; các dòng sai đã được ghi log và bỏ qua | (không ghi) |

### 8.3. Vòng lặp đọc dòng, từng bước

1. **`line_number++`**: dòng tiêu đề là dòng 0, dòng dữ liệu đầu tiên là dòng 1, đúng quy ước số dòng của đề. Số dòng tăng cho **mọi** dòng, kể cả dòng sai hay dòng trống, để số dòng trong log luôn khớp với vị trí thật trong file.
2. **Cắt ký tự xuống dòng**, rồi **bỏ qua dòng trống** mà không báo lỗi.
3. **Quá giới hạn:** dòng dữ liệu thứ 10001 không còn chỗ trong mảng: ghi `Error 09` một lần rồi `break`, dừng đọc. 10000 dòng đầu vẫn được xử lý bình thường.
4. **Dòng sai:** `parse_sample_line` trả 0: ghi `Error 06` kèm số dòng, `continue` sang dòng sau.
5. **Dòng trùng:** cùng ID và cùng thời gian với một mẫu đã nhận: ghi `Error 07` với số dòng của lần đầu và số dòng hiện tại, rồi bỏ qua (giữ dòng đầu, bỏ dòng sau như đề yêu cầu). Chi tiết ở buổi 8.
6. **Dòng tốt:** lưu mẫu vào `items` và số dòng của nó vào `lines` ở **cùng vị trí**, rồi tăng số đếm.

**Vì sao lưu số dòng?** Để báo `Error 07: data at line X and Y are duplicated`, chương trình cần biết dòng đầu tiên (X) nằm ở đâu. Mảng `lines` song song với `items`: `lines[k]` là số dòng của `items[k]`.

**Hai cách đóng file:** ở phần đầu (lỗi tiêu đề) hàm `fclose` rồi `return` ngay; ở cuối hàm có một `fclose`. Mọi đường ra khỏi hàm đều đóng file.

---

## 9. Chạy thử

**Chạy toàn bộ bộ test:** bấm đúp `chay_thu_task2.bat`. Phần 1 chạy ba chương trình kiểm tra `common.c` (buổi 1, 2, 3); phần 2 biên dịch `power_process`; phần 3 chạy 14 trường hợp. Kết quả đúng là mọi dòng `DAT`.

Liên quan tới buổi này:
- `test_buoi3`: 16 trường hợp của `read_load_specs` (file đúng, file rỗng, thiếu tiêu đề, dấu chấm phẩy, thiếu cột, thừa cột, mã tải sai, tên trống, mảng đầy, file không tồn tại...).
- Trường hợp `01_mau_chuan`: chạy không tham số, dùng tên file mặc định.
- Trường hợp `02_ngoai_le`: chạy với một tham số.
- Trường hợp `05_thieu_tieu_de`, `06_khong_co_file_du_lieu`: hai kết quả lỗi của `read_samples`.

**Tự chạy bằng tay** để thấy chương trình làm gì: trong VS Code mở Terminal (`Ctrl + ` `), gõ:

```
cd tests\task2\01_mau_chuan
..\power_process.exe
type load_condition.csv
type task2.log
```

(`power_process.exe` được `chay_thu_task2.bat` tạo ra trong `tests\task2`. Chạy file `.bat` ít nhất một lần trước.)

---

## 10. Câu hỏi tự kiểm tra

**Câu 1.** Vì sao `read_load_specs` không ghi log mà trả về một mã kết quả?

<details><summary>Xem đáp án</summary>

Hàm dùng chung cho Task 1 và Task 2, mà hai task báo lỗi file tải bằng mã khác nhau và ghi vào hai file log khác nhau (`task1.log`, `task2.log`). Trả về mã kết quả để mỗi chương trình tự quyết định ghi lỗi gì.
</details>

**Câu 2.** Vì sao phải `strcpy` tên loại tải vào struct, không được gán `spec.name = fields[1]`?

<details><summary>Xem đáp án</summary>

`name` là một mảng `char` nằm trong struct, không gán mảng bằng `=` được. Hơn nữa `fields[1]` trỏ vào bên trong `line`, mà `line` bị ghi đè ở lần `fgets` sau, nên phải chép nội dung ra.
</details>

**Câu 3.** `(*count)++` và `*count++` khác nhau thế nào?

<details><summary>Xem đáp án</summary>

`(*count)++` tăng giá trị của biến mà `count` trỏ tới. `*count++` được hiểu là `*(count++)`: tăng **con trỏ** `count` (cho nó trỏ sang ô kế tiếp), giá trị cần tăng không đổi.
</details>

**Câu 4.** Vì sao các mảng của `task2.c` phải khai báo `static`?

<details><summary>Xem đáp án</summary>

Mỗi mảng chứa tới 10000 phần tử, tổng cộng hơn 1 MB. Biến cục bộ bình thường nằm trên ngăn xếp, mặc định chỉ khoảng 1 MB trên Windows, nên chương trình sẽ sập vì tràn ngăn xếp. Biến `static` nằm ở vùng dữ liệu tĩnh, tồn tại suốt chương trình và không bị giới hạn đó.
</details>

**Câu 5.** Dòng `2,2026:11:10 15:16:03,-10.8,5` bị `parse_sample_line` xử lý thế nào? Nó đi vào file nào?

<details><summary>Xem đáp án</summary>

Trả 1: mọi trường đều đọc được, −10.8 là một số hợp lệ. Dòng được lưu vào mảng `samples`. Ở Task 2.1, công suất −10.8 nằm ngoài dải đo nên dòng được ghi vào `power_outlier.csv`. Không có dòng log nào.
</details>

**Câu 6.** File dữ liệu có dòng trống ở dòng 3. Dòng dữ liệu kế tiếp bị sai: log ghi số dòng mấy?

<details><summary>Xem đáp án</summary>

`Error 06: invalid data at line 4`. Số dòng tăng cho mọi dòng, kể cả dòng trống, nên số trong log khớp với vị trí thật của dòng trong file (dòng tiêu đề là 0).
</details>

**Câu 7.** Vì sao `read_samples` lưu số dòng của từng mẫu vào mảng `lines`?

<details><summary>Xem đáp án</summary>

Để báo dòng trùng theo đúng mẫu `Error 07: data at line X and Y are duplicated`: khi gặp dòng Y trùng, chương trình cần biết dòng X của mẫu đã nhận trước đó.
</details>

**Câu 8.** Vì sao `parse_arguments` nhận `const char **load_path`?

<details><summary>Xem đáp án</summary>

Hàm cần sửa biến `const char *load_path` của `main`, cho nó trỏ vào tên file. Muốn hàm sửa biến nào thì truyền địa chỉ biến đó; địa chỉ của một `const char *` có kiểu `const char **`.
</details>
