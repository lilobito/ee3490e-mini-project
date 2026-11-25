# Buổi 4 – Task 2.1: tách dữ liệu hợp lệ và ngoại lệ

Task 2.1 chia các mẫu đã đọc được thành hai nhóm: **hợp lệ** (ghi vào `power_valid.csv`) và **ngoại lệ** (ghi vào `power_outlier.csv`). Chỉ các mẫu hợp lệ được dùng cho Task 2.2 và 2.3.

Code liên quan trong `task2.c`: `is_valid_power`, `write_samples`, `filter_outliers` (phần "Task 2.1" của file).

> Mở file này trong VS Code rồi bấm `Ctrl + Shift + V` để xem dạng đã định dạng.

---

## Mục lục

1. [Đề yêu cầu gì](#1-đề-yêu-cầu-gì)
2. [Phân biệt dòng sai và ngoại lệ](#2-phân-biệt-dòng-sai-và-ngoại-lệ)
3. [Kiến thức nền: so sánh số thực và biểu thức logic](#3-kiến-thức-nền-so-sánh-số-thực-và-biểu-thức-logic)
4. [Hàm is_valid_power](#4-hàm-is_valid_power)
5. [Hàm write_samples](#5-hàm-write_samples)
6. [Hàm filter_outliers](#6-hàm-filter_outliers)
7. [Chạy thử](#7-chạy-thử)
8. [Câu hỏi tự kiểm tra](#8-câu-hỏi-tự-kiểm-tra)

---

## 1. Đề yêu cầu gì

> Giả định rằng các giá trị công suất hợp lệ phải nằm trong phạm vi đo hoặc bằng 0 (không nếu tải tắt). Bất kỳ giá trị nào khác đều được coi là không hợp lệ. [...] Các điểm dữ liệu ngoại lệ được lưu trữ trong một tệp có tên `power_outlier.csv`. [...] Dòng đầu tiên là "số lượng điểm dữ liệu ngoại lệ: X". Các giá trị dữ liệu hợp lệ cũng được lưu trữ trong tệp `power_valid.csv` với cùng định dạng như tệp dữ liệu đầu vào.

Phạm vi đo của đồng hồ: **1 W đến 3000 W**. Vậy công suất P hợp lệ khi:

```
P = 0      hoặc      1 ≤ P ≤ 3000
```

| Công suất | Hợp lệ? | Lý do |
|---|---|---|
| 0.0 | Có | Tải tắt |
| 0.5 | Không | Lớn hơn 0 nhưng nhỏ hơn mức đo thấp nhất 1 W |
| 1.0 | Có | Đúng biên dưới |
| 107.8 | Có | Trong dải |
| 3000.0 | Có | Đúng biên trên |
| 3000.1 | Không | Vượt dải đo |
| −10.8 | Không | Công suất âm |
| 3448.8 | Không | Vượt dải đo |

Ví dụ file ngoại lệ trong đề:

```
number of outliers: 6
id,time,power,load
3,2026:11:10 15:15:03,0.2,4
2,2026:11:10 15:16:03,-10.8,1
...
```

Đề viết dòng đầu bằng tiếng Việt ("số lượng điểm dữ liệu ngoại lệ: X") nhưng ví dụ dùng tiếng Anh `number of outliers: 6`. Nhóm theo đúng ví dụ.

---

## 2. Phân biệt dòng sai và ngoại lệ

Đây là chỗ dễ nhầm nhất của Task 2, và rất hay bị hỏi khi vấn đáp.

| | Dòng sai | Ngoại lệ |
|---|---|---|
| Là gì | Dòng **không đọc được**: thiếu trường, trường trống, sai kiểu, ID âm, thời gian sai | Dòng **đọc được**, mọi trường đúng kiểu, nhưng công suất nằm ngoài dải đo |
| Ví dụ | `2,2026:11:10 15:16:03,,3` | `2,2026:11:10 15:16:03,-10.8,1` |
| Phát hiện ở | `parse_sample_line` (buổi 3) | `is_valid_power` (buổi này) |
| Xử lý | Ghi `Error 06: invalid data at line X` vào log, bỏ qua dòng | Ghi vào `power_outlier.csv`, **không** ghi log |
| Có trong `power_valid.csv`? | Không | Không |
| Có trong `power_outlier.csv`? | Không | Có |

Hai loại đều bị loại khỏi phần tính toán 2.2, 2.3, nhưng theo hai con đường khác nhau. Dòng sai là lỗi của **file** (dữ liệu hỏng). Ngoại lệ là **dữ liệu đo** bất thường (cảm biến báo giá trị vô lý), nên được giữ lại trong một file riêng để người dùng xem.

---

## 3. Kiến thức nền: so sánh số thực và biểu thức logic

### 3.1. So sánh với 0 có an toàn không?

Buổi 2 đã nói: số thực lưu dạng nhị phân nên thường không chính xác tuyệt đối, và không nên so sánh hai số **tính toán ra** bằng `==`. Nhưng ở đây, `power` được đọc từ chuỗi bằng `strtod`. Chuỗi `"0"`, `"0.0"` hay `"0.00"` đều cho đúng giá trị 0 (0 biểu diễn chính xác trong nhị phân). Vì vậy `power == 0.0` là an toàn.

Tương tự, so với biên 1.0 và 3000.0: cả hai biểu diễn chính xác, và giá trị đọc từ `"3000.0"` bằng đúng 3000.0, nên `power <= METER_MAX` cho kết quả đúng ở biên.

### 3.2. Biểu thức logic trả về số

Trong C, phép so sánh và các toán tử `&&`, `||`, `!` cho kết quả là **số nguyên**: 1 nếu đúng, 0 nếu sai. Vì vậy một hàm có thể trả về thẳng một biểu thức logic:

```c
return power == 0.0 || (power >= METER_MIN && power <= METER_MAX);
```

| Toán tử | Nghĩa | Thứ tự ưu tiên |
|---|---|---|
| `!` | Phủ định | Cao nhất |
| `&&` | Và | Cao hơn `\|\|` |
| `\|\|` | Hoặc | Thấp nhất |

`&&` ưu tiên cao hơn `||`, nên cặp ngoặc trong biểu thức trên thực ra không bắt buộc. Nhưng nên viết ngoặc để người đọc thấy ngay ý nghĩa: "bằng 0, **hoặc** (nằm giữa hai biên)".

---

## 4. Hàm is_valid_power

```c
static int
is_valid_power (double power)
{
  return power == 0.0 || (power >= METER_MIN && power <= METER_MAX);
}
```

| Phần | Giải thích |
|---|---|
| `static int` | Hàm chỉ dùng trong `task2.c`; trả về 1 hoặc 0. |
| `power == 0.0` | Tải tắt: hợp lệ. |
| `power >= METER_MIN && power <= METER_MAX` | Nằm trong dải đo, **tính cả hai biên**. `METER_MIN` (1.0) và `METER_MAX` (3000.0) định nghĩa trong `common.h`, không viết cứng số. |
| `\|\|` | Một trong hai điều kiện đúng là hợp lệ. |

Một hàm chỉ có một dòng vẫn đáng tách riêng: nó được gọi ở **ba** chỗ (đếm ngoại lệ, ghi file ngoại lệ, ghi file hợp lệ). Nếu sau này quy tắc hợp lệ thay đổi, chỉ sửa một chỗ.

---

## 5. Hàm write_samples

```c
static void
write_samples (FILE *output, const Sample items[], int count, int want_valid)
{
  char text[TIME_TEXT_SIZE];
  int i;

  fprintf (output, "%s\n", DATA_HEADER);
  for (i = 0; i < count; i++)
    {
      if (is_valid_power (items[i].power) != want_valid)
        continue;
      format_time (items[i].time, text, sizeof text);
      fprintf (output, "%d,%s,%.1f,%d\n", items[i].id, text, items[i].power,
               items[i].load);
    }
}
```

| Phần | Giải thích |
|---|---|
| `FILE *output` | File đã mở sẵn. Hàm chỉ ghi, không mở và không đóng; việc đó do nơi gọi làm. |
| `const Sample items[]` | Các mẫu, chỉ đọc. |
| `int want_valid` | 1 nếu muốn ghi các mẫu hợp lệ, 0 nếu muốn ghi các ngoại lệ. Nhờ tham số này, **một** hàm dùng cho cả hai file. |
| `fprintf (output, "%s\n", DATA_HEADER);` | Ghi dòng tiêu đề `id,time,power,load`: đề yêu cầu cùng định dạng với file đầu vào. |
| `if (is_valid_power (...) != want_valid) continue;` | Bỏ qua mẫu không thuộc nhóm đang ghi. `is_valid_power` trả đúng 1 hoặc 0, nên so trực tiếp với `want_valid` được. |
| `format_time (items[i].time, text, sizeof text);` | Đổi `time_t` về chuỗi `YYYY:MM:DD hh:mm:ss` (buổi 2). |
| `fprintf (output, "%d,%s,%.1f,%d\n", ...)` | Ghi một dòng. `%.1f` cho đúng 1 chữ số thập phân như đề yêu cầu, nên `0` được in là `0.0`. |

**Thứ tự dòng** trong file ra giữ nguyên thứ tự trong file vào, vì hàm duyệt mảng từ đầu tới cuối và `samples` lưu theo thứ tự đọc.

---

## 6. Hàm filter_outliers

### 6.1. Code

```c
static int
filter_outliers (const Sample items[], int count, Sample valid_items[],
                 int *valid_count)
{
  FILE *file;
  int i;

  *valid_count = 0;
  for (i = 0; i < count; i++)
    if (is_valid_power (items[i].power))
      valid_items[(*valid_count)++] = items[i];

  file = fopen (OUTLIER_NAME, "w");
  if (file == NULL)
    return 0;
  fprintf (file, "number of outliers: %d\n", count - *valid_count);
  write_samples (file, items, count, 0);
  fclose (file);

  file = fopen (VALID_NAME, "w");
  if (file == NULL)
    return 0;
  write_samples (file, items, count, 1);
  fclose (file);
  return 1;
}
```

### 6.2. Ba bước

**Bước 1: chép các mẫu hợp lệ sang mảng `valid` và đếm.**

```c
valid_items[(*valid_count)++] = items[i];
```

Dòng này làm hai việc: ghi mẫu vào vị trí `*valid_count`, **rồi** tăng `*valid_count` lên 1. Toán tử `++` đặt *sau* (hậu tố) trả về giá trị cũ rồi mới tăng. Viết tách ra cho dễ hiểu:

```c
valid_items[*valid_count] = items[i];
(*valid_count)++;
```

Sau vòng lặp, `valid_items` chứa đúng các mẫu hợp lệ theo thứ tự ban đầu, và Task 2.2 chỉ làm việc trên mảng này, đúng yêu cầu "chỉ những giá trị hợp lệ mới được dùng cho các tác vụ từ 2.2 trở đi".

**Bước 2: ghi file ngoại lệ.** Dòng đầu là số ngoại lệ, phải biết trước khi ghi các dòng. Số ngoại lệ bằng tổng số mẫu trừ số mẫu hợp lệ: `count - *valid_count`. Bước 1 đã đếm xong nên ở đây có ngay con số này.

> **Vì sao không ghi file trong lúc đếm?** Dòng đầu tiên của file là con số tổng, mà con số đó chỉ biết khi đã duyệt hết. Ghi một file văn bản thì không quay lại sửa dòng đầu được. Vì vậy phải đếm xong (bước 1) rồi mới ghi (bước 2).

**Bước 3: ghi file hợp lệ.**

**Mở file bằng `"w"`:** đề cho phép ghi đè file cũ. Nếu `fopen` trả `NULL` (thường vì file cũ đang ở chế độ chỉ đọc), hàm trả 0 và `main` ghi `Error 08: cannot override output file` (buổi 8).

---

## 7. Chạy thử

Bấm đúp `chay_thu_task2.bat`. Các trường hợp liên quan tới Task 2.1:

| Trường hợp | Kiểm tra |
|---|---|
| `01_mau_chuan` | Không có ngoại lệ: dòng đầu `number of outliers: 0`, `power_valid.csv` giống file vào |
| `02_ngoai_le` | 3 ngoại lệ (0.5, −10.8, 3448.8) ở dòng 3, 8, 11; `power_valid.csv` có đúng 12 dòng còn lại; log **rỗng** |
| `03_dong_sai` | 7 dòng sai: có trong log, **không** có trong cả hai file của 2.1 |
| `12_khong_ghi_duoc` | `power_valid.csv` chỉ đọc: `Error 08` |

Xem tận mắt kết quả của `02_ngoai_le`:

```
cd tests\task2\02_ngoai_le
..\power_process.exe load_specs.csv
type power_outlier.csv
```

Kết quả đúng:

```
number of outliers: 3
id,time,power,load
1,2026:11:10 08:15:00,0.5,3
2,2026:11:10 09:15:00,-10.8,5
1,2026:11:10 09:45:00,3448.8,3
```

---

## 8. Câu hỏi tự kiểm tra

**Câu 1.** Công suất 0.5 có hợp lệ không? Còn 0.0 và 3000.0?

<details><summary>Xem đáp án</summary>

0.5 không hợp lệ: khác 0 nhưng nhỏ hơn mức đo thấp nhất 1 W. 0.0 hợp lệ (tải tắt). 3000.0 hợp lệ vì dải đo tính cả biên trên.
</details>

**Câu 2.** Một dòng có công suất trống và một dòng có công suất −10.8 được xử lý khác nhau thế nào?

<details><summary>Xem đáp án</summary>

Công suất trống: dòng không đọc được, là dòng sai, ghi `Error 06` vào log và bỏ qua; không xuất hiện trong file nào của 2.1. Công suất −10.8: dòng đọc được, là ngoại lệ, ghi vào `power_outlier.csv`, không ghi log. Cả hai đều không dùng cho 2.2 và 2.3.
</details>

**Câu 3.** Vì sao so sánh `power == 0.0` an toàn trong trường hợp này?

<details><summary>Xem đáp án</summary>

`power` được đọc trực tiếp từ chuỗi bằng `strtod`, không qua phép tính. Chuỗi `"0"` hay `"0.0"` cho đúng giá trị 0, mà 0 biểu diễn chính xác trong nhị phân. Lo ngại về sai số chỉ áp dụng cho các số tính toán ra.
</details>

**Câu 4.** Vì sao phải đếm xong số ngoại lệ rồi mới ghi `power_outlier.csv`?

<details><summary>Xem đáp án</summary>

Dòng đầu tiên của file là tổng số ngoại lệ, chỉ biết khi đã duyệt hết các mẫu. Đã ghi file văn bản thì không quay lại sửa dòng đầu được, nên phải đếm trước, ghi sau.
</details>

**Câu 5.** `valid_items[(*valid_count)++] = items[i];` làm gì?

<details><summary>Xem đáp án</summary>

Ghi mẫu vào vị trí `*valid_count` hiện tại, rồi tăng `*valid_count` lên 1. Toán tử `++` hậu tố trả về giá trị cũ rồi mới tăng.
</details>

**Câu 6.** Vì sao dùng một hàm `write_samples` với tham số `want_valid`, thay vì viết hai hàm?

<details><summary>Xem đáp án</summary>

Hai file có cùng định dạng (cùng tiêu đề, cùng cách in một dòng), chỉ khác nhóm mẫu được ghi. Một hàm với tham số chọn nhóm tránh lặp code; sửa định dạng chỉ cần sửa một chỗ.
</details>

**Câu 7.** Nếu `power_valid.csv` đang ở chế độ chỉ đọc, chuyện gì xảy ra?

<details><summary>Xem đáp án</summary>

`fopen (VALID_NAME, "w")` trả `NULL`, `filter_outliers` trả 0, `main` ghi `Error 08: cannot override output file` rồi dừng. File ngoại lệ đã được ghi trước đó; các file của 2.2 và 2.3 không được tạo.
</details>
