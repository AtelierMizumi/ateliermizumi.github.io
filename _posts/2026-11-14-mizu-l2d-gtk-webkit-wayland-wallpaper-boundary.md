---
layout: post
title: "mizu-l2d: GTK, WebKit và ranh giới của một wallpaper Wayland"
date: 2026-11-14
categories: ['build-log', 'linux', 'case-study']
tags: ['mizu-l2d', 'gtk', 'webkit', 'wayland', 'wallpaper', 'python']
description: "Đọc cách mizu-l2d tách host GTK/WebKit, renderer và engine của wallpaper Wayland, cùng với giới hạn của từng hướng triển khai."
lang: vi-VN
comments: false
toc: true
source_revision: b7712458764ad001283009a34a1a5d247eff9502
verified_on: 2026-11-09
editorial_status: published
---

Một live wallpaper trên Wayland không phải là “ảnh động có chạy được vào background”. Nó phải quyết định ai quản lý layer, ai render nội dung, và khi nào nó được dừng. Nếu hai quyết định đó bị trộn lẫn, wallper có thể render đúng nhưng vẫn không thể sống cùng trạng thái của compositor.

Bài này đọc commit `b7712458764ad001283009a34a1a5d247eff9502` của [mizu-l2d](https://github.com/AtelierMizumi/mizu-l2d). Mục tiêu không phải giới thiệu kiến trúc như một danh sách tính năng, mà phân biệt ba boundary: host, renderer và engine.

Phạm vi kiểm chứng là đọc source và kiểm tra tĩnh; không có session thật nào bị thay đổi và không có configuration host nào được đưa lên bài.

## Ba boundary chính

**Host** là tiến trình GTK/WebKit của app. Nó chịu trách nhiệm mở cửa sổ background layer và giữ nguồn nội dung. Thuật toán không nằm trong host; nó chỉ chọn khi nào host có thể dừng/khởi động renderer.

**Renderer** là phần WebKit/render engine. Nó nhận nội dung Live2D và trình bày lên bề mặt. Một host chỉ phản ứng với trạng thái mà nó kiểm soát, không cần biết việc nội dung.

**Engine** là codec/phần chứa Live2D và runtime ghi nhớ. Nó là trung gian giữa host và nội dung, nhưng không tự thay đổi cách wallpaper được trình bày.

Ranh giới giữa host và renderer: host chỉ quyết định “khi nào rendering có hiệu lực”; renderer quyết định “hiển thị cái gì”.

## Điểm cần kiểm chứng: load_config()

Việc đọc [wall/mizu-l2d-wall.py](https://github.com/AtelierMizumi/mizu-l2d/blob/b7712458764ad001283009a34a1a5d247eff9502/wall/mizu-l2d-wall.py) tại revision đó cho thấy `load_config()` được load thông qua file config. Trong trường hợp không tồn tại hoặc có lỗi, nó thường fallback sang giá trị mặc định. Một phần code ghi trạng thái lỗi vào log file và vẫn trả về status `ok` trong trường hợp lỗi không nghiêm trọng. Điểm này không phải là rendering; nó là trạng thái của một pipeline.

## Module renderer và engine

Một phần khác của nguồn là [web/main.js](https://github.com/AtelierMizumi/mizu-l2d/blob/b7712458764ad001283009a34a1a5d247eff9502/web/main.js), cho thấy renderer load Cubism SDK và tạo instance. Trong lúc này, host đóng vai trò mount/demount. Nếu host dừng, renderer không còn khả năng xử lý nội dung vì không còn background surface.

## Ranh giới của một live wallpaper

Một wallpaper Wayland không cần kiểm soát input, đặc biệt nếu nó dùng cùng toolkit layer như window manager. Sự khác biệt chính nằm ở việc tường nền là không gian background layer, còn UI là window layer; hai layer này chỉ giao nhau ở compositor.

## Câu hỏi kiểm chứng còn mở

Khi triển khai tiếp theo, cần phân biệt rõ kiểu load_config() cho thấy behavior khác thường: fallback trong trường hợp không tồn tại file config cho phép renderer tiếp tục chạy dù config lỗi. Đó không phải lỗi của host hay renderer, nhưng cần đọc rõ câu trả lời từ log.

Một học liệu cần giữ theo đúng thứ tự: đọc host trước, rồi engine, rồi renderer.

## Kết luận hẹp

Bài này không đưa bất kỳ tuyên bố nào về `100% parity`, native C++ render nhân vật hoàn chỉnh hay hotplug đã được audit. Những tuyên bố đó cần proof runtime của VM và có vai trò riêng.

Nội dung chính là ranh giới bản chất của mizu-l2d: host/ renderer tách biệt bằng trạng thái IPC, không phải bằng một binary duy nhất thực hiện cả hai việc.
