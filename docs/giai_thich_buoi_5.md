# Buổi 5 – Task 2.2 phần 1: điều kiện tải, đầu giờ, sắp xếp

Task 2.2 là phần khó nhất của Task 2. Nó được chia làm hai buổi:
- **Buổi này:** ba "công cụ" mà Task 2.2 cần: xác định điều kiện tải theo Bảng 1, tìm đầu giờ của một thời điểm, và sắp xếp mảng bằng `qsort`.
- **Buổi 6:** dùng ba công cụ đó để tính trung bình giờ, năng lượng tích lũy, và ghi `load_condition.csv`.

Code liên quan trong `task2.c`: `find_load`, `classify_condition`, `hour_start`, `compare_samples`, `compare_hours`.

> Mở file này trong VS Code rồi bấm `Ctrl + Shift + V` để xem dạng đã định dạng.

---

## Mục lục

1. [Đề yêu cầu gì ở Task 2.2](#1-đề-yêu-cầu-gì-ở-task-22)
2. [Bảng 1 và hàm classify_condition](#2-bảng-1-và-hàm-classify_condition)
3. [Hàm find_load](#3-hàm-find_load)
4. [Đầu giờ và hàm hour_start](#4-đầu-giờ-và-hàm-hour_start)
5. [Kiến thức nền: con trỏ void và con trỏ hàm](#5-kiến-thức-nền-con-trỏ-void-và-con-trỏ-hàm)
6. [Sắp xếp bằng qsort](#6-sắp-xếp-bằng-qsort)
7. [Hai hàm so sánh](#7-hai-hàm-so-sánh)
8. [Câu hỏi tự kiểm tra](#8-câu-hỏi-tự-kiểm-tra)

---

## 1. Đề yêu cầu gì ở Task 2.2

> Chương trình cần tính toán công suất trung bình mỗi giờ, ví dụ: công suất trung bình lúc 2026:11:10 16:00:00 là giá trị trung bình của tất cả các giá trị công suất từ 16:00:00 đến 16:59:59. Sau đó xác định tình trạng tải dựa trên giá trị trung bình đó theo Bảng 1. Mức tiêu thụ năng lượng tích lũy là tổng của công suất trung bình mỗi giờ từ thời điểm đầu đến thời điểm hiện tại.

File kết quả `load_condition.csv`:

```
id,time,power,energy,load,condition
1,2026:11:10 16:00:00,28.8,28.8,3,inefficient
2,2026:11:10 16:00:00,25.4,25.4,1,good
...
1,2026:11:10 17:00:00,53.9,82.7,3,good
```

Mỗi dòng là **một cảm biến trong một giờ**. Để làm được, chương trình phải:

1. Gom các mẫu **cùng cảm biến, cùng giờ** lại với nhau → cần biết "giờ" của một mẫu (**đầu giờ**) và cần các mẫu cùng nhóm nằm cạnh nhau (**sắp xếp**).
2. Tính trung bình của mỗi nhóm.
3. Xác định điều kiện tải từ trung bình đó (**Bảng 1**).
4. Cộng dồn trung bình theo từng cảm biến để có năng lượng.

Bước 1 và 3 dùng các công cụ của buổi này; bước 2 và 4 ở buổi 6.

**Lưu ý:** ví dụ `load_condition.csv` trong đề **không khớp** với file thông số tải mẫu (máy bàn 53.9 W được ghi là `good` dù min của máy bàn là 60). Chỉ dùng ví dụ đó để xem định dạng, không dùng làm đáp án.

---

## 2. Bảng 1 và hàm classify_condition

### 2.1. Bảng 1 của đề

Bản dịch tiếng Việt của đề ghi "P phút" (thực ra là P **min**) và "sự thất bại" (là **failure**). Viết lại cho rõ:

| Công suất trung bình P | Điều kiện | Mã |
|---|---|---|
| P = 0 | `off` (tắt) | 1 |
| 0 < P < min | `inefficient` (không hiệu quả) | 2 |
| min ≤ P ≤ max | `good` (tốt) | 3 |
| max < P < failure | `overloaded` (quá tải) | 4 |
| P ≥ failure | `failure` (sự cố) | 5 |
| Trường hợp khác | `unknown` | 0 |

`min`, `max`, `failure` lấy từ file thông số tải, **theo đúng loại tải của cảm biến**. Ví dụ với Lamp (min 5, max 15, failure 20):

```
   off   inefficient       good          overloaded     failure
    │◄──────────►│◄────────────────►│◄─────────────►│◄────────────
    0            5                 15               20
```

| P | Điều kiện |
|---|---|
| 0.0 | off |
| 4.9 | inefficient |
| 5.0 | good (biên dưới thuộc good) |
| 15.0 | good (biên trên thuộc good) |
| 15.1 | overloaded |
| 20.0 | failure (biên failure thuộc failure) |

### 2.2. Code

```c
static int
classify_condition (double power, const LoadSpec *spec)
{
  if (spec == NULL)
    return CONDITION_UNKNOWN;
  if (power == 0.0)
    return CONDITION_OFF;
  if (power < spec->min_power)
    return CONDITION_INEFFICIENT;
  if (power <= spec->max_power)
    return CONDITION_GOOD;
  if (power < spec->failure_power)
    return CONDITION_OVERLOADED;
  return CONDITION_FAILURE;
}
```

### 2.3. Vì sao viết được ngắn như vậy

Đây là một **chuỗi `if` nối tiếp, mỗi nhánh `return` ngay**. Khi chương trình tới được một dòng `if`, mọi điều kiện phía trên đã chắc chắn **sai**. Vì vậy mỗi nhánh chỉ cần một phép so sánh:

| Dòng | Tới được đây nghĩa là | Nên chỉ cần kiểm tra |
|---|---|---|
| `power < spec->min_power` | Đã biết P ≠ 0 | P < min là đủ cho "0 < P < min" |
| `power <= spec->max_power` | Đã biết P ≥ min | P ≤ max là đủ cho "min ≤ P ≤ max" |
| `power < spec->failure_power` | Đã biết P > max | P < failure là đủ cho "max < P < failure" |
| `return CONDITION_FAILURE` | Đã biết P ≥ failure | Không cần kiểm tra gì |

**`spec == NULL`**: cảm biến mang một mã tải không có trong file thông số tải (trường hợp lỗi `Error 04` ở buổi 8). Không có min, max, failure để so, nên điều kiện là `unknown`. Kiểm tra `NULL` phải đứng **đầu tiên**: các dòng sau dùng `spec->min_power`, mà truy cập qua con trỏ `NULL` làm chương trình sập.

**P âm** không thể xảy ra ở đây vì Task 2.1 đã loại mọi giá trị âm khỏi mảng `valid`, và trung bình của các số không âm thì không âm.

**Dùng tên hằng `CONDITION_GOOD` thay vì số 3**: đọc code hiểu ngay, và đúng yêu cầu không viết cứng.

---

## 3. Hàm find_load

```c
static const LoadSpec *
find_load (const LoadSpec specs[], int count, int code)
{
  int i;

  for (i = 0; i < count; i++)
    if (specs[i].code == code)
      return &specs[i];
  return NULL;
}
```

Tìm loại tải có mã `code` trong mảng `specs`. Trả về **địa chỉ** phần tử tìm được (`&specs[i]`), hoặc `NULL` nếu không có.

**Vì sao trả về con trỏ mà không trả về cả struct?** Không phải chép cả khối dữ liệu, và có một giá trị đặc biệt (`NULL`) để báo "không tìm thấy". Kiểu trả về là `const LoadSpec *` vì nơi gọi chỉ đọc thông số, không được sửa.

**Tìm tuần tự** (duyệt từ đầu tới cuối): file tải chỉ có vài loại tải, nên cách đơn giản nhất cũng đủ nhanh.

---

## 4. Đầu giờ và hàm hour_start

### 4.1. Bài toán

Mẫu lúc `16:00:00`, `16:30:00`, `16:59:59` thuộc **cùng** giờ 16. Mẫu lúc `17:00:00` thuộc giờ 17. Cần một cách biến mỗi thời điểm thành "đại diện của giờ chứa nó", tức **đầu giờ**:

| Thời điểm | Đầu giờ |
|---|---|
| 2026:11:10 16:00:00 | 2026:11:10 16:00:00 |
| 2026:11:10 16:42:07 | 2026:11:10 16:00:00 |
| 2026:11:10 16:59:59 | 2026:11:10 16:00:00 |
| 2026:11:10 17:00:00 | 2026:11:10 17:00:00 |

Hai mẫu cùng giờ khi và chỉ khi đầu giờ của chúng bằng nhau. Đầu giờ cũng chính là giá trị ghi vào cột `time` của `load_condition.csv`.

### 4.2. Code

```c
static time_t
hour_start (time_t moment)
{
  struct tm parts = *localtime (&moment);

  parts.tm_min = 0;
  parts.tm_sec = 0;
  parts.tm_isdst = -1;
  return mktime (&parts);
}
```

| Dòng | Giải thích |
|---|---|
| `struct tm parts = *localtime (&moment);` | `localtime` đổi số giây thành năm, tháng, ngày, giờ, phút, giây (buổi 2) và trả về **con trỏ** tới một vùng nhớ dùng chung. Dấu `*` lấy nội dung tại con trỏ đó, và phép gán **chép** toàn bộ struct vào biến riêng `parts`. Phải chép vì lần gọi `localtime` sau sẽ ghi đè vùng nhớ dùng chung. |
| `parts.tm_min = 0; parts.tm_sec = 0;` | Đưa phút và giây về 0, giữ nguyên năm, tháng, ngày, giờ. |
| `parts.tm_isdst = -1;` | Để thư viện tự xác định giờ mùa hè, như buổi 2. |
| `return mktime (&parts);` | Đổi ngược thành số giây. |

### 4.3. Có cách nào ngắn hơn không?

Có thể nghĩ tới `moment - moment % 3600` (bỏ phần dư khi chia cho 3600 giây). Cách đó chỉ đúng khi giờ địa phương lệch giờ UTC một số **nguyên** giờ. Việt Nam lệch 7 giờ nên tình cờ đúng, nhưng một số nơi lệch 30 hoặc 45 phút (Ấn Độ là UTC+5:30), và ở đó kết quả sai. Đi qua `localtime` và `mktime` thì đúng ở mọi múi giờ. Đây là một câu trả lời tốt nếu bị hỏi "sao không dùng phép chia lấy dư?".

---

## 5. Kiến thức nền: con trỏ void và con trỏ hàm

`qsort` cần hai khái niệm mới.

### 5.1. Con trỏ void

`void *` là "con trỏ tới **bất kỳ** kiểu nào". Nó chỉ chứa địa chỉ, không biết ở đó có gì. Muốn đọc dữ liệu thì phải chuyển nó về con trỏ có kiểu cụ thể:

```c
const void *left = ...;          /* chỉ biết đây là một địa chỉ */
const Sample *a = left;          /* giờ coi địa chỉ đó là một Sample */
printf ("%d", a->id);
```

`qsort` dùng `void *` vì nó phải sắp xếp được **mọi** loại mảng: mảng `int`, mảng `Sample`, mảng `HourRecord`...

### 5.2. Con trỏ hàm

Tên một hàm, khi viết không có ngoặc, chính là **địa chỉ của hàm đó**. Có thể truyền địa chỉ này cho một hàm khác, để hàm kia **gọi lại** hàm của mình.

```c
qsort (items, count, sizeof items[0], compare_samples);
                                      ^^^^^^^^^^^^^^^
                         truyền hàm, không gọi nó (không có ngoặc)
```

`qsort` biết cách sắp xếp nhưng không biết "phần tử nào đứng trước". Mỗi khi cần so hai phần tử, nó gọi lại hàm `compare_samples` của bạn để hỏi.

---

## 6. Sắp xếp bằng qsort

### 6.1. Cú pháp

```c
void qsort (void *base, size_t count, size_t size,
            int (*compare) (const void *, const void *));
```

| Tham số | Ý nghĩa | Trong Task 2.2 |
|---|---|---|
| `base` | Địa chỉ đầu mảng | `items` |
| `count` | Số phần tử | `count` |
| `size` | Kích thước **một** phần tử, tính bằng byte | `sizeof items[0]` |
| `compare` | Hàm so sánh hai phần tử | `compare_samples` |

`qsort` có trong `<stdlib.h>`, thuộc thư viện chuẩn nên đề cho phép dùng.

### 6.2. Quy ước của hàm so sánh

Hàm nhận địa chỉ hai phần tử `left`, `right` và trả về:

| Trả về | Nghĩa |
|---|---|
| Số âm | `left` phải đứng **trước** `right` |
| Số dương | `left` phải đứng **sau** `right` |
| 0 | Hai phần tử ngang nhau |

### 6.3. Vì sao phải sắp xếp

Sau khi sắp xếp theo **ID rồi tới thời gian**, mọi mẫu cùng cảm biến nằm liền nhau, và trong đó theo thời gian tăng dần. Vì vậy các mẫu **cùng cảm biến, cùng giờ** tạo thành một **đoạn liên tiếp** trong mảng. Buổi 6 chỉ cần duyệt mảng **một lượt** là gom được mọi nhóm.

Ví dụ với file mẫu, trước và sau khi sắp xếp:

```
Trước (thứ tự trong file):        Sau (theo ID rồi thời gian):
 1  08:00  100.0                    1  08:00  100.0  ┐ cảm biến 1, giờ 08
 2  08:00   10.0                    1  08:30  200.0  ┘
 1  08:30  200.0                    1  09:00   30.0  ┐ cảm biến 1, giờ 09
 2  08:30   12.0                    1  09:30   50.0  ┘
 1  09:00   30.0                    1  10:00    0.0  ┐ cảm biến 1, giờ 10
 2  09:00   16.0                    1  10:30    0.0  ┘
 1  09:30   50.0                    2  08:00   10.0  ┐ cảm biến 2, giờ 08
 2  09:30   18.0                    2  08:30   12.0  ┘
 1  10:00    0.0                    2  09:00   16.0  ┐ cảm biến 2, giờ 09
 2  10:00   25.0                    2  09:30   18.0  ┘
 1  10:30    0.0                    2  10:00   25.0  ┐ cảm biến 2, giờ 10
 2  10:30   15.0                    2  10:30   15.0  ┘
```

Không sắp xếp thì để gom một nhóm phải tìm khắp mảng cho mỗi nhóm, chậm và phức tạp hơn nhiều.

---

## 7. Hai hàm so sánh

### 7.1. compare_samples: theo ID, rồi theo thời gian

```c
static int
compare_samples (const void *left, const void *right)
{
  const Sample *a = left;
  const Sample *b = right;

  if (a->id != b->id)
    return a->id < b->id ? -1 : 1;
  if (a->time != b->time)
    return a->time < b->time ? -1 : 1;
  return 0;
}
```

| Dòng | Giải thích |
|---|---|
| `const Sample *a = left;` | Chuyển con trỏ `void` về con trỏ `Sample` để đọc được các trường (mục 5.1). |
| `if (a->id != b->id) return a->id < b->id ? -1 : 1;` | **Khóa chính** là ID. Khác ID thì ID nhỏ đứng trước. `điều_kiện ? x : y` là toán tử ba ngôi: điều kiện đúng thì lấy `x`, sai thì lấy `y`. |
| `if (a->time != b->time) ...` | Chỉ khi cùng ID mới so **khóa phụ** là thời gian. |
| `return 0;` | Cùng ID và cùng thời gian. Không xảy ra ở đây, vì `read_samples` đã loại mọi dòng trùng. |

**Vì sao không viết `return a->id - b->id;`?** Cách đó ngắn, nhưng với số lớn phép trừ có thể bị tràn. Với `time_t` (8 byte) còn tệ hơn: hiệu hai thời gian ép về `int` (4 byte, là kiểu trả về) có thể bị cắt và đổi dấu, cho thứ tự sai. Dùng phép so sánh thì luôn đúng.

### 7.2. compare_hours: theo thời gian, rồi theo ID

```c
static int
compare_hours (const void *left, const void *right)
{
  const HourRecord *a = left;
  const HourRecord *b = right;

  if (a->time != b->time)
    return a->time < b->time ? -1 : 1;
  if (a->id != b->id)
    return a->id < b->id ? -1 : 1;
  return 0;
}
```

Giống hệt về cấu trúc nhưng **đảo thứ tự hai khóa**. Hàm này sắp các bản ghi kết quả trước khi ghi file, để `load_condition.csv` có thứ tự giống ví dụ của đề: mọi cảm biến của giờ 16, rồi mọi cảm biến của giờ 17...

```
1,2026:11:10 16:00:00,...
2,2026:11:10 16:00:00,...
3,2026:11:10 16:00:00,...
1,2026:11:10 17:00:00,...
```

**Tóm lại, Task 2.2 sắp xếp hai lần:**
1. Mẫu theo **(ID, thời gian)** để gom nhóm (buổi 6).
2. Kết quả theo **(thời gian, ID)** để ghi file đúng thứ tự của đề.

---

## 8. Câu hỏi tự kiểm tra

**Câu 1.** Lamp có min 5, max 15, failure 20. Trung bình giờ là 15.0, 15.05 và 20.0 thì điều kiện là gì?

<details><summary>Xem đáp án</summary>

15.0 là `good` (min ≤ P ≤ max, tính cả biên). 15.05 là `overloaded` (max < P < failure). 20.0 là `failure` (P ≥ failure).
</details>

**Câu 2.** Vì sao `classify_condition` chỉ cần một phép so sánh ở mỗi nhánh?

<details><summary>Xem đáp án</summary>

Mỗi nhánh `return` ngay khi đúng. Tới được một nhánh nghĩa là mọi điều kiện phía trên đều sai, nên phần còn lại của khoảng (ví dụ "P ≥ min" khi xét "P ≤ max") đã được bảo đảm.
</details>

**Câu 3.** Vì sao phải kiểm tra `spec == NULL` trước tiên?

<details><summary>Xem đáp án</summary>

`spec` là `NULL` khi mã tải không có trong file tải. Các dòng sau đọc `spec->min_power`; truy cập qua con trỏ `NULL` làm chương trình sập. Kiểm tra trước để trả `unknown` an toàn.
</details>

**Câu 4.** `hour_start` chép kết quả của `localtime` vào một biến riêng. Vì sao không dùng thẳng con trỏ `localtime` trả về?

<details><summary>Xem đáp án</summary>

`localtime` trả con trỏ tới một vùng nhớ dùng chung; mỗi lần gọi sau sẽ ghi đè nội dung đó. Chép ra biến riêng thì an toàn, và sửa phút, giây trên bản chép không ảnh hưởng gì khác.
</details>

**Câu 5.** Vì sao không tính đầu giờ bằng `moment - moment % 3600`?

<details><summary>Xem đáp án</summary>

Phép đó làm tròn theo giờ UTC. Nó chỉ đúng ở các múi giờ lệch UTC một số nguyên giờ (Việt Nam UTC+7 tình cờ đúng), và sai ở múi giờ lệch 30 hay 45 phút. Đi qua `localtime` và `mktime` đúng ở mọi múi giờ.
</details>

**Câu 6.** `qsort` biết so sánh hai `Sample` bằng cách nào?

<details><summary>Xem đáp án</summary>

Nó không biết. Ta truyền cho nó địa chỉ hàm `compare_samples`; mỗi khi cần so hai phần tử, `qsort` gọi lại hàm này với địa chỉ hai phần tử và dựa vào giá trị trả về (âm, dương, 0).
</details>

**Câu 7.** Vì sao phải sắp xếp các mẫu theo ID rồi thời gian trước khi tính trung bình giờ?

<details><summary>Xem đáp án</summary>

Sau khi sắp xếp, các mẫu cùng cảm biến và cùng giờ nằm liền nhau thành một đoạn, nên chỉ cần duyệt mảng một lượt là gom được từng nhóm và tính trung bình.
</details>

**Câu 8.** Vì sao hàm so sánh không viết `return a->time - b->time;`?

<details><summary>Xem đáp án</summary>

Hiệu hai `time_t` là số 8 byte, còn kiểu trả về là `int` 4 byte. Khi ép về `int`, giá trị có thể bị cắt và đổi dấu, cho thứ tự sai. Phép so sánh `<` không có vấn đề này.
</details>

**Câu 9.** `load_condition.csv` được sắp theo thứ tự nào, và hàm nào làm việc đó?

<details><summary>Xem đáp án</summary>

Theo thời gian, rồi theo ID cảm biến, giống ví dụ của đề. `compare_hours` là hàm so sánh dùng cho lần sắp xếp thứ hai, trên mảng kết quả.
</details>
