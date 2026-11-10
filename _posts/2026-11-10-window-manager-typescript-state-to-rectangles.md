---
layout: post
title: "Window manager bằng TypeScript: khi ba cửa sổ thành sáu cái không nằm trong vùng dùng"
date: 2026-11-10
categories: ['build-log', 'frontend']
tags: ['typescript', 'window-manager', 'riceboot', 'testing']
description: "Tại commit be164eebab91ed53ef0a0ccfb0489bbc42677e73, hàm `dwindle` chia tiling theo hai nhánh; probe thuần túy cho ra 6 ô, trong đó 3 tràn ra ngoài `area`. Bài giới thiệu cách đọc kết quả — không phải tutorial cài đặt."
lang: vi-VN
comments: false
toc: true
source_revision: be164eebab91ed53ef0a0ccfb0489bbc42677e73
verified_on: 2026-11-09
editorial_status: draft-review
---

## Ba cửa nhỏ nhất mà bộ test vẫn pass

Đừng bắt đầu bằng danh sách phạm vi. Bắt đầu bằng thứ gì đó nghiệm đúng, ngược lại lại.

Tại commit `be164eebab91ed53ef0a0ccfb0489bbc42677e73` của **riceboot**, hàm chia phần phủ (`dwindle`) nhận `ids`, `area` và `gaps`, trả về `Record<string, Rect>`. Bệ test layout có một case: 6 cửa sổ nằm trong một vùng dọc rộng `50`, cao `40`, outer gap `20`, inner gap `10`. Trước khi đọc kết quả, hỏi một câu:

> Bộ test này bảo vệ điều gì thực sự — "không tràn" hay "kích thước không âm"?

Dùng cùng assertion hiện có, kết quả của hàm là:

| ID | x | y | w | h |
|---|---|---|---|---|
| a | 20 | 20 | 1 | 1 |
| b | 31 | 20 | 1 | 1 |
| c | 42 | 20 | 1 | 1 |
| d | 53 | 20 | 1 | 1 |
| e | 64 | 20 | 1 | 1 |
| f | 75 | 20 | 1 | 1 |

`d`, `e`, `f` nằm ở `x ≥ 50` — bên phải của vùng dùng. Bộ test vẫn pass vì assertion chỉ kiểm tra kích thước không âm, khác 0 — không kiểm tra sự kiện nằm trong `area`.

> **Hiện trạng kiểm chứng, 09/11/2026:** Vitest chạy 33 file test, 182 test pass (jsdom). Không có Playwright/E2E trong lượt này.

## Lệnh chạy thật, và bảng tọa độ nó trả về

Đoạn dưới đây gọi thẳng hàm layout (pure function, không khởi động VM, không mở GUI). Copy và chạy trong repo cùng phiên bản:

```bash
node --input-type=module <<'JS'
import { dwindle } from './src/compositor/layout/dwindle.ts'

const gaps = { inner: 10, outer: 20 }
const normal = dwindle(['a', 'b', 'c'], { x: 0, y: 0, w: 1000, h: 600 }, gaps)
console.log('normal:\n' + JSON.stringify(normal, null, 2))

const area = { x: 0, y: 0, w: 50, h: 40 }
const tiny = dwindle(['a','b','c','d','e','f'], area, gaps)
const outside = Object.entries(tiny)
  .filter(([, r]) =>
    r.x < area.x || r.y < area.y ||
    r.x + r.w > area.x + area.w || r.y + r.h > area.y + area.h)
  .map(([id]) => id)
console.log('tiny-area:\n' + JSON.stringify(tiny, null, 2))
console.log('outside:', JSON.stringify(outside))
JS
```

```
normal: {"a":{"x":20,"y":20,"w":475,"h":560},"b":{"x":505,"y":20,"w":475,"h":275},"c":{"x":505,"y":305,"w":475,"h":275}}
tiny-area: {"a":{"x":20,"y":20,"w":1,"h":1},"b":{"x":31,"y":20,"w":1,"h":1},"c":{"x":42,"y":20,"w":1,"h":1},"d":{"x":53,"y":20,"w":1,"h":1},"e":{"x":64,"y":20,"w":1,"h":1},"f":{"x":75,"y":20,"w":1,"h":1}}
outside: ["d","e","f"]
```

Lời thử này quay trở về hai câu hỏi: (1) tại sao 3 cửa sổ đầu vào vùng, 3 sau lại tràn? (2) assertion hiện hành bảo vệ điều gì — và điều gì lại bị bỏ lại phía sau?

## Hàm chia hai phần qui định thứ tự cắt

Nền tảng chỉ hai thao tác: chia ngang hoặc chia dọc — tuỳ theo chiều đang xét. Phần đầu của mỗi lần chia thuộc cửa sổ đang xét; phần còn lại tiếp tục dùng cho cửa sổ tiếp theo.

```typescript
export function dwindle(ids: string[], area: Rect, gaps: Gaps): Record<string, Rect> {
  const out: Record<string, Rect> = {}
  if (!ids.length) return out
  let cur: Rect = {                          // 1) gốc: nằm trong lề outer
    x: area.x + gaps.outer,
    y: area.y + gaps.outer,
    w: Math.max(1, area.w - 2 * gaps.outer), // 2) rộng = diện tích - 2*outer; tránh < 1
    h: Math.max(1, area.h - 2 * gaps.outer), // 2) cao tương tự
  }
  for (let i = 0; i < ids.length; i++) {
    if (i === ids.length - 1) { out[ids[i]] = cur; break } // 3) cuối cùng nhận số còn lại
    let first: Rect, rest: Rect
    if (cur.w >= cur.h) {                     // 4) rộng hơn -> chia ngang (trái/phải)
      const w = Math.max(1, Math.floor((cur.w - gaps.inner) / 2)) // 5) phần đầu = floor((w - inner)/2)
      first = { ...cur, w }
      rest = { ...cur, x: cur.x + w + gaps.inner, w: Math.max(1, cur.w - w - gaps.inner) } // 6) phần còn lại: tràn lên 1px, không đánh giới hạn mép area
    } else { ... }                            // 7) cao hơn -> chia dọc
    out[ids[i]] = first
    cur = rest
  }
  return out
}
```

Nhiệm vụ quan trọng nhất trên đây là **số 6**: `rest.w = Math.max(1, cur.w - w - gaps.inner)`. Hàm chỉ đảm bảo giá trị không âm, không đảm bảo `x + w` không vượt `area.x + area.w`. Nên khi diện tích hữu dụng quá hẹp, mỗi vòng lặp lại `cur` di chuyển sang phải (bằng `w + inner`), và những cửa sổ sau cùng bị tràn ra.

> Trích đề mục của hàm từ `dwindle.ts`:
> > "Hyprland's default dwindle layout: the first window fills the area, each following window splits the previous one in half — side by side when the rect is wide, stacked when it is tall. Sizes never drop below 1px."
> > — `src/compositor/layout/dwindle.ts`, docstring.

## Cụm hình để thấy thứ tự cắt

Diện tích thực tế sau lề ngoài ở case `50 × 40` chỉ còn `10 × 1` (dài `50 − 40`, rộng `40 − 40`). Từ `x = 20`, mỗi cửa sau chiếm `1 + 10 = 11` px. Kết quả như hình dưới (không tỉ lệ):

```
y = 20, dùng từ x = 20 đến x = 50
+-----+-----+-----+-----+-----+-----+
|  a  |  b  |  c  |  d  |  e  |  f  |
+-----+-----+-----+-----+-----+-----+
x=20 31   42   53*  64*  75*          → d, e, f tràn bên phải (> mép 50)
```

Mỗi `*` là một cửa sổ nằm ngoài vùng được cho phép, dù hàm vẫn đem ra "dung" theo assertion hiện có.

## State của "cửa sổ" không nằm trong một khối

Biểu tượng `Rect` của `dwindle` qui thụ độc một thứ tự cắt. Những thứ khác trong store của riceboot giữ hạng mục riêng:

| State | Trách nhiệm |
|---|---|
| `windows` | Thông tin và mode của mỗi cửa sổ |
| `order` | Thứ tự tiling theo workspace |
| `active` | Workspace đang chọn |
| `focused` | ID đang nhận focus |
| `area`, `gaps` | Đầu vào hình học |

Khi mở cửa sổ, store thêm ID vào thứ tự workspace hiện tại và focus cửa sổ mới. Gọi `focus(id)` chỉ cập nhật `focused` — không tự đổi `order`. Ngày mà focus, order, area và geometry nằm cùng một khối, bản kiểm soát `dwindle` sẽ khó tách rời, và một thử thách chỉ ảnh hưởng một khối lại sẽ lách rối cả hệ thống.

## Kết bài — điều bạn có thể thử ngay

Quay trở lại đầu bài: khi một test vẫn pass trong khi dữ liệu tràn ra bên ngoài vùng dùng, luôn hỏi ngược: *bộ test bảo vệ điều gì chính xác?* Nếu câu trả lời là "kích thước không âm", thì "cửa sổ nằm trong vùng dùng" vẫn chưa được bảo vệ.

Bạn có thể thử ngay: thay assertion từ các kích thước không âm sang một điều kiện nằm trong bounds trên cả `x` và `y` của `area`. Chọn chính sách khi không đủ chỗ — giảm gaps, giới hạn số ô, hoặc cho phép overflow có chủ đích — sau khi test của condition đó đã mở. Mỗi lựa chọn sẽ đổi contract của layout.

## Verification

- Commit được đọc: `be164eebab91ed53ef0a0ccfb0489bbc42677e73` của `riceboot`.
- Cụm lệnh `dwindle` được gọi trực tiếp từ file TS; phiên bản Node `v26.13.0`, pnpm `12.9.1`, repo đã cài dependencies.
- `npx vitest run src/compositor/layout/dwindle.test.ts` → 6 test pass.
- Toàn bộ Vitest (33 file test) → 182 test pass, trên `jsdom`.
- Không chạy Playwright/E2E, không thao tác pointer/browser trong lượt kiểm chứng này.
