---
layout: post
title: "Niri và KDE sống cùng máy: hợp đồng isolation chỉ bảo vệ đúng thứ nó liệt kê"
date: 2026-11-11
categories: ['linux', 'desktop', 'engineering-case-study']
tags: ['niri', 'kde-plasma', 'dotfiles', 'isolation', 'testing', 'linux']
description: "Sửa một file không nằm trong danh sách, compare vẫn trả OK — và đó không có nghĩa máy không đổi. Đọc check-plasma-intact.sh để thấy một hợp đồng có giới hạn được viết thành mã."
lang: vi-VN
comments: false
toc: true
source_revision: de1ea53ff5848c8c7fa720ce4f1753a7af23baa7
verified_on: 2026-11-09
editorial_status: published
---

## Sửa một file không được liệt kê, compare vẫn OK

Mở một desktop KDE, chỉnh một file trong `~/.config` mà bạn không nghĩ tới, rồi chạy kiểm tra "desktop còn lại có còn nguyên không". Nó trả `OK`. Bạn có tin không?

Câu trả lời là: `OK` chỉ nói về một tập file nhất định. Nó không phủ toàn bộ máy. Điều đó không phải lỗi — đó là cách hợp đồng được viết. Cái khó là đọc được hợp đồng đó từ mã.

## Lệnh chạy thật trên một fixture cô lập

Fixture cô lập của repository tạo `HOME`, `XDG_CONFIG_HOME` và `XDG_STATE_HOME` tạm, đồng thời dùng stub `pgrep` và `gsettings` để không hỏi session hay cấu hình máy thật. Trong một clone riêng của dotfiles tại source revision, chạy:

```bash
bash tests/test_check_plasma_intact.sh
```

Probe whitelist riêng cũng dùng script của revision đó và một `XDG_CONFIG_HOME` tạm: chụp snapshot, sửa `kdeglobals` thì compare báo `CHANGED` (exit 1); khôi phục nó rồi sửa `not-watched.txt` thì compare vẫn `OK` (exit 0). Script trực tiếp cần stub `PGREP_BIN` để không phụ thuộc trạng thái session host.

Kết quả suite tại revision nêu trên:

```
PASS modified_kdeglobals_exits_1_and_names_file
PASS unchanged_state_exits_0
RESULT passed=15 failed=0 skipped=0
```

Probe riêng cho thấy khi một file ngoài danh sách bị sửa, `compare` vẫn `OK`. Lý do không nằm ở hash, mà ở danh sách.

## Hợp đồng nằm trong một mảng

Toàn bộ hợp đồng là tám relative-path, khai báo thẳng trong script:

```bash
WATCHED_FILES=(
  kdeglobals
  kdedefaults/kdeglobals
  mimeapps.list
  gtk-3.0/settings.ini
  gtk-4.0/settings.ini
  xdg-desktop-portal-kderc
  xdg-desktop-portal/portals.conf
  xdg-desktop-portal/kde-portals.conf
)
```

Hai gsettings keys trong schema `org.gnome.desktop.interface` (`color-scheme`, `gtk-theme`) nằm cùng hợp đồng. Lệnh `packages` kiểm tra thêm ba package `xdg-desktop-portal-kde`, `plasma-desktop`, `sddm`.

```bash
hash_of() {
  local path="$CONFIG_HOME/$1"
  if [[ -f "$path" ]]; then
    sha256sum <"$path" | cut -d' ' -f1
  else
    echo MISSING            # file chưa từng tồn tại cũng là một giá trị, không phải lỗi
  fi
}
```

```bash
while IFS=$'\t' read -r old rel; do
  new=$(hash_of "$rel")
  if [[ "$old" != "$new" ]]; then
    echo "CHANGED: $rel"    # chỉ những path có trong snapshot mới được hỏi
    violations=1
  fi
done <"$state_dir/files.sha256"
```

Ba điểm quyết định nằm ở đây:

- `WATCHED_FILES` xác định phần file của hợp đồng; snapshot còn theo dõi hai gsettings key.
- `hash_of` ghi `MISSING` cho file chưa tồn tại — nên file xuất hiện hoặc biến mất cũng là một thay đổi, không phải một lỗi đọc.
- Vòng `compare` chỉ đọc từng dòng của snapshot. Một file nằm ngoài mảng không bao giờ được hỏi, kể cả khi nó vừa bị sửa.

## Luồng snapshot → compare

```
snapshot
  hash 8 file + 2 gsettings key
  → state_dir/files.sha256, state_dir/gsettings.txt

compare
  for each (old_hash, rel) in files.sha256:
      new_hash = hash_of(rel)        # chỉ rel có trong snapshot
      differ? → CHANGED: rel
  for each (key, old_value) in gsettings.txt:
      differ? → GSETTINGS CHANGED: key
  no violation → OK

~/.config
├── kdeglobals            ← trong WATCHED_FILES → được hỏi
├── mimeapps.list         ← trong WATCHED_FILES → được hỏi
└── not-watched.txt       ← không có trong mảng → không bao giờ được hỏi
```

Khi `refuse_if_plasma_running()` thấy tiến trình `plasmashell` đang chạy, script trả exit code `2` thay vì cho snapshot/compare chạy. Lý do nằm trong comment: Plasma có thể tự ghi một số file thuộc diện theo dõi, nên comparison qua một Plasma session dễ phát sinh false positive. Cách repository kỳ vọng là chạy từ niri session.

## Trích dẫn gốc

> "Prove that a rice install left KDE Plasma alone."
> — `scripts/check-plasma-intact.sh`, header comment.

Câu đó nói rõ mục tiêu: không phải "chứng minh máy không đổi", mà là "chứng minh bản cài rice không động vào phần của Plasma".

## Kết bài — hợp đồng có tên thay vì lời hứa trừu tượng

Một kết quả `OK` chỉ đúng trong phạm vi tên của nó. Câu hỏi đúng không phải "có giữ được mọi file không?", mà là "những file nào có ý nghĩa bảo vệ, và thay đổi nào mới thực sự gây nguy hiểm cho desktop còn lại?". Việc quyết định đó đôi khi không nằm trong script — nằm ở mảng `WATCHED_FILES`.

Bạn có thể thử trong một clone riêng: thêm một path vào `WATCHED_FILES`, tạo snapshot mới, thay đổi file đó rồi compare. Hợp đồng vẫn nhỏ, nhưng giờ bao gồm thêm path này.

## Verification

- Đọc script và test tại `de1ea53ff5848c8c7fa720ce4f1753a7af23baa7`; diff hai file này với revision hiện tại `c58814f48833726870518ff4cff0fd9bd5982513` rỗng.
- Chạy `bash tests/test_check_plasma_intact.sh` tại source revision: `RESULT passed=15 failed=0 skipped=0`.
- Chạy probe cô lập bằng script đúng revision: không đổi → `OK` (exit 0); sửa `kdeglobals` → `CHANGED` (exit 1); file ngoài whitelist thay đổi → `OK` (exit 0). `PGREP_BIN` và `GSETTINGS_BIN` được trỏ tới stub/false; không động tới session thật hay cấu hình host.
- Bài chỉ khẳng định phạm vi của snapshot/compare, không suy rộng từ đó thành bằng chứng toàn bộ máy bất biến.
