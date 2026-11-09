---
layout: post
title: "riceboot: desktop mô phỏng, filesystem của browser và một guest Linux là ba chuyện khác nhau"
date: 2026-11-09
categories: ['build-log', 'frontend', 'case-study']
tags: ['riceboot', 'browser', 'linux', 'v86', 'filesystem', 'architecture']
description: "Đọc cách riceboot ghép simulator, VFS trong browser và guest Linux bằng v86 để phân biệt chúng lần lượt là bề mặt phản hồi, bề mặt lưu trữ và phần thực thi bản chất."
lang: vi-VN
comments: false
toc: true
source_revision: be164eebab91ed53ef0a0ccfb0489bbc42677e73
verified_on: 2026-11-09
editorial_status: published
---

Cách branding của một web desktop giả dễ bị hiểu nhầm. Người dùng nhìn thấy UI của window manager trong browser, bấm terminal và tưởng toàn bộ hệ điều hành Arch đang chạy trong tab. Nhưng **riceboot** có ba lớp khác nhau có thể được nhìn thấy trong source: simulator UI, filesystem mô phỏng và guest Linux được boot bằng [v86](https://github.com/copy.sh/v86).

Bài này đọc commit `be164eebab91ed53ef0a0ccfb0489bbc42677e73` của [riceboot](https://github.com/AtelierMizumi/riceboot) để tách ba lớp đó. Bài [window manager bằng TypeScript](/posts/window-manager-typescript-state-to-rectangles/) trước đó đã đi sâu vào layout state machine của riceboot; bài này đọc từ file hệ thống và browser quanh state machine đó.

Phạm vi kiểm chứng là đọc source và kiểm tra runtime logic thuần, không phải một demo VM boot hay trải nghiệm browser đầy đủ.

## Ba lớp và ranh giới của chúng

**Lớp simulator** là phần React/Zustand hiển thị desktop. Store của `src/compositor/store.ts` giữ danh sách cửa sổ, workspace, focus và mode như ở bài trước. Chức năng này hiển thị bề mặt phản hồi; nó không chứa kernel hay driver Linux.

**Lớp filesystem mô phỏng** được triển khai trong `vfs/index.ts`. [Source khởi tạo file system](https://github.com/AtelierMizumi/riceboot/blob/be164eebab91ed53ef0a0ccfb0489bbc42677e73/vfs/index.ts) cho phép desktop simulator lưu/lấy file không cần chạy kernel thật. Nó là bề mặt lưu trữ: ghi file, đọc lại và xem chúng xuất hiện trong UI.

**Lớp guest Linux** tách hoàn toàn khỏi hai lớp trên. Trong [tools/build-image/build.ts](https://github.com/AtelierMizumi/riceboot/blob/be164eebab91ed53ef0a0ccfb0489bbc42677e73/tools/build-image/build.ts), image guest được build trước và boot qua v86 trong browser; kết quả của việc boot là một Linux thật nhỏ ở cấp guest. Đồng thời đáp án về cấu tạo của lớp simulator không cần dùng kernel của guest.

## Vì sao ranh giới này quan trọng

Liên kết ba lớp này với nhau để viết đúng. Đề xuất tuyên bố “chạy Arch trong browser” cần bằng chứng rõ ràng. Image trên thực tế được build từ Alpine trong tools/build-image/build.ts, không phải Arch. Simulator là của người dùng tự viết trong TypeScript. Guest layer là Linux thật, nhưng nằm ở mức VM guest, không phải mức hệ thống chủ của người dùng.

## Hệ quả thực tiễn

Việc phân biệt lớp simulator với lớp guest cũng cho thấy khi người dùng muốn mount một drive mới trong browser, cần quyết định cụ thể thứ được mount là gì: simulator VFS hay guest Linux. Hai đường đi có thể giống nhau ở bề ngoài, nhưng không nhất thiết phải giống nhau ở kernel.

## Nhìn lại sự khác biệt giữa ranh giới lớp bề mặt và phần hard-core

Câu hỏi tốt hơn cho desktop mô phỏng không phải “nó có chạy Linux không?”, vì đáp án thuộc về guest layer. Câu hỏi hữu ích hơn là: “phần nào tôi đang nói tới: simulator, VFS hay guest?” Giữ câu trả lời bằng đúng layer của nó, và tuyên bố là chính xác.
