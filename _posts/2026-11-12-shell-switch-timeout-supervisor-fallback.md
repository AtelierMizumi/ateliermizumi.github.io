---
layout: post
title: "Một nút đổi shell phải biết tự quay lại: timeout, supervisor và fallback"
date: 2026-11-12
categories: ['linux', 'desktop', 'engineering-case-study']
tags: ['niri', 'quickshell', 'shell', 'fallback', 'timeout', 'testing']
description: "Trong switch-shell.sh, timeout được xử lý như từ chối thử shell mới. Đọc state machine và test stub, không nhầm chúng với một lần rollback trên desktop thật."
lang: vi-VN
comments: false
toc: true
source_revision: de1ea53ff5848c8c7fa720ce4f1753a7af23baa7
verified_on: 2026-11-09
editorial_status: published
---

## Không trả lời cũng là một lựa chọn

Nếu shell mới gặp lỗi trong lúc thử, người dùng có thể không kịp xác nhận. Trong `switch-shell.sh`, phản hồi hết thời gian chờ được xử lý như từ chối; test stub kiểm tra rằng file lựa chọn trở về `noctalia`. Đây là hành vi được mô tả và kiểm thử trong script, không phải kết quả của một lần chuyển shell quan sát trên desktop thật.

Điểm đáng chú ý là "không trả lời" đã được định nghĩa thành một lựa chọn: từ chối. Điều đó nghe như chi tiết nhỏ, nhưng nó quyết định script sẽ làm gì khi người dùng không xác nhận.

## Lệnh chạy thật: dry-run và suite

Chạy đúng script tại commit `de1ea53ff5848c8c7fa720ce4f1753a7af23baa7`:

```bash
bash scripts/switch-shell.sh --dry-run
```

```
==> Checking for package conflicts...
==> [dry-run] Preflight checks passed.
==> [dry-run] Would install packages: quickshell ttf-material-symbols-variable brightnessctl cliphist wlsunset
==> [dry-run] Would merge branch smart-shell into main
==> [dry-run] Complete (no changes made).
```

Dry-run chạy hết preflight rồi dừng, không sửa gì. Bộ test stub của repository chạy thật:

```bash
bash tests/test_switch_shell.sh
```

```
PASS t_dry_run_runs_no_mutating_commands
PASS t_conflicting_package_aborts_before_sudo
PASS t_rollback_works_without_quickshell_and_keeps_settings
PASS t_confirm_timeout_reverts_to_noctalia
PASS t_confirm_yes_keeps_mizu
PASS t_the_shell_supervisor_is_stopped_before_the_shell_on_every_switch
PASS t_installs_the_packages_the_shell_runs_tools_from
PASS t_never_installs_leftover_staged_packages_over_the_repo_ones
PASS t_main_checkout_merges_smart_shell_before_stowing
PASS t_failed_merge_stops_before_stow_and_switch
PASS t_refuses_to_stow_from_a_worktree
RESULT passed=11 failed=0 skipped=0
```

Hai test `t_confirm_timeout_reverts_to_noctalia` và `t_confirm_yes_keeps_mizu` kiểm tra hai nhánh timeout và xác nhận giữ shell.

## Thứ tự dừng supervisor

Điểm dễ bỏ qua nhất nằm trong `reload_session()`. Đây là sơ đồ rút gọn, không phải listing nguyên văn; phần dừng supervisor và xử lý shell được mô tả bằng comment:

```bash
reload_session() {
  local target_shell="$1"
  if command -v niri >/dev/null 2>&1; then
    niri msg action load-config-file >/dev/null 2>&1 || true
  fi
  # Dừng shell-start trước; xem giải thích bên dưới.
  # Sau đó xử lý tiến trình theo target_shell.
}
```

Hai điểm quyết định:

- **Supervisor được xử lý trước shell.** Comment trong source nêu `shell-start` tự dừng shell rồi thoát; pattern được neo để khớp command line của supervisor, tránh tìm rộng theo tên file.
- **Target shell được dừng/khởi động sau cùng**, tuỳ theo `target_shell`.

## State machine

```
noctalia (trạng thái ban đầu theo luồng trong script)
   │ chọn mizu
   ▼
mizu (pending)  ── ghi 'mizu' vào ~/.config/niri/shell, reload
   │
   ├─ người dùng giữ lại ─────────────► mizu (committed)
   │
   ├─ người dùng từ chối (N/n) ───────► reverting → noctalia
   │
   └─ read -t hết thời hạn, không trả lời ► reverting → noctalia
```

Mặc định thời hạn là 60 giây; script cho phép đổi bằng `MIZU_CONFIRM_SECONDS`.

Trong luồng xác nhận của script, cả ba nhánh đều có đích. Test stub mô phỏng timeout và kiểm tra file lựa chọn thành `noctalia`; nó không chứng minh việc rollback đã diễn ra trong một phiên desktop thật.

## Trích dẫn gốc

> "shell-start restarts a shell that dies with an error, so it goes first: it stops its shell and exits."
> — `scripts/switch-shell.sh`, comment trong `reload_session()`.

## Kết bài — timeout là một trạng thái, không phải một lỗ

Nếu một tính năng có thể ở trạng thái pending, đừng để timeout bị coi là không có kết quả. Làm cho timeout trở thành một trạng thái có đích rõ ràng; nếu không, người dùng phải chờ đợi hoặc tự đoán điều gì sẽ xảy ra tiếp theo.

Nếu muốn tìm hiểu nhánh rollback riêng, hãy đọc phần `--rollback` trong source và test tương ứng; đừng chạy script chuyển shell trên session đang dùng chỉ để thử bài viết này.

## Verification

- Script đọc tại commit `de1ea53ff5848c8c7fa720ce4f1753a7af23baa7`.
- `bash scripts/switch-shell.sh --dry-run` chạy thật, exit 0, không sửa file trên máy.
- `bash tests/test_switch_shell.sh` chạy thật: 11 pass, 0 fail, trên bộ stub (sudo/pacman/git/stow/niri/pkill giả).
- Test kiểm chứng tính đúng đắn của stub: xác nhận trạng thái cuối và thứ tự lệnh, không phải demo chuyển shell trên desktop thật.
- Không thay đổi session thật, không thay shell, không đưa config host lên bài.
