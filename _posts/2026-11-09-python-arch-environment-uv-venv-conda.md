---
layout: post
title: "Python trên Arch: chọn môi trường trước khi chọn uv, venv hay Conda"
date: 2026-11-09
categories: ['guide', 'linux', 'python']
tags: ['python', 'arch-linux', 'uv', 'venv', 'conda', 'environment']
description: "So sánh uv project environment, venv và Conda theo ngôn ngữ của official docs, không lẫn Conda với Anaconda Distribution."
lang: vi-VN
comments: false
toc: true
verified_on: 2026-11-09
editorial_status: published
---

Cách dùng Python trên Arch thường bắt đầu bằng một câu hỏi cụ sai: “cài uv, venv hay Conda?”. Những tool đó không cùng một khái niệm. `venv` là module tạo virtual environment trong standard library. `uv` quản lý project environment, dùng `.venv`, `uv.lock` và `.python-version`. Conda là package/environment manager có workflow riêng, không phải là tên khác của Anaconda Distribution.

Bài trước trên blog đã từng lẫn Conda với Anaconda và gọi `jupyter build` là bước khởi tạo Jupyter; bài này không lặp lại những lỗi đó. Nó đọc [uv project](https://docs.astral.sh/uv/guides/projects), [uv project environment](https://docs.astral.sh/uv/concepts/projects/environment/) và [venv](https://docs.python.org/3/library/venv.html) từ docs chính thức ngày kiểm tra.

## Ba khái niệm khác nhau

**Virtual environment** là một interpreter và các package được cài vào isololated directory. `venv` làm việc này trong Python standard library, không cần một manager bên ngoài.

**Project environment** của uv là một mô hình cụ thể: `.venv/` chứa môi trường được quản lý per-project, `uv.lock` khóa lại các dependency của project, `.python-version` ghi lại Python version mà uv nên dùng. Khi chạy `uv run` hoặc `uv sync`, uv đồng bộ môi trường với lockfile. Nó có thể tải và quản lý Python version riêng nếu cần.

**Conda environment** nằm trong hệ sinh thái Conda. Nó gồm các package channels, commands và khái niệm environment của Conda. Cách cài đặt và cập nhật của nó khác hẳn Project environment của uv.

Việc lẫn ba khái niệm này dễ khiến độc giả cài thêm một thứ không cần thiết.

## Vì sao thứ tự chọn là env trước, tool sau

Câu hỏi tốt hơn là:

1. Project này có cần isolated Python version riêng cho project không?
2. Nếu cần, ai quản lý project environment?
3. Người dùng đã quen với command nào?

Chọn env trước, rồi mới chọn tool. Một project nhỏ chỉ cần chứng minh rằng Python đâu đó trên máy khác project Python version của mình.

## Kiểm chứng fixture uv

Một project fixture được tạo bằng uv 0.13.7 ở lần kiểm tra này. Khi chạy `uv init --name blog-fixture --package`, project tạo `pyproject.toml` với `name = "blog-fixture"` và `requires-python = ">=3.12"`. Thuật toán suy luận từ các dòng đó là:

- Project khai báo `requires-python` để constraint Python version.
- Environment manager, không phải venv module, sẽ tự tạo project environment tương ứng.

Không cần cài vào system Python, không cần Conda, không cần tạo environment bằng tay.

## Một decision tree ngắn

- Cần isolated project environment + lockfile: uy thuộc uv project.
- Chỉ cần virtual env dùng ngay: thuộc venv.
- Cần Conda ecosystem, channels và environment quản lý riêng: thuộc Conda.

Chọn sai thứ tự cũng là nguồn gây ra hiểu lầm; cùng một env nhưng tool khác nhau có thể cho ra workflow khác nhau.
