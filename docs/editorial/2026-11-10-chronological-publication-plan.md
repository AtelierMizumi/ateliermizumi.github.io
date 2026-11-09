# Blog revival — kế hoạch lịch đăng theo tiến trình thực tế

Ngày lập: 2026-11-10. Trạng thái: **chưa thực hiện bước nào ngoài việc đọc và plan**. Mọi thay đổi cần được duyệt riêng theo từng đợt.

## Nguyên tắc

1. **Mỗi ngày tối đa một bài mới.** Việc công bố nhiều bài cùng một ngày khiến người đọc và thuật toán đều có cảm giác nội dung bị dồn, dù hoá ra đơn giản là đã được lưu cùng lúc.
2. **Front-matter date phải phản ánh ngày xuất bản thực tế.** Không dùng quá khứ tự bịa. Nếu bài viết tuần này, ngày có thể viết là ngày của tuần này, không phải một quá khứ đã sửa.
3. **Series và priority vẫn được giữ.** Bài được sắp xếp theo tiến trình đã xảy ra: đã có group đầu tiên gồm #16, #1, #2; sau đó là #15, #13; cuối cùng là #5 và #32.
4. Việc đổi ngày cần được chấp nhận rõ ràng vì nó có thể đổi URL và break bookmark cũ.
5. Nội dung có `NEEDS_AUTHOR` hoặc execution nhỏ không giả vờ đã hoàn thành; chỉ được đăng sau checkin.

## Trạng thái hiện tại

Đã có năm commit trên `origin/master` từ batch đã đăng:

- `2026-11-09` — Window manager #16
- `2026-11-10` — Isolation #1
- `2026-11-10` — Shell switch #2
- `2026-11-10` — osu! cache #32
- `2026-11-10` — Python Arch #5
- `2026-11-10` — riceboot 3-layer #15
- `2026-11-10` — mizu-l2d #13

Hệ thống hiện muốn chuyển viết ngày trên 5 bài còn lại thành một lịch liên tiếp theo thứ tự đọc series.

## Lịch trình đổi lịch

| Thứ tự đọc | Bài | Ngày xuất bản dự kiến | Ghi chú |
|---:|---|---|---|
| 1 | #16 Window manager bằng TypeScript | 2026-11-10 | Bài đã viết và đã được duyệt; đây là bài đầu tiên |
| 2 | #1 Niri và KDE isolation | 2026-11-11 | Theo sau #16 |
| 3 | #2 Shell switch timeout | 2026-11-12 | Cùng cụm #16–#1–#2 |
| 4 | #15 riceboot 3-layer | 2026-11-13 | Theo sau #16 |
| 5 | #13 mizu-l2d | 2026-11-14 | Sau #15, để phân biệt hai project khác nhau |
| 6 | #5 Python Arch | 2026-11-15 | Theo sau #13 để không chặn trước series khác |
| 7 | #32 osu! cache | 2026-11-16 | Cuối batch này |

**Ghi chú về đổi date:** các bài sau bước 3 và 4 có thể thay đổi thứ tự vào vì mỗi bài thuộc series khác nhau; thứ tự sẽ không có tác động lớn tới việc đọc nội dung.

## Việc thực hiện lịch

| Task | Hành động | Kiểm tra |
|---|---|---|
| T1 | Đổi filename và `date:` ở 6 bài thành ngày dự kiến ở bảng | Mỗi file có đúng một date và đúng filename |
| T2 | Build production và xác nhận HTML-Proofer pass | HTML-Proofer pass |
| T3 | Commit: `docs: reassign publication dates chronologically` và push | Push thành công |
| T4 | Đọc lại remote Pages: xác nhận cả 6 bài đều có mặt | Số bài mới có trong output production |
| T5 | (no-op) Tidak ada commit mới bên ngoài scope đợt 1 | — |

Các log nội bộ nằm ở `docs/editorial/evidence/2026-11-09/` giữ nguyên. Các change chưa có stamp thời gian khác sẽ được ghi lại ở commit riêng.

## Batch B còn lại — đề xuất cadence

Việc xuất bản các bài B còn lại không cần đổi status giả. Lịch dưới đây dùng **mỗi ngày một bài**:

| Tuần | Bài | Nghi chú |
|---|---|---|
| 17-22 Nov | #22, #18, #21, #3, #30 | Nội dung có static + experiment |
| 24-30 Nov | #33, #15, #9, #10, #11 | Ưu tiên filing tốt và ít rủi ro đường link |
| 1-7 Dec | #34, #20, #14, #26, #27 | Cần giữ schedule nhẹ |
| 8-14 Dec | #28, #29, #25, #31, #7 | Lab có test độc lập |
| 15-19 Dec | #4 | Safety gate |

**C bài cần author xác nhận trước:** #6, #12, #23, #24, #35.

## Verification evidence này phụ thuộc

- date đổi xong, các link nội bộ cùng series không bị nội bộ được dùng.
- HTML-Proofer sẽ xác nhận link nội bộ sau đổi lịch; bookmark cũ sẽ trỏ 404 và cần redirect nếu có thể.
- Lịch chỉ thay đổi các ngày này ở 5 bài mới; các bài cũ không bị đổi.

## Câu hỏi cho Thuận

1. Lịch rải rác 2026-11-10…2026-11-16 có phù hợp muốn không?
2. Nếu có bookmark lâu năm cần giữ nguyên URL, có nên giữ nguyên lịch hiện tại và thêm commit "scheduled" cho tương lai thay vì đổi quá khứ?

Nếu Thuận đồng ý, mình sẽ thực hiện các bước T1–T4 và thông báo kết quả cuối.
