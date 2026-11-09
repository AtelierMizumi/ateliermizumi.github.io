---
layout: post
title: "Một nút đổi shell phải biết tự quay lại: timeout, supervisor và fallback"
date: 2026-11-09
categories: ['linux', 'desktop', 'engineering-case-study']
tags: ['niri', 'quickshell', 'shell', 'fallback', 'timeout', 'testing']
description: "Đọc state machine chuyển shell trong dotfiles: xác nhận, timeout, thứ tự dừng supervisor và đường quay lại."
lang: vi-VN
comments: false
toc: true
source_revision: de1ea53ff5848c8c7fa720ce4f1753a7af23baa7
verified_on: 2026-11-09
editorial_status: published
---

Cách đơn giản nhất để đổi shell là viết file chọn shell rồi reload config. Trong một desktop rice, điều đó có thể giúp người dùng thử một giao diện mới, nhưng đặt ra một câu hỏi khó hơn: người dùng thử xong thì sao? Nếu shell mới lỗi, ai sẽ đưa người dùng về trạng thái cũ?

Trong `dotfiles` ở commit `de1ea53ff5848c8c7fa720ce4f1753a7af23baa7`, [switch-shell.sh](https://github.com/AtelierMizumi/dotfiles/blob/de1ea53ff5848c8c7fa720ce4f1753a7af23baa7/scripts/switch-shell.sh) không chỉ viết `mizu` vào `~/.config/niri/shell`. Nó mở một cửa sổ xác nhận có thời gian đếm ngược, và trạng thái “không chọn gì” được xử lý như một lựa chọn từ chối. Bài này đọc state machine đó và đối chiếu với bộ test stub của repository.

Phạm vi kiểm chứng là đọc source và chạy stub trong sandbox. Không có session thật nào bị chuyển đổi, không có shell nào được thay thế và không có configuration host nào được đưa lên bài.

## State machine trước khi nói nút bấm

State thực tế mà người dùng nhìn thấy là `noctalia` đang chạy. Khi chọn `mizu`, script làm ba việc:

1. Ghi `mizu` vào file chọn shell ở `~/.config/niri/shell`.
2. Reload niri và khởi động shell mới.
3. Mở cửa sổ xác nhận trong 60 giây để người dùng giữ lại hoặc quay lại.

Nếu người dùng nhập lựa chọn bắt đầu bằng `N` hoặc `n`, script viết lại `noctalia` và reload. Nếu không có input trong thời hạn mà `read -t` định trước, script cũng coi đó là lựa chọn từ chối. Nếu `MIZU_CONFIRM_SECONDS` được đặt, script dùng giá trị đó thay cho mặc định.

Như vậy, “không phản hồi” không phải là trạng thái chờ vô hạn. Nó có một kết thúc đã định trước.

## Thứ tự dừng supervisor

Một điểm dễ bỏ qua nhất nằm trong `reload_session()`. Script[stops supervisor](https://github.com/AtelierMizumi/dotfiles/blob/de1ea53ff5848c8c7fa720ce4f1753a7af23baa7/scripts/switch-shell.sh#L45-L47) có tên `shell-start` trước khi nó dừng shell; sau đó nó mới dừng `quickshell`/`qs` hoặc `noctalia` tùy vào shell đích. Lý do nằm ngay trong comment của source: `shell-start` sẽ khởi động lại một shell chết với trạng thái lỗi. Nếu bạn dừng shell trước, supervisor sẽ thấy đó là trạng thái lỗi và có thể chạy lại đúng thứ bạn vừa muốn dừng.

Thứ tự này giải thích vì sao bài báo case study dành nhiều thời gian cho state machine hơn là nút bấm:

- **`noctalia`**: trạng thái fallback chạy bình thường.
- **`mizu` (pending)**: shell mới đã được ghi vào file chọn shell và đang chạy, nhưng chưa được xác nhận.
- **`mizu` (committed)**: người dùng chọn giữ lại hoặc script được gọi với `--yes`.
- **`reverting`**: ghi `noctalia` và chuyển lại sau khi script đọc đáp án không tích cực.
- **`noctalia` (đã về lại)**: trạng thái kết thúc an toàn của quá trình thử.

Một người dùng thử shell mới có thể nhìn thấy màn hình tạm thời của shell lỗi. Điểm khác biệt là máy không cần thao tác tay thứ hai để thoát khỏi trạng thái đó.

## Timeout, confirm và rollback

Ba đường ra từ state pending cũng cần được tách riêng:

- Người dùng giữ lại: script thoát thành công.
- Người dùng từ chối: script viết lại `noctalia`, reload và thoát với mã lỗi; đây cũng là cách tránh cho shell lỗi tồn tại như lựa chọn ngầm.
- Người dùng không trả lời: `read` hết timeout, script ghi lại `noctalia` và reload.

Một chế độ `--rollback` riêng cũng viết `noctalia` mà không nhắc lại lựa chọn. Đó là lối thoát cho người muốn đưa session về mặc định nhanh hơn.

## Kiểm chứng mà script tự cung cấp

Trong [tests/test_switch_shell.sh](https://github.com/AtelierMizumi/dotfiles/blob/de1ea53ff5848c8c7fa720ce4f1753a7af23baa7/tests/test_switch_shell.sh), mỗi test tạo sandbox có lệnh sudo/pacman/git/stow/niri/pkill giả. Lần kiểm chứng ngày 09/11/2026 chạy toàn bộ suite này và nhận 11 pass trên 11 test.

Hai test liên quan trực tiếp tới logic ở đây:

- Timeout xác nhận quay lại `noctalia`.
- `reload_session` phải dừng supervisor trước shell trên mọi lần chuyển.

Các test này kiểm chứng tính đúng đắn của stub: xác nhận trạng thái cuối và thứ tự lệnh, không phải một demo chuyển shell trên desktop thật.

## Từ một nút đổi shell tới mô hình cho tính năng khác

Mô hình này nói lên một điều rộng hơn nút bấm: nếu một tính năng có thể ở trạng thái pending, đừng để timeout bị coi là không có kết quả. Làm cho timeout trở thành một trang thái có đích rõ ràng, nếu không người dùng phải chờ đợi hoặc tự đoán điều gì sẽ xảy ra tiếp theo.

Nhưng bài này vẫn không đo tốc độ chuyển shell, không chứng minh độ mượt của animation, và không thay việc kiểm tra một shell thật trên session. Nó chỉ cho thấy câu hỏi rollback đã được trả lời trong script bằng mô hình trạng thái, và các test stub bảo vệ thứ tự đó.
