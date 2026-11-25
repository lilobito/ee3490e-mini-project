# Buổi 6 – Task 2.2 phần 2: trung bình giờ, năng lượng tích lũy, ghi file

Buổi này ghép ba công cụ của buổi 5 thành hàm chính của Task 2.2: `compute_hourly`, rồi ghi kết quả bằng `write_load_condition`. `compute_hourly` cũng là hàm nên vẽ **lưu đồ thuật toán** cho báo cáo (đề yêu cầu ít nhất một lưu đồ cho mỗi task).

> Mở file này trong VS Code rồi bấm `Ctrl + Shift + V` để xem dạng đã định dạng.

---

## Mục lục

1. [Ý tưởng: gom nhóm bằng hai chỉ số](#1-ý-tưởng-gom-nhóm-bằng-hai-chỉ-số)
2. [Năng lượng tích lũy là gì](#2-năng-lượng-tích-lũy-là-gì)
3. [Hàm compute_hourly](#3-hàm-compute_hourly)
4. [Chạy tay trên file mẫu](#4-chạy-tay-trên-file-mẫu)
5. [Hàm write_load_condition](#5-hàm-write_load_condition)
6. [Lưu đồ compute_hourly cho báo cáo](#6-lưu-đồ-compute_hourly-cho-báo-cáo)
7. [Những lựa chọn của nhóm cần ghi vào báo cáo](#7-những-lựa-chọn-của-nhóm-cần-ghi-vào-báo-cáo)
8. [Chạy thử](#8-chạy-thử)
9. [Câu hỏi tự kiểm tra](#9-câu-hỏi-tự-kiểm-tra)

---

## 1. Ý tưởng: gom nhóm bằng hai chỉ số

Sau khi sắp xếp (buổi 5), các mẫu cùng cảm biến và cùng giờ nằm liền nhau. Gom nhóm bằng hai chỉ số:

- `i` đứng ở **đầu** một nhóm.
- `j` chạy từ `i` tới khi gặp mẫu đầu tiên **không** cùng nhóm.
- Nhóm gồm các mẫu từ vị trí `i` tới `j - 1`, tức có `j - i` mẫu.
- Xử lý xong nhóm thì cho `i` nhảy tới `j`: đó là đầu nhóm kế tiếp.

```
vị trí:   0      1      2      3      4      5      6  ...
        ┌──────┬──────┬──────┬──────┬──────┬──────┬──────┐
        │1 08h │1 08h │1 09h │1 09h │1 10h │1 10h │2 08h │
        └──────┴──────┴──────┴──────┴──────┴──────┴──────┘
lượt 1:   i             j                                  nhóm (1, 08h): 2 mẫu
lượt 2:                 i             j                    nhóm (1, 09h): 2 mẫu
lượt 3:                               i             j      nhóm (1, 10h): 2 mẫu
lượt 4:                                             i  ... nhóm (2, 08h) ...
```

Mỗi mẫu được `j` đi qua đúng một lần, nên toàn bộ việc gom nhóm chỉ là **một lượt duyệt** mảng.

---

## 2. Năng lượng tích lũy là gì

**Năng lượng** = công suất × thời gian. Công suất trung bình của một giờ (đơn vị W) nhân với 1 giờ cho năng lượng tiêu thụ trong giờ đó (đơn vị Wh). Vì mỗi khoảng đúng 1 giờ, năng lượng của giờ đó **bằng đúng con số** công suất trung bình.

**Năng lượng tích lũy** của một cảm biến tại một giờ = tổng năng lượng từ giờ đầu tiên của cảm biến đó tới giờ hiện tại:

| Giờ | Trung bình (W) | Năng lượng tích lũy (Wh) |
|---|---|---|
| 08:00 | 150.0 | 150.0 |
| 09:00 | 40.0 | 150.0 + 40.0 = 190.0 |
| 10:00 | 0.0 | 190.0 + 0.0 = 190.0 |

Năng lượng tính **riêng cho từng cảm biến**, nên khi chuyển sang cảm biến khác phải đặt lại về 0. Vì mảng đã sắp theo ID, mọi giờ của một cảm biến nằm liền nhau, nên chỉ cần một biến `energy` và đặt lại khi ID đổi.

Khi tải tắt (trung bình 0.0), năng lượng tích lũy **giữ nguyên**, không giảm.

---

## 3. Hàm compute_hourly

### 3.1. Code

```c
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
```

### 3.2. Tham số và kết quả

| Tham số | Ý nghĩa |
|---|---|
| `Sample items[]` | Các mẫu **hợp lệ** (mảng `valid` của Task 2.1). Không có `const` vì hàm sắp xếp lại mảng này. |
| `int count` | Số mẫu. |
| `const LoadSpec specs[]`, `int spec_count` | Các loại tải, để tra min, max, failure. |
| `HourRecord records[]` | Mảng ghi kết quả, một phần tử cho mỗi cảm biến trong mỗi giờ. |
| Trả về | Số bản ghi đã tạo. |

Số bản ghi không bao giờ vượt quá số mẫu (mỗi nhóm có ít nhất một mẫu), nên mảng `hours` cùng kích thước `MAX_SAMPLES` là đủ.

### 3.3. Giải thích từng phần

| Phần | Giải thích |
|---|---|
| `record_count = 0` | Số bản ghi đã tạo, cũng là vị trí sẽ ghi bản ghi kế tiếp. |
| `previous_id = 0` | ID của nhóm trước. Bắt đầu bằng 0, mà ID hợp lệ luôn dương, nên nhóm đầu tiên chắc chắn được coi là "cảm biến mới". |
| `energy = 0.0` | Năng lượng tích lũy của cảm biến hiện tại. |
| `qsort (items, ..., compare_samples);` | Sắp theo (ID, thời gian) để các nhóm nằm liền nhau (buổi 5). |
| `while (i < count)` | Mỗi lượt của vòng ngoài xử lý **một nhóm**. |
| `int id = items[i].id; time_t hour = hour_start (items[i].time);` | Khóa của nhóm hiện tại: cảm biến nào, giờ nào. Lấy từ mẫu đầu nhóm. |
| `HourRecord *record = &records[record_count];` | Con trỏ tới chỗ sẽ ghi kết quả của nhóm này, để các dòng sau viết gọn `record->id` thay vì `records[record_count].id`. |
| `if (id != previous_id) { energy = 0.0; previous_id = id; }` | Sang cảm biến mới: đặt lại năng lượng (mục 2). |
| Vòng `while (j < count && ...)` | Đi qua các mẫu của nhóm, cộng dồn công suất vào `sum`. Ba điều kiện: chưa hết mảng; cùng ID; cùng đầu giờ. **`j < count` phải đứng đầu**: nhờ `&&` dừng sớm, khi `j` đã bằng `count` thì không đọc `items[j]` (nằm ngoài phần đã dùng của mảng). |
| `average = sum / (j - i);` | Trung bình = tổng / số mẫu. `sum` là `double` nên phép chia là chia số thực. `j - i` luôn ít nhất bằng 1 (mẫu ở vị trí `i` chắc chắn thuộc nhóm), nên không bao giờ chia cho 0. |
| `energy += average;` | Cộng năng lượng của giờ này vào tổng tích lũy. |
| Sáu dòng `record->...` | Điền bản ghi. `load` lấy từ mẫu đầu nhóm (mọi mẫu của một cảm biến cùng mã tải). `condition` tra Bảng 1 với loại tải tìm được bằng `find_load`. |
| `record_count++; i = j;` | Sang nhóm kế tiếp. |
| `qsort (records, ..., compare_hours);` | Sắp kết quả theo (thời gian, ID) để ghi đúng thứ tự của đề (buổi 5). |

---

## 4. Chạy tay trên file mẫu

File `tests\power_sensor.csv`: cảm biến 1 mang tải 3 (Desktop: min 60, max 700, failure 1000); cảm biến 2 mang tải 5 (Lamp: min 5, max 15, failure 20).

Sau khi sắp xếp, vòng ngoài chạy 6 lượt:

| Lượt | `i` | Nhóm | Các mẫu | `j - i` | Trung bình | `energy` sau | So với Bảng 1 | Điều kiện |
|---|---|---|---|---|---|---|---|---|
| 1 | 0 | ID 1, 08h | 100.0, 200.0 | 2 | 150.0 | 150.0 (ID mới, đặt lại) | 60 ≤ 150 ≤ 700 | good |
| 2 | 2 | ID 1, 09h | 30.0, 50.0 | 2 | 40.0 | 190.0 | 0 < 40 < 60 | inefficient |
| 3 | 4 | ID 1, 10h | 0.0, 0.0 | 2 | 0.0 | 190.0 | P = 0 | off |
| 4 | 6 | ID 2, 08h | 10.0, 12.0 | 2 | 11.0 | 11.0 (ID mới, đặt lại) | 5 ≤ 11 ≤ 15 | good |
| 5 | 8 | ID 2, 09h | 16.0, 18.0 | 2 | 17.0 | 28.0 | 15 < 17 < 20 | overloaded |
| 6 | 10 | ID 2, 10h | 25.0, 15.0 | 2 | 20.0 | 48.0 | 20 ≥ 20 | failure |

Sau lần sắp xếp thứ hai theo (thời gian, ID), file `load_condition.csv`:

```
id,time,power,energy,load,condition
1,2026:11:10 08:00:00,150.0,150.0,3,good
2,2026:11:10 08:00:00,11.0,11.0,5,good
1,2026:11:10 09:00:00,40.0,190.0,3,inefficient
2,2026:11:10 09:00:00,17.0,28.0,5,overloaded
1,2026:11:10 10:00:00,0.0,190.0,3,off
2,2026:11:10 10:00:00,20.0,48.0,5,failure
```

Đây chính là file mong đợi của trường hợp `01_mau_chuan`, và chương trình cho ra giống hệt. File mẫu được thiết kế để có đủ cả 5 điều kiện của Bảng 1 (trừ unknown), cả đúng biên (lượt 6: trung bình bằng đúng failure).

---

## 5. Hàm write_load_condition

```c
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
```

| Phần | Giải thích |
|---|---|
| `fopen (CONDITION_NAME, "w")` | Mở `load_condition.csv` để ghi đè. Không mở được thì trả 0, `main` ghi `Error 08`. |
| `CONDITION_HEADER` | Dòng tiêu đề `id,time,power,energy,load,condition`. |
| `format_time (records[i].time, ...)` | Đầu giờ thành chuỗi. |
| `%.1f` cho công suất và năng lượng | Đúng 1 chữ số thập phân. Tải tắt in là `0.0`. |
| `condition_names[records[i].condition]` | Đổi mã (0 đến 5) thành tên. Mảng tên có chỉ số trùng với mã (buổi 3). |

**File này là đầu vào của Task 3 do A viết.** Sáu tên điều kiện, cách in số và dòng tiêu đề phải khớp từng ký tự với những gì Task 3 đọc. Đây là lý do hai bạn phải chốt sáu tên ngay từ đầu.

---

## 6. Lưu đồ compute_hourly cho báo cáo

Các khối cần vẽ (hình bầu dục: bắt đầu, kết thúc; hình chữ nhật: xử lý; hình thoi: điều kiện):

1. **Bắt đầu**.
2. **Xử lý:** sắp xếp các mẫu theo (ID, thời gian); `i ← 0`; `n ← 0`; `energy ← 0`; `previous_id ← 0`.
3. **Điều kiện:** `i < count`? Sai → tới khối 11.
4. **Xử lý:** `id ← ID của mẫu i`; `hour ← đầu giờ của mẫu i`; `sum ← 0`.
5. **Điều kiện:** `id ≠ previous_id`? Đúng → **Xử lý:** `energy ← 0`, `previous_id ← id`.
6. **Xử lý:** `j ← i`.
7. **Điều kiện:** `j < count` và mẫu `j` cùng ID và cùng đầu giờ? Đúng → **Xử lý:** `sum ← sum + công suất mẫu j`, `j ← j + 1`, quay lại khối 7.
8. **Xử lý:** `average ← sum / (j − i)`; `energy ← energy + average`.
9. **Xử lý:** điều kiện tải ← Bảng 1 (`classify_condition`); lưu bản ghi thứ `n`.
10. **Xử lý:** `n ← n + 1`; `i ← j`; quay lại khối 3.
11. **Xử lý:** sắp xếp các bản ghi theo (thời gian, ID).
12. **Kết thúc**, trả về `n`.

Vẽ bằng draw.io (app.diagrams.net) hoặc Shapes trong Word. Tên biến và tên hàm trong hình phải trùng với code.

---

## 7. Những lựa chọn của nhóm cần ghi vào báo cáo

Đề không nói rõ các điểm sau; nhóm đã chọn và nên ghi vào bảng quyết định:

| Điểm chưa rõ | Lựa chọn | Lý do |
|---|---|---|
| Năng lượng cộng từ trung bình đã làm tròn hay chưa | Cộng từ giá trị **chưa làm tròn**, chỉ làm tròn khi in | Chính xác hơn; hai cách có thể lệch 0.1 ở một vài dòng |
| Điều kiện tải xét theo trung bình đã làm tròn hay chưa | Theo giá trị **chưa làm tròn** | Nhất quán với cách tính năng lượng |
| Năng lượng của mỗi cảm biến tính từ đâu | Từ giờ đầu tiên của cảm biến đó trong dữ liệu hợp lệ | Đúng câu "từ thời điểm đầu đến thời điểm hiện tại" |
| Giờ không có mẫu hợp lệ nào | Không có dòng cho giờ đó | Không có số liệu để tính trung bình |
| Thứ tự dòng trong file | Theo thời gian, rồi theo ID | Giống ví dụ của đề |

---

## 8. Chạy thử

Bấm đúp `chay_thu_task2.bat`. Liên quan tới Task 2.2:

| Trường hợp | Kiểm tra |
|---|---|
| `01_mau_chuan` | 6 bản ghi, đủ 5 điều kiện, như bảng chạy tay ở mục 4 |
| `02_ngoai_le`, `03_dong_sai`, `04_trung_lap` | Ngoại lệ, dòng sai, dòng trùng **không** làm thay đổi trung bình: `load_condition.csv` giống hệt `01_mau_chuan` |
| `09_ma_tai_la` | Cảm biến 3 mang mã tải 9 không có trong file tải: điều kiện `unknown` |
| `14_du_lieu_that` | 4 cảm biến, 10 giờ: 44 bản ghi. Cảm biến 4 bị lỗi lúc 05:00, nên giờ đó trung bình chỉ 1.1 (`inefficient`), các giờ sau đều `off` và năng lượng đứng yên |

Kết quả của `14_du_lieu_that` được đối chiếu với một bản tính viết riêng bằng Python từ đề bài, không dựa vào code C. Hai bản cho kết quả giống hệt nhau.

---

## 9. Câu hỏi tự kiểm tra

**Câu 1.** Vì sao việc gom nhóm chỉ cần một lượt duyệt mảng?

<details><summary>Xem đáp án</summary>

Mảng đã được sắp theo (ID, thời gian), nên mỗi nhóm cùng cảm biến và cùng giờ là một đoạn liên tiếp. Chỉ số `j` đi từ đầu tới cuối mảng đúng một lần, mỗi mẫu được cộng vào đúng nhóm của nó.
</details>

**Câu 2.** Trong vòng `while (j < count && items[j].id == id && ...)`, vì sao `j < count` phải đứng đầu?

<details><summary>Xem đáp án</summary>

`&&` kiểm tra từ trái sang phải và dừng ngay khi gặp vế sai. Khi `j` đã bằng `count`, vế đầu sai nên `items[j]` không bị đọc. Nếu đảo thứ tự, chương trình sẽ đọc phần tử ngoài phần mảng đã dùng.
</details>

**Câu 3.** Có khi nào `average = sum / (j - i)` chia cho 0 không?

<details><summary>Xem đáp án</summary>

Không. Mẫu ở vị trí `i` luôn thuộc nhóm của chính nó (cùng ID, cùng đầu giờ với chính nó), nên vòng trong chạy ít nhất một lần và `j - i` ít nhất bằng 1.
</details>

**Câu 4.** Vì sao tổng các công suất trung bình giờ lại là năng lượng?

<details><summary>Xem đáp án</summary>

Năng lượng bằng công suất nhân thời gian. Công suất trung bình của một giờ (W) nhân 1 giờ cho năng lượng của giờ đó (Wh), có cùng con số. Cộng dồn qua các giờ cho năng lượng tích lũy.
</details>

**Câu 5.** Khi nào biến `energy` được đặt lại về 0, và vì sao?

<details><summary>Xem đáp án</summary>

Khi bắt đầu nhóm của một cảm biến mới (`id != previous_id`). Năng lượng tích lũy tính riêng cho từng cảm biến. Mảng sắp theo ID nên mọi giờ của một cảm biến nằm liền nhau, chỉ cần đặt lại khi ID đổi.
</details>

**Câu 6.** Cảm biến tắt cả giờ 10 (mọi mẫu bằng 0). Dòng của giờ đó có điều kiện gì và năng lượng tích lũy thay đổi thế nào?

<details><summary>Xem đáp án</summary>

Trung bình 0.0, điều kiện `off`. Năng lượng tích lũy giữ nguyên bằng giá trị của giờ trước, vì cộng thêm 0.
</details>

**Câu 7.** Vì sao `compute_hourly` nhận `Sample items[]` không có `const`?

<details><summary>Xem đáp án</summary>

Hàm gọi `qsort` để sắp xếp lại chính mảng này, tức là có sửa nội dung mảng.
</details>

**Câu 8.** Vì sao file `load_condition.csv` phải khớp từng ký tự với những gì Task 3 đọc?

<details><summary>Xem đáp án</summary>

Task 3 của A đọc file này để đóng gói thành gói tin. Một tên điều kiện viết khác, hay công suất in `0` thay vì `0.0`, có thể làm Task 3 coi dòng đó là dữ liệu sai hoặc tạo lại file không giống bản gốc khi chuyển ngược.
</details>

**Câu 9.** Ngoại lệ trong `02_ngoai_le` có ảnh hưởng gì tới `load_condition.csv` không?

<details><summary>Xem đáp án</summary>

Không. Task 2.2 chỉ dùng mảng `valid`, đã bỏ ngoại lệ ở Task 2.1, nên `load_condition.csv` của `02_ngoai_le` giống hệt `01_mau_chuan`.
</details>
