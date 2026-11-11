---
layout: post
title: "riceboot: desktop mô phỏng, filesystem của browser và guest Linux là ba chuyện khác nhau"
date: 2026-11-13
categories: ['build-log', 'frontend', 'case-study']
tags: ['riceboot', 'browser', 'linux', 'v86', 'filesystem', 'architecture']
description: "Bạn thấy một desktop Arch trong browser và tưởng Linux đang chạy. Nhưng trong source có ba lớp khác nhau — và chỉ một lớp là Linux thật."
lang: vi-VN
comments: false
toc: true
source_revision: be164eebab91ed53ef0a0ccfb0489bbc42677e73
verified_on: 2026-11-09
editorial_status: published
---

## Mở một tab, thấy một desktop Arch

Bạn mở một tab, thấy một desktop Arch, gõ `fastfetch` vào terminal. Cảm giác đó rất thuyết phục. Nhưng trong source của **riceboot** có ba lớp khác nhau, và chỉ một lớp là Linux thật.

Nhầm lẫn này không ngẫu nhiên: UI của window manager, filesystem mô phỏng và guest Linux đều hiện ra trong cùng một tab. Để nói đúng, phải tách chúng ra.

## Kiểm tra VFS bằng test

Chạy suite VFS tại đúng revision source:

```bash
pnpm exec vitest run src/kernel/vfs/
```

```text
Test Files  4 passed (4)
     Tests  31 passed (31)
```

Đây là kết quả của test suite, không phải một probe Node độc lập. Source cho biết memory VFS được khởi tạo từ `Tree`, đánh dấu `persistent: false`, còn `createVfs()` thử IndexedDB trước rồi mới fallback sang memory.

## Ba lớp, ba trách nhiệm

Tầng filesystem mô phỏng được tạo bằng một hàm có fallback rõ ràng:

```typescript
export async function createVfs(): Promise<Vfs> {
  try {
    return await createIdbVfs()      // ưu tiên IndexedDB
  } catch {
    return createMemoryVfs()         // fallback khi IndexedDB lỗi
  }
}
```

Hợp đồng của tầng này nằm trong `Vfs`:

```typescript
export interface Vfs {
  readFile(p: string): Promise<string>
  writeFile(p: string, data: string): Promise<void>
  mkdir(p: string, opts?: { recursive?: boolean }): Promise<void>
  readdir(p: string): Promise<string[]>
  stat(p: string): Promise<Stat>
  exists(p: string): Promise<boolean>
  rm(p: string, opts?: { recursive?: boolean }): Promise<void>
  watch(prefix: string, cb: (e: WatchEvent) => void): () => void
  reset(): Promise<void>
  readonly persistent: boolean       // có sống qua reload không
}
```

Ba điểm quyết định:

- **`createVfs()` chọn IndexedDB, fallback về memory.** Hai backend cùng một hợp đồng, khác nhau ở `persistent`.
- **`persistent` cho biết backend có hỗ trợ lưu bền hay không.** Memory VFS không giữ dữ liệu sau khi trang đóng; IndexedDB được thiết kế để lưu qua các lần tải lại, tùy chính sách lưu trữ của browser.
- **Tầng này không có kernel.** Nó chỉ là một cấu trúc dữ liệu có `readFile`/`writeFile`/`mkdir`.

Hai phần còn lại gồm `src/compositor/store.ts` (simulator UI), `tools/build-image/build.ts` (build guest image) và `src/vm/VmSession.ts` (khởi tạo v86 trong browser).

## Sơ đồ ba lớp

```
┌──────────────────────────────────────────────┐
│ simulator UI   src/compositor/store.ts        │  bề mặt phản hồi
│   windows, order, active, focused            │  không có kernel
├──────────────────────────────────────────────┤
│ VFS            src/kernel/vfs/                │  bề mặt lưu trữ
│   readFile / writeFile / mkdir / watch       │  IndexedDB hoặc memory
├──────────────────────────────────────────────┤
│ guest Linux                                  │  thực thi ở cấp guest
│   build-image: Alpine ISO + ext2 image       │  chuẩn bị image
│   VmSession.ts: v86                          │  khởi tạo emulator
└──────────────────────────────────────────────┘
```

Simulator và VFS có thể được kiểm thử mà không cần boot guest. Bài này chưa kiểm tra guest hoạt động độc lập với giao diện.

## Trích dẫn gốc

> "This is a riced Arch Linux desktop running in your browser."
> — `src/kernel/vfs/index.ts`, nội dung `README.txt` mặc định trong VFS.

Câu đó là mô tả xuất hiện trong VFS. Nó không chứng minh guest là Arch: image được build từ Alpine. Tầng simulator và VFS cũng không tự biến thành một hệ điều hành Linux đang chạy.

## Kết bài — hỏi đúng tầng

Câu hỏi tốt hơn cho một desktop mô phỏng không phải "nó có chạy Linux không?", vì câu trả lời thuộc về tầng guest. Câu hỏi hữu ích hơn là: "phần nào tôi đang nói tới: simulator, VFS hay guest?".

Bạn có thể đọc test của memory VFS và IndexedDB VFS để xem cùng một interface được kiểm tra qua hai backend như thế nào.

## Verification

- Source đọc tại commit `be164eebab91ed53ef0a0ccfb0489bbc42677e73`.
- `pnpm exec vitest run src/kernel/vfs/` chạy tại source revision `be164eebab91ed53ef0a0ccfb0489bbc42677e73`: 4 file test, 31 test pass.
- Đọc `src/kernel/vfs/index.ts`, `src/kernel/vfs/memory.ts`, `src/compositor/store.ts` và `tools/build-image/build.ts`: VFS IndexedDB/memory và UI compositor là các tầng riêng; image guest được build từ Alpine `alpine-virt-3.24.2-x86.iso`, không phải Arch.
- Không chạy probe Node độc lập, không boot VM thật và không chạy trải nghiệm browser đầy đủ trong lượt kiểm chứng này.
