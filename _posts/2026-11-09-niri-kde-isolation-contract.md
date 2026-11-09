---
layout: post
title: "Niri và KDE sống cùng máy: kiểm tra một hợp đồng isolation có giới hạn"
date: 2026-11-09
categories: ['linux', 'desktop', 'engineering-case-study']
tags: ['niri', 'kde-plasma', 'dotfiles', 'isolation', 'testing', 'linux']
description: "Từ lời hứa giữ nguyên desktop còn lại, biến thành một tập file và key có hash, compare được."
lang: vi-VN
comments: false
toc: true
source_revision: de1ea53ff5848c8c7fa720ce4f1753a7af23baa7
verified_on: 2026-11-09
editorial_status: published
---

Cách đơn giản nhất để giữ hai desktop cùng tồn tại là đặt chúng vào hai thư mục riêng. Nhưng một desktop chưa bao giờ chỉ sống trong một thư mục. Nó còn dùng portal, MIME default, theme và gsettings. Nếu một cấu hình đổi nhầm một phần trong số đó, desktop còn lại trông vẫn hoạt động, nhưng cảm giác đã khác trước.

Trong `dotfiles` ở commit `de1ea53ff5848c8c7fa720ce4f1753a7af23baa7`, lời hứa đó không được giữ bằng bằng chứng hình ảnh. Nó được biến thành một tập file/gsettings có thể snapshot và compare. Bài này đọc cách [check-plasma-intact.sh](https://github.com/AtelierMizumi/dotfiles/blob/de1ea53ff5848c8c7fa720ce4f1753a7af23baa7/scripts/check-plasma-intact.sh) làm việc ở revision đó và đối chiếu với kết quả của một lần chạy fixture bị cô lập.

Phạm vi kiểm chứng là đọc source và chạy stub trong một thư mục tạm. Không có session thật nào bị thay đổi, không có package nào được cài và không có configuration host nào bị đưa lên bài.

## Lời hứa được viết thành danh sách

`WATCHED_FILES` của script liệt kê tám relative-path, không phải toàn bộ `~/.config`:

- `kdeglobals`
- `kdedefaults/kdeglobals`
- `mimeapps.list`
- `gtk-3.0/settings.ini`
- `gtk-4.0/settings.ini`
- `xdg-desktop-portal-kderc`
- `xdg-desktop-portal/portals.conf`
- `xdg-desktop-portal/kde-portals.conf`

Hai gsettings keys trong schema `org.gnome.desktop.interface` cũng có trong hợp đồng:

- `color-scheme`
- `gtk-theme`

Lệnh `packages` kiểm tra thêm ba package `xdg-desktop-portal-kde`, `plasma-desktop`, `sddm`; cùng với đó script xác nhận mỗi package có `0 missing files` qua pacman trong phân nhánh của nó.

Hợp đồng này có hai mặt. Nó đủ hẹp để có thể trả lời rõ ràng khi một file bị thay đổi, nhưng không đủ rộng để nói toàn bộ máy không đổi.

## Snapshot, compare và một cảnh báo quan trọng

Khi `refuse_if_plasma_running()` thấy tiến trình `plasmashell` đang chạy, script trả exit code `2` thay vì cho `--state-dir` mới snapshot hoặc compare. Lý do nằm ngay trong comment của script: Plasma có thể tự ghi một số file thuộc diện theo dõi, vì vậy comparison qua một Plasma session dễ phát sinh false positive. Cách mà repository kỳ vọng là chạy từ niri session.

`cmd_snapshot` ghi hash của tám file và giá trị gsettings vào thư mục state riêng. File không tồn tại được ghi là `MISSING`, không phải lỗi. Gsettings không đọc được được ghi là `UNAVAILABLE`.

`cmd_compare` so hash mới với ảnh snapshot, bao gồm cả sự xuất hiện hoặc biến mất của file. Cả hai đều có thể tạo violation nếu giá trị trong hai snapshot khác nhau.

Như vậy, kiểm tra được chia thành ba loại:

1. Snapshot: bắt đầu từ trạng thái hiện tại.
2. Compare: phát hiện thay đổi so với ảnh đã ghi.
3. Packages/noctalia: kiểm tra package và một tập configuration khác.

## Biên độ của một whitelist

Ví dụ, một thay đổi ngoài phạm vi sẽ không làm `compare` báo violation không phải vì điều đó là tốt; đơn giản là file đó không thuộc `WATCHED_FILES`. Ngược lại, nếu Plasma tự ghi chính file trong whitelist, việc so sánh có thể phát sinh false positive. Script cảm nhận giới hạn đó và từ chối chạy trong session của Plasma thay vì ngồi giả vờ so sánh được mọi trạng thái.

Đối tượng không bao gồm trong hợp đồng cho bài này vẫn có thể khác với trước đó. Configuration mới của niri ngoài danh sách, service của hệ thống, mount, filesystem và session khác nằm ngoài phép kiểm tra. Một kết quả `OK: Plasma-shared files and gsettings match the snapshot` nên đọc theo đúng phạm vi tên của nó: số file và key đó khớp, không phải không có bất kỳ thay đổi nào trên máy.

## Từ fixture kiểm tra tới log thật

Trong bộ test stub của repository, mỗi case được chạy độc lập. [Test này](https://github.com/AtelierMizumi/dotfiles/blob/de1ea53ff5848c8c7fa720ce4f1753a7af23baa7/tests/test_check_plasma_intact.sh) xác nhận logic snapshot/compare phát hiện file thay đổi và không phát hiện file không đổi. Đó là kiểm chứng về việc đọc, hash và so sánh, không phải kiểm chứng về mọi hành vi של compositor.

Một workspace fixture được chuẩn bị riêng có thể tái hiện tính chất hẹp này mà không chạm tới host:

1. Tạo một XDG config/config giả và kiểm tra tính ổn định của hash.
2. Thay đổi một file trong danh sách watched.
3. Thay đổi một file không đưa vào danh sách.
4. Ghi lại cả hai kết quả.

Điểm cần lưu ý là bước ba: nếu file ngoài danh sách thay đổi và `compare` không báo lỗi, đó không có nghĩa file đó trống rỗng trên máy thật. Nhưng vì nó không thuộc tập watched, compare không có cơ sở để báo lỗi. Tính chất này không bị hạ thấp thành “zero leakage tuyệt đối”.

## Từ một hợp đồng nhỏ tới bài học chung

Kit thiết kế ở đây có thể mang sang một hệ thống khác::

1. Liệt kê mặt được bảo vệ.
2. Định nghĩa trạng thái gốc để compare.
3. Cho phép kết quả `MISSING` và `UNAVAILABLE`.
4. Đánh dấu rõ sự khác biệt giữa kiểm tra đổi thành công và kiểm tra không đổi.

Câu hỏi tiếp theo cần kiểm tra, nhất là bản chất của hợp đồng này, không phải là “chúng ta có thể giữ mọi file không đổi không?”, mà là “những file nào có ý nghĩa bảo vệ và những thay đổi nào mới thực sự gây nguy hiểm cho desktop còn lại?” Việc quyết định đôi khi không nằm trong script.

Bài tiếp theo đọc một state machine cụ thể đi kèm: *Một nút đổi shell phải biết tự quay lại* — bài đó sẽ được đọc theo thứ tự đọc của series.
