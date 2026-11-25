# Buổi 8 – Chín loại lỗi của Task 2 và bộ test tự động

Đề liệt kê 9 loại lỗi cho Task 2 (Error 01 đến 09), và cho phép nhóm đề xuất thêm. Buổi này tổng hợp: mỗi lỗi xảy ra khi nào, được phát hiện ở đâu trong code, chương trình làm gì sau đó, và bộ test kiểm tra từng lỗi ra sao.

Code liên quan trong `task2.c`: các `#define` mã lỗi, `log_line_error`, `find_duplicate`, `check_load_specs`, `all_loads_known`, cùng các khối báo lỗi trong `read_samples` và `main`.

> Mở file này trong VS Code rồi bấm `Ctrl + Shift + V` để xem dạng đã định dạng.

---

## Mục lục

1. [Bảng tổng hợp 10 mã lỗi](#1-bảng-tổng-hợp-10-mã-lỗi)
2. [Mã lỗi và mô tả được khai báo thế nào](#2-mã-lỗi-và-mô-tả-được-khai-báo-thế-nào)
3. [Lỗi có số dòng: hàm log_line_error](#3-lỗi-có-số-dòng-hàm-log_line_error)
4. [Error 07: dòng trùng và hàm find_duplicate](#4-error-07-dòng-trùng-và-hàm-find_duplicate)
5. [Error 04: hai tình huống, hai hàm](#5-error-04-hai-tình-huống-hai-hàm)
6. [Error 08 và Error 09](#6-error-08-và-error-09)
7. [Error 10: lỗi nhóm tự đề xuất](#7-error-10-lỗi-nhóm-tự-đề-xuất)
8. [Bảng quyết định cho báo cáo](#8-bảng-quyết-định-cho-báo-cáo)
9. [Bộ test tự động](#9-bộ-test-tự-động)
10. [Câu hỏi tự kiểm tra](#10-câu-hỏi-tự-kiểm-tra)

---

## 1. Bảng tổng hợp 10 mã lỗi

| Mã | Khi nào | Phát hiện ở | Dòng ghi vào task2.log | Sau đó | Test |
|---|---|---|---|---|---|
| 01 | Không mở được file dữ liệu | `read_samples` trả `READ_NOT_FOUND` | `Error 01: input file not found or not accessible` | Dừng | `06` |
| 02 | File dữ liệu rỗng hoặc dòng đầu không phải `id,time,power,load` | `read_samples` trả `READ_BAD_FORMAT` | `Error 02: invalid input data file` | Dừng | `05` |
| 03 | File tải rỗng, sai tiêu đề, sai số cột, giá trị không phải số | `read_load_specs` trả `LOAD_INVALID` | `Error 03: invalid input load file` | Dừng | `07` |
| 04 | File tải đúng định dạng nhưng thông số vô lý | `check_load_specs` | `Error 04: invalid input load file` | Dừng | `08` |
| 04 | Dữ liệu có mã tải không có trong file tải | `all_loads_known` | `Error 04: invalid input load file` | Chạy tiếp | `09` |
| 05 | Quá 2 tham số | `parse_arguments` | `Error 05: invalid command` | Dừng | `10` |
| 06 | Một dòng dữ liệu không đọc được | `parse_sample_line` trong `read_samples` | `Error 06: invalid data at line X` | Bỏ dòng, chạy tiếp | `03` |
| 07 | Một dòng trùng ID và thời gian với dòng trước | `find_duplicate` trong `read_samples` | `Error 07: data at line X and Y are duplicated` | Giữ X, bỏ Y, chạy tiếp | `04` |
| 08 | Không ghi được một file kết quả | `filter_outliers`, `write_load_condition`, `summarize_by_load` trả 0 | `Error 08: cannot override output file` | Dừng | `12` |
| 09 | File có hơn 10000 dòng dữ liệu | `read_samples` | `Error 09: input file is too large` | Xử lý 10000 dòng đầu | `13` |
| 10 | Không mở được file tải (tự đề xuất) | `read_load_specs` trả `LOAD_NOT_FOUND` | `Error 10: load file not found or not accessible` | Dừng | `11` |

Cột "Test" là tên thư mục trong `tests\task2` kiểm tra lỗi đó (mục 9).

**Nguyên tắc chung:** lỗi làm chương trình **không thể làm tiếp** thì dừng; lỗi chỉ ảnh hưởng **một phần dữ liệu** thì ghi log và chạy tiếp với phần còn lại. Mọi lỗi chỉ ghi vào log, **không in gì ra màn hình**, đúng yêu cầu của đề.

---

## 2. Mã lỗi và mô tả được khai báo thế nào

```c
#define ERR_INPUT_NOT_FOUND 1
#define ERR_INVALID_DATA_FILE 2
...
#define ERR_LOAD_NOT_FOUND 10

#define MSG_INPUT_NOT_FOUND "input file not found or not accessible"
#define MSG_INVALID_LINE "invalid data at line %d"
#define MSG_DUPLICATED "data at line %d and %d are duplicated"
...
```

**Vì sao đặt tên cho cả mã lẫn mô tả:**
- Không viết cứng số và chuỗi giữa code (đề chấm phong cách).
- Đọc `log_error (LOG_NAME, ERR_INVALID_COMMAND, MSG_INVALID_COMMAND)` hiểu ngay; `log_error ("task2.log", 5, "invalid command")` thì phải tra đề mới biết 5 là gì.
- Mọi thông báo nằm ở một chỗ, dễ đối chiếu với đề.

**Error 03 và 04 dùng chung mô tả** `invalid input load file`, vì đề ghi như vậy cho cả hai. Mã khác nhau đủ để phân biệt. Đây là một câu hỏi đã gửi thầy (bảng quyết định, mục 8).

Mô tả viết bằng tiếng Anh, đúng yêu cầu "DESCRIPTION là mô tả chi tiết về lỗi, viết bằng tiếng Anh".

---

## 3. Lỗi có số dòng: hàm log_line_error

Error 06 và 07 phải kèm số dòng. Mô tả của chúng chứa `%d`, là **chuỗi định dạng** cho `printf`:

```c
static void
log_line_error (int code, const char *format, int first, int second)
{
  char message[MESSAGE_SIZE];

  snprintf (message, sizeof message, format, first, second);
  log_error (LOG_NAME, code, message);
}
```

| Phần | Giải thích |
|---|---|
| `format` | Một trong hai chuỗi `MSG_INVALID_LINE` (một `%d`) hoặc `MSG_DUPLICATED` (hai `%d`). |
| `snprintf (message, sizeof message, format, first, second)` | Điền số dòng vào chỗ `%d`, ghi kết quả vào `message`, không bao giờ vượt `MESSAGE_SIZE` (80) ký tự. |
| Truyền `second` khi format chỉ có một `%d` | An toàn: `printf` và họ hàng của nó bỏ qua các tham số thừa. Nơi gọi Error 06 truyền 0. |
| `log_error (LOG_NAME, code, message)` | Ghi dòng `Error AB: ...` như mọi lỗi khác. |

Ví dụ: `log_line_error (ERR_DUPLICATED, MSG_DUPLICATED, 2, 5)` ghi `Error 07: data at line 2 and 5 are duplicated`.

**Vì sao tách thành hàm:** Error 06 và 07 đều cần ghép số vào chuỗi rồi ghi log. Viết một hàm thay vì lặp đoạn `snprintf` ở hai chỗ.

### Quy ước số dòng

| File | Dòng 0 | Dòng 1 |
|---|---|---|
| File dữ liệu | Dòng tiêu đề `id,time,power,load` | Dòng dữ liệu đầu tiên |

Đề viết rõ: "Dòng đầu tiên trong tệp đầu vào, cũng là dòng tiêu đề, là dòng 0". Trong VS Code, số dòng hiển thị bắt đầu từ 1, nên **dòng X trong log nằm ở dòng X + 1 của VS Code**.

### Các kiểu dòng sai (Error 06)

| Dòng | Lý do sai | Trong đề? |
|---|---|---|
| `, , ,` | Mọi trường trống | Có |
| `-2,2026:11:10 15:16:03,107.8,3` | ID âm | Có |
| `2,2026:11:10 15:16:,107.8,3` | Thời gian thiếu giây | Có |
| `2,2026:11:10 15:16:03,,3` | Công suất trống | Có |
| `2,2026:11:10 15:16:03,107.8,` | Mã tải trống | Có |
| `1,2026:11:10 11:00:00,50.0,3,9` | Thừa một cột | Nhóm thêm |
| `1,2026:13:10 11:00:00,50.0,3` | Tháng 13 | Nhóm thêm |

Trường hợp `03_dong_sai` chứa đủ 7 kiểu này.

---

## 4. Error 07: dòng trùng và hàm find_duplicate

### 4.1. Đề yêu cầu

> Nếu tệp đầu vào chứa dữ liệu trùng lặp, tức là hai hoặc nhiều dòng trong tệp giống hệt nhau **hoặc có cùng ID và thời gian**, thông báo lỗi phải bao gồm số dòng [...] `Error 07: data at line X and Y are duplicated`, X là dòng xuất hiện lần đầu và Y là dòng giống với X. Chương trình nên xử lý dòng X như bình thường và bỏ qua dòng Y.

Hai dòng giống hệt nhau thì chắc chắn cùng ID và thời gian, nên chỉ cần **một** phép kiểm tra: cùng ID và cùng thời gian. Một cảm biến không thể đo hai giá trị khác nhau tại cùng một thời điểm.

### 4.2. Code

```c
static int
find_duplicate (const Sample items[], int count, const Sample *candidate)
{
  int i;

  for (i = 0; i < count; i++)
    if (items[i].id == candidate->id && items[i].time == candidate->time)
      return i;
  return -1;
}
```

Và trong `read_samples`:

```c
first = find_duplicate (items, *count, &sample);
if (first >= 0)
  {
    log_line_error (ERR_DUPLICATED, MSG_DUPLICATED, lines[first],
                    line_number);
    continue;
  }
```

| Điểm | Giải thích |
|---|---|
| So sánh trên `Sample` đã đọc | `time` là `time_t` (số giây) nên so bằng `==` chính xác. So trên số nhanh và chắc hơn so chuỗi. |
| Chỉ tìm trong các mẫu **đã nhận** | `items` chỉ chứa các dòng hợp lệ và không trùng. Dòng sai không bao giờ được so. |
| Trả về **vị trí** của mẫu trùng | Để lấy số dòng của nó từ mảng `lines` song song: `lines[first]` là X. |
| `continue` | Bỏ qua dòng Y: không lưu vào mảng. Dòng X giữ nguyên, được xử lý bình thường. |

### 4.3. Chi phí

Mỗi dòng mới được so với mọi mẫu đã nhận. Với n dòng, tổng số phép so khoảng n × n / 2. Với 10000 dòng là khoảng 50 triệu phép so sánh hai số nguyên: máy tính làm trong khoảng một phần mười giây. Đủ nhanh cho giới hạn của đề. Nếu bị hỏi "có cách nhanh hơn không?": có thể sắp xếp trước rồi chỉ so các phần tử kề nhau, nhưng khi đó phải giữ lại số dòng gốc và thứ tự "dòng nào xuất hiện trước", phức tạp hơn mà không cần thiết ở đây.

### 4.4. Ví dụ (trường hợp `04_trung_lap`)

| Dòng | Nội dung | Kết quả |
|---|---|---|
| 1 | `1,2026:11:10 08:00:00,100.0,3` | Nhận |
| 2 | `2,2026:11:10 08:00:00,10.0,5` | Nhận |
| 5 | `2,2026:11:10 08:00:00,10.0,5` (giống hệt dòng 2) | `Error 07: data at line 2 and 5 are duplicated`, bỏ |
| 9 | `1,2026:11:10 08:00:00,999.0,3` (cùng ID, thời gian với dòng 1, khác công suất) | `Error 07: data at line 1 and 9 are duplicated`, bỏ |

Giá trị 999.0 của dòng 9 bị bỏ qua nên không làm sai trung bình giờ: `load_condition.csv` của trường hợp này giống hệt `01_mau_chuan`.

---

## 5. Error 04: hai tình huống, hai hàm

Đề mô tả Error 04: "tệp thông số tải có định dạng phù hợp nhưng thông tin không tương thích với tệp dữ liệu và thông số kỹ thuật của công tơ, ví dụ giá trị không chính xác về công suất tối thiểu hoặc tối đa, công suất khi xảy ra sự cố không chính xác, **hoặc** mã tải hoàn toàn khác với mã trong tệp dữ liệu". Có hai tình huống khác nhau.

### 5.1. Thông số tải vô lý: check_load_specs

```c
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
```

Mỗi loại tải phải thỏa: **0 ≤ min ≤ max < failure ≤ 3000**, và không có hai loại tải trùng mã.

| Điều kiện | Vì sao |
|---|---|
| min ≥ 0 | Công suất không âm |
| min ≤ max | Khoảng hoạt động bình thường phải có nghĩa |
| max < failure | Công suất sự cố phải cao hơn mức tối đa bình thường; nếu không, Bảng 1 không có vùng `overloaded` |
| failure ≤ 3000 | Đồng hồ chỉ đo tới 3000 W, không thể phát hiện sự cố trên mức đó: "không tương thích với thông số kỹ thuật của công tơ" |
| Mã không trùng | Hai loại tải cùng mã thì không biết cảm biến thuộc loại nào |

Vòng `for (j = 0; j < i; j++)` so loại tải thứ `i` với mọi loại tải đứng trước nó. Như vậy mọi cặp được so đúng một lần.

**Sau đó:** dừng chương trình. Không có thông số đúng thì không phân loại được điều kiện tải.

### 5.2. Mã tải lạ trong dữ liệu: all_loads_known

```c
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
```

Gọi sau khi đọc xong file dữ liệu. Có một mẫu mang mã tải không tồn tại là ghi `Error 04` **một lần** cho cả file.

**Sau đó:** chạy tiếp. Các cảm biến có mã tải đúng vẫn được xử lý bình thường. Cảm biến có mã tải lạ:
- vẫn có trong `power_valid.csv` (hoặc `power_outlier.csv`), vì giá trị đo của nó không sai;
- có điều kiện `unknown` trong `load_condition.csv`, vì không có min, max, failure để so;
- không xuất hiện trong `power_summary.csv`, vì không có loại tải để cộng vào.

Trường hợp `09_ma_tai_la`: cảm biến 3 mang mã tải 9, cho ra dòng `3,2026:11:10 08:00:00,60.0,60.0,9,unknown`.

---

## 6. Error 08 và Error 09

### 6.1. Error 08: không ghi được file kết quả

Đề: "Tệp đầu ra đã tồn tại và là tệp chỉ đọc". Khi đó `fopen (..., "w")` trả `NULL`. Ba hàm ghi file (`filter_outliers`, `write_load_condition`, `summarize_by_load`) trả 0, và `main` ghi `Error 08: cannot override output file` rồi dừng.

**Thử bằng tay:** trong File Explorer, bấm chuột phải vào một file kết quả, chọn Properties, tích **Read-only**, rồi chạy lại chương trình. Nhớ bỏ tích sau khi thử. Trong Command Prompt: `attrib +R power_valid.csv` và `attrib -R power_valid.csv`.

### 6.2. Error 09: file quá lớn

Đề cho hai lựa chọn khi file có hơn 10000 dòng dữ liệu: xử lý hết, hoặc chỉ xử lý 10000 dòng đầu và ghi `Error 09`. Nhóm chọn cách thứ hai (buổi 3, mục 4).

```c
if (line_number > max_items)
  {
    log_error (LOG_NAME, ERR_TOO_LARGE, MSG_TOO_LARGE);
    break;
  }
```

| Điểm | Giải thích |
|---|---|
| So `line_number`, không so số mẫu đã nhận | "10000 dòng đầu" là 10000 dòng của **file**, kể cả dòng sai và dòng trùng. |
| Đặt **sau** bước bỏ qua dòng trống | Một dòng trống thừa ở cuối file đúng 10000 dòng không làm báo lỗi nhầm. |
| `break` | Dừng đọc, ghi lỗi đúng một lần. 10000 dòng đã đọc vẫn được xử lý đầy đủ ở 2.1, 2.2, 2.3. |

Trường hợp `13_qua_lon` có 10005 dòng dữ liệu: `power_valid.csv` có đúng 10000 dòng.

---

## 7. Error 10: lỗi nhóm tự đề xuất

Đề liệt kê lỗi cho file **dữ liệu** không mở được (Error 01) nhưng không có mã cho file **tải** không mở được. Nếu dùng chung Error 01 thì người đọc log không biết file nào bị thiếu. Nhóm thêm:

```
Error 10: load file not found or not accessible
```

Đề cho phép: "Sinh viên có thể đề xuất thêm các lỗi khác có thể xảy ra nhưng chưa được liệt kê ở trên. Những lỗi đó cần được lưu vào tệp log tương ứng và **mô tả trong báo cáo**." Phải nhớ mô tả lỗi này trong báo cáo.

**Thứ tự kiểm tra trong `main`:** file tải được đọc trước file dữ liệu, vì phải có thông số tải mới kiểm tra được mã tải trong dữ liệu.

---

## 8. Bảng quyết định cho báo cáo

Các điểm đề không nói rõ và lựa chọn của nhóm trong Task 2. Đưa bảng này vào báo cáo, cập nhật khi thầy trả lời:

| # | Điểm chưa rõ | Lựa chọn của nhóm | Ở đâu trong code |
|---|---|---|---|
| 1 | Error 03 và 04 cùng một mô tả | Giữ nguyên mô tả của đề, chỉ khác mã | `MSG_INVALID_LOAD` |
| 2 | Chỉ có một tham số: là file tải hay file dữ liệu | File tải, theo thứ tự trong cú pháp lệnh | `parse_arguments` |
| 3 | Không mở được file tải | Lỗi tự đề xuất Error 10 | `main` |
| 4 | Thông số tải thế nào là "không tương thích" | 0 ≤ min ≤ max < failure ≤ 3000, không trùng mã | `check_load_specs` |
| 5 | Dữ liệu có mã tải lạ: dừng hay chạy tiếp | Error 04 một lần, chạy tiếp; bản ghi có điều kiện `unknown`, không vào 2.3 | `all_loads_known` |
| 6 | Dòng trống trong file dữ liệu | Bỏ qua, không báo lỗi; vẫn tính số dòng để số dòng khớp với file | `read_samples` |
| 7 | File quá 10000 dòng | Chỉ xử lý 10000 dòng đầu, Error 09 | `read_samples` |
| 8 | Log ghi đè hay ghi nối giữa các lần chạy | Xóa log ở đầu mỗi lần chạy | `clear_log` |
| 9 | Công suất tải tắt in `0` hay `0.0` | `0.0`, vì đề yêu cầu 1 chữ số thập phân | `%.1f` |
| 10 | Năng lượng và điều kiện tải tính từ trung bình làm tròn hay không | Từ giá trị chưa làm tròn; chỉ làm tròn khi in | `compute_hourly` |
| 11 | Task 2.3 khi mô phỏng nhiều ngày | Cộng mọi ngày vào cùng giờ trong ngày | `summarize_by_load` |
| 12 | Loại tải không có cảm biến nào trong 2.3 | Không in | `summarize_by_load` |
| 13 | Dòng đầu file ngoại lệ tiếng Việt hay tiếng Anh | `number of outliers: X` như ví dụ | `filter_outliers` |

---

## 9. Bộ test tự động

### 9.1. Cấu trúc

```
tests\task2\
├── 01_mau_chuan\
│   ├── load_specs.csv          file vào
│   ├── power_sensor.csv        file vào
│   ├── lenh.txt                tham số dòng lệnh (trống: không tham số)
│   ├── mo_ta.txt               mô tả trường hợp
│   └── mong_doi\               kết quả mong đợi
│       ├── task2.log
│       ├── power_outlier.csv
│       ├── power_valid.csv
│       ├── load_condition.csv
│       └── power_summary.csv
├── 02_ngoai_le\
...
└── 14_du_lieu_that\
```

| Thư mục | Kiểm tra |
|---|---|
| `01_mau_chuan` | File mẫu tính tay được, không tham số (tên mặc định) |
| `02_ngoai_le` | 3 ngoại lệ; một tham số |
| `03_dong_sai` | 7 kiểu dòng sai: Error 06 |
| `04_trung_lap` | Dòng giống hệt và dòng cùng ID, thời gian: Error 07 |
| `05_thieu_tieu_de` | Error 02 |
| `06_khong_co_file_du_lieu` | Error 01 |
| `07_file_tai_sai_dinh_dang` | Error 03 |
| `08_file_tai_vo_ly` | Error 04, dừng |
| `09_ma_tai_la` | Error 04, chạy tiếp, điều kiện `unknown` |
| `10_qua_nhieu_tham_so` | Error 05 |
| `11_khong_co_file_tai` | Error 10 |
| `12_khong_ghi_duoc` | Error 08 (file `.bat` đặt `power_valid.csv` thành chỉ đọc trước khi chạy) |
| `13_qua_lon` | 10005 dòng: Error 09, xử lý 10000 dòng |
| `14_du_lieu_that` | 4 cảm biến, 10 giờ, có lúc tắt và một lần sự cố |

### 9.2. Kết quả mong đợi lấy từ đâu

- `01_mau_chuan`: tính tay (bảng chạy tay ở buổi 6).
- Các trường hợp còn lại: từ một **bản tính viết riêng bằng Python**, viết từ đề bài chứ không dịch từ code C. Hai bản độc lập cho cùng kết quả thì khả năng cả hai cùng sai một chỗ là rất thấp. Lấy chính kết quả của chương trình làm đáp án thì không phát hiện được gì.

### 9.3. chay_thu_task2.bat làm gì

1. Biên dịch và chạy `test_buoi1`, `test_buoi2`, `test_buoi3`, in số trường hợp sai của mỗi chương trình.
2. Biên dịch `power_process.exe` vào `tests\task2`.
3. Với mỗi thư mục trường hợp:
   - xóa các file kết quả cũ;
   - đọc tham số từ `lenh.txt`;
   - nếu có `chi_doc.txt`, tạo file có tên ghi trong đó và đặt chế độ chỉ đọc;
   - chạy `power_process.exe` với các tham số đó;
   - so từng file trong `mong_doi` với file cùng tên vừa tạo bằng lệnh `fc` (so nội dung hai file văn bản);
   - in `DAT` nếu mọi file giống, `SAI` kèm tên file khác nếu không.
4. In tổng số `DAT` và `SAI`, lưu toàn bộ vào `ket_qua_task2.txt`.

Kết quả hiện tại trên máy bạn: **14 DAT, 0 SAI**. Tôi đã lấy một số file kết quả thật từ máy bạn về so lại từng byte với file mong đợi, để chắc lệnh `fc` không báo đạt nhầm.

### 9.4. Khi sửa code

Sau **mỗi** lần sửa `task2.c` hay `common.c`, chạy lại `chay_thu_task2.bat`. Nếu một trường hợp chuyển từ `DAT` sang `SAI`, lần sửa vừa rồi đã làm hỏng thứ gì đó. Mở file trong `mong_doi` và file vừa tạo của trường hợp đó, so bằng mắt (VS Code: chọn hai file, chuột phải, **Compare Selected**) để thấy khác ở đâu.

### 9.5. Tự thêm một trường hợp

1. Tạo thư mục mới trong `tests\task2`, ví dụ `15_cua_toi`.
2. Đặt file vào (`load_specs.csv` và file dữ liệu), viết `lenh.txt` và `mo_ta.txt`.
3. Tạo thư mục `mong_doi` và tự viết các file kết quả bạn **mong đợi** (tính tay).
4. Chạy `chay_thu_task2.bat`.

Tự tính tay một trường hợp nhỏ rồi xem chương trình có ra đúng không là cách tốt nhất để hiểu sâu và để chuẩn bị vấn đáp.

---

## 10. Câu hỏi tự kiểm tra

**Câu 1.** Dòng `2,2026:11:10 15:16:03,,3` nằm ở dòng 7 của VS Code. Log ghi gì?

<details><summary>Xem đáp án</summary>

`Error 06: invalid data at line 6`. Dòng tiêu đề là dòng 0 theo đề, trong khi VS Code đánh số từ 1, nên dòng 7 của VS Code là dòng 6 theo đề.
</details>

**Câu 2.** Hai dòng cùng ID, cùng thời gian nhưng khác công suất có bị coi là trùng không? Chương trình giữ dòng nào?

<details><summary>Xem đáp án</summary>

Có, đề nói trùng là "giống hệt nhau hoặc có cùng ID và thời gian". Chương trình giữ dòng xuất hiện trước (X), bỏ dòng sau (Y), ghi `Error 07: data at line X and Y are duplicated`.
</details>

**Câu 3.** Vì sao `find_duplicate` chỉ tìm trong các mẫu đã nhận, không so với mọi dòng đã đọc?

<details><summary>Xem đáp án</summary>

Dòng sai không đọc được nên không có ID và thời gian để so; dòng trùng đã bị bỏ. Mảng các mẫu đã nhận chứa đúng những dòng có thể là "dòng X" của một lần trùng sau này.
</details>

**Câu 4.** Error 04 có hai tình huống. Tình huống nào làm chương trình dừng, tình huống nào chạy tiếp, và vì sao?

<details><summary>Xem đáp án</summary>

Thông số tải vô lý (ví dụ min lớn hơn max): dừng, vì không có thông số đúng thì không phân loại được điều kiện tải cho ai cả. Dữ liệu có mã tải lạ: chạy tiếp, vì chỉ ảnh hưởng các cảm biến mang mã đó; các cảm biến khác vẫn xử lý được.
</details>

**Câu 5.** Vì sao một loại tải có failure bằng 5000 bị coi là không tương thích?

<details><summary>Xem đáp án</summary>

Đồng hồ chỉ đo được tới 3000 W, nên không bao giờ ghi nhận được công suất đạt mức sự cố 5000 W: thông số tải không tương thích với thông số kỹ thuật của công tơ.
</details>

**Câu 6.** File có đúng 10000 dòng dữ liệu và một dòng trống ở cuối. Có `Error 09` không?

<details><summary>Xem đáp án</summary>

Không. Dòng trống được bỏ qua trước bước kiểm tra giới hạn, nên chỉ có dòng dữ liệu thật thứ 10001 mới làm báo lỗi.
</details>

**Câu 7.** Error 10 là gì, và vì sao nhóm thêm nó?

<details><summary>Xem đáp án</summary>

`Error 10: load file not found or not accessible`, khi không mở được file thông số tải. Đề chỉ có mã cho file dữ liệu không mở được (Error 01); dùng chung mã thì không biết file nào bị thiếu. Đề cho phép đề xuất thêm lỗi, với điều kiện mô tả trong báo cáo.
</details>

**Câu 8.** Kết quả mong đợi của bộ test được tạo thế nào? Vì sao không lấy luôn kết quả của chương trình làm đáp án?

<details><summary>Xem đáp án</summary>

Trường hợp mẫu được tính tay; các trường hợp khác lấy từ một bản tính viết riêng bằng Python từ đề bài. Nếu lấy kết quả của chính chương trình làm đáp án thì test luôn đạt, kể cả khi chương trình sai, nên không kiểm tra được gì.
</details>

**Câu 9.** `log_line_error` được gọi cho Error 06 với tham số `second` bằng 0, trong khi chuỗi chỉ có một `%d`. Có sao không?

<details><summary>Xem đáp án</summary>

Không. `snprintf` chỉ dùng số tham số mà chuỗi định dạng cần; tham số thừa bị bỏ qua.
</details>

**Câu 10.** Vì sao mọi lỗi chỉ ghi vào file log mà không in ra màn hình?

<details><summary>Xem đáp án</summary>

Đề yêu cầu: "Chương trình cần lưu trữ mọi lỗi phát sinh trong quá trình thực thi vào các tệp log, KHÔNG in ra màn hình", mỗi task một file log riêng (`task2.log` cho Task 2).
</details>
