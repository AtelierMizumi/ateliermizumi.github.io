---
layout: post
title: "Window manager bằng TypeScript: từ state tới những hình chữ nhật"
date: 2026-11-09
categories: ['build-log', 'frontend']
tags: ['typescript', 'window-manager', 'riceboot', 'testing']
description: "Đọc lõi layout của riceboot, theo một ví dụ ba cửa sổ và kiểm tra một giới hạn mà bộ test xanh chưa bắt được."
lang: vi-VN
comments: false
toc: true
source_revision: be164eebab91ed53ef0a0ccfb0489bbc42677e73
verified_on: 2026-11-09
editorial_status: published
---

Để xếp ba cửa sổ cạnh nhau trong một desktop chạy trên browser, phần lõi chưa cần biết terminal đang hiển thị gì. Nó cần danh sách cửa sổ, vùng có thể sử dụng và khoảng cách giữa các cửa sổ. Kết quả là một bảng tọa độ.

Bài này đọc phần đó trong **riceboot**, tại commit `be164eebab91ed53ef0a0ccfb0489bbc42677e73`. Phạm vi là window state và layout của desktop mô phỏng. Nó không mô tả một Wayland compositor thật, cũng không hướng dẫn chạy Hyprland trong browser.

> **Phạm vi kiểm chứng, ngày 09/11/2026:** bộ Vitest của revision trên đã chạy với 33 file test và 182 test đều pass. Ví dụ tọa độ và trường hợp vùng quá nhỏ bên dưới được gọi trực tiếp từ hàm `dwindle`. Chưa chạy Playwright/E2E hoặc kiểm tra thao tác chuột bằng browser trong lượt kiểm chứng này. Bài đang ở trạng thái draft để tác giả duyệt.
{: .prompt-info }

## Bắt đầu từ dữ liệu, chưa cần animation

Trong [source layout](https://github.com/AtelierMizumi/riceboot/blob/be164eebab91ed53ef0a0ccfb0489bbc42677e73/src/compositor/layout/dwindle.ts), một hình chữ nhật có bốn trường:

```typescript
interface Rect {
  x: number
  y: number
  w: number
  h: number
}
```

`x`, `y` chỉ góc trên bên trái; `w`, `h` là kích thước. Cửa sổ được nhận diện bằng ID riêng. Nhờ vậy, đổi vị trí không cần đổi danh tính cửa sổ, và đổi tiêu đề không cần xây lại mô hình layout.

Hàm được dùng để tính tiling nhận ba đầu vào:

- `ids`: thứ tự các cửa sổ cần xếp.
- `area`: vùng được phép dùng, không nhất thiết là toàn bộ viewport.
- `gaps`: khoảng cách bên ngoài (`outer`) và giữa các ô (`inner`).

Đầu ra là `Record<string, Rect>`, tức mỗi ID ứng với một hình chữ nhật. Hàm không cần DOM, kích thước font hay nội dung app. Với cùng đầu vào, có thể so trực tiếp bảng tọa độ trả về trong test.

## Đi qua ví dụ ba cửa sổ

Dùng đúng fixture của [bộ test layout](https://github.com/AtelierMizumi/riceboot/blob/be164eebab91ed53ef0a0ccfb0489bbc42677e73/src/compositor/layout/dwindle.test.ts):

```javascript
const area = { x: 0, y: 0, w: 1000, h: 600 }
const gaps = { inner: 10, outer: 20 }
const ids = ['a', 'b', 'c']
```

Sau khi chừa outer gaps, ô ban đầu có góc `(20, 20)` và kích thước `960 × 560`. Khi còn nhiều cửa sổ, implementation chia vùng đang xét thành hai phần. Vùng rộng hơn cao thì chia trái/phải; vùng cao hơn rộng thì chia trên/dưới.

Cửa sổ hiện tại nhận phần đầu. Vòng lặp tiếp tục với phần còn lại. Với fixture này, kết quả chạy hàm là:

| Cửa sổ | x | y | w | h |
|---|---:|---:|---:|---:|
| `a` | 20 | 20 | 475 | 560 |
| `b` | 505 | 20 | 475 | 275 |
| `c` | 505 | 305 | 475 | 275 |

Có thể đọc quan hệ giữa các ô như sau; sơ đồ này không theo tỉ lệ:

```text
+------------------+------------------+
|                  |        b         |
|        a         +------------------+
|                  |        c         |
+------------------+------------------+
```

Điểm dễ bỏ qua là **phần còn lại** quyết định nhánh chia tiếp theo. Implementation này không nhận ID của cửa sổ đang focus để chọn ô cần tách. Vì vậy, đây là một layout giản lược cần đọc theo code của nó, không phải lời hứa khớp toàn bộ hành vi của một window manager khác.

Một chi tiết nhỏ khác nằm ở phép làm tròn: kích thước phần đầu dùng `Math.floor`, phần sau lấy phần còn lại. Khi phép chia không đều, hai ô không nhất thiết bằng nhau tuyệt đối. Test nên so với quy tắc đó thay vì giả định mọi lần split đều đối xứng.

## Focus, workspace và geometry là những state khác nhau

[Store của riceboot](https://github.com/AtelierMizumi/riceboot/blob/be164eebab91ed53ef0a0ccfb0489bbc42677e73/src/compositor/store.ts) giữ riêng:

| State | Trách nhiệm |
|---|---|
| `windows` | Thông tin và mode của từng cửa sổ |
| `order` | Thứ tự tiling theo workspace |
| `active` | Workspace đang được chọn |
| `focused` | ID đang nhận focus |
| `area`, `gaps` | Đầu vào hình học |

Khi mở cửa sổ, store thêm ID vào thứ tự của workspace hiện tại và focus cửa sổ mới. Khi gọi `focus(id)`, đường xử lý đó chỉ cập nhật `focused`; nó không tự đổi `order`. Đây là khác biệt giữa chọn một cửa sổ và sắp xếp lại cửa sổ.

Đường tính `layout()` trước hết gọi `dwindle` cho thứ tự của workspace đang active. Sau đó, floating window đi qua nhánh tính/clamp hình riêng, còn fullscreen window nhận `area`. Các mode này không cùng một chính sách hình học.

Cách tách đó cho phép test từng thao tác: đóng cửa sổ đang focus thì chọn ai tiếp theo; chuyển workspace có để lẫn cửa sổ không; đổi `area` có tính lại layout không. [Các test store](https://github.com/AtelierMizumi/riceboot/blob/be164eebab91ed53ef0a0ccfb0489bbc42677e73/src/compositor/store.test.ts) kiểm tra những hành vi này mà không cần mở desktop thật.

## Kết quả test xanh, nhưng có một giới hạn cụ thể

Lệnh đã chạy trong repo ở revision nêu trên:

```bash
pnpm test
```

Phần tổng kết của Vitest:

```text
Test Files  33 passed (33)
     Tests  182 passed (182)
```

Đây là bộ test được [cấu hình trong Vite](https://github.com/AtelierMizumi/riceboot/blob/be164eebab91ed53ef0a0ccfb0489bbc42677e73/vite.config.ts), chạy với jsdom. Nó không phải kết quả Playwright/E2E.

Trong test layout có case vùng `50 × 40`, với outer gap `20`, inner gap `10` và sáu cửa sổ. Assertion của case này kiểm tra số kết quả và kích thước mỗi ô ít nhất là một pixel. Nó **không** kiểm tra mọi ô đều nằm trong `area`.

Gọi hàm thật với chính các đầu vào ấy cho kết quả:

| Cửa sổ | x | y | w | h |
|---|---:|---:|---:|---:|
| `a` | 20 | 20 | 1 | 1 |
| `b` | 31 | 20 | 1 | 1 |
| `c` | 42 | 20 | 1 | 1 |
| `d` | 53 | 20 | 1 | 1 |
| `e` | 64 | 20 | 1 | 1 |
| `f` | 75 | 20 | 1 | 1 |

Các ô `d`, `e`, `f` nằm ngoài cạnh phải của vùng rộng `50`. Bộ test vẫn pass vì kết quả này đáp ứng assertion hiện có: kích thước không âm và không bằng không.

`Math.max(1, ...)` bảo vệ kích thước tối thiểu. Nó không tự bảo vệ điều kiện nằm trong bounds. Hai yêu cầu đó cần được đặt tên và kiểm tra riêng.

Trường hợp này được tái hiện bằng một probe gọi pure function, không phải bằng một sự cố UI đã quan sát. Bài không sửa thuật toán. Nếu yêu cầu của sản phẩm là giữ mọi cửa sổ trong bounds, bước sửa tiếp theo cần bắt đầu bằng test cho yêu cầu đó, rồi chọn chính sách khi không đủ chỗ: giảm gaps, giới hạn tiling hoặc cho phép overflow có chủ đích. Mỗi lựa chọn sẽ thay đổi contract của layout.

## Tự chạy lại hai ví dụ

Chọn một thư mục mới để clone, không dùng lệnh checkout dưới đây trên một working tree đang có thay đổi cần giữ:

```bash
git clone https://github.com/AtelierMizumi/riceboot.git
cd riceboot
git checkout be164eebab91ed53ef0a0ccfb0489bbc42677e73
pnpm install --frozen-lockfile
pnpm test
```

Lượt kiểm chứng cho bài dùng Node.js `v26.13.0` và pnpm `12.9.1`, trên clone đã có dependencies. Chuỗi clone/install ở trên là hướng dẫn tái hiện từ source; lượt này không thực hiện lại một fresh clone và fresh install.

Từ root của repo, chạy đoạn sau bằng Node để gọi cùng hàm TypeScript. Môi trường kiểm chứng hỗ trợ trực tiếp file `.ts`; nếu dùng Node phiên bản khác, cần kiểm tra hỗ trợ TypeScript của phiên bản đó.

```bash
node --input-type=module <<'JS'
import { dwindle } from './src/compositor/layout/dwindle.ts'

const gaps = { inner: 10, outer: 20 }
const normal = dwindle(
  ['a', 'b', 'c'],
  { x: 0, y: 0, w: 1000, h: 600 },
  gaps,
)
console.log(JSON.stringify(normal, null, 2))

const area = { x: 0, y: 0, w: 50, h: 40 }
const tiny = dwindle(['a', 'b', 'c', 'd', 'e', 'f'], area, gaps)
const outside = Object.entries(tiny)
  .filter(([, r]) =>
    r.x < area.x || r.y < area.y ||
    r.x + r.w > area.x + area.w ||
    r.y + r.h > area.y + area.h)
  .map(([id]) => id)
console.log(JSON.stringify({ tiny, outside }, null, 2))
JS
```

`outside` trong lượt kiểm chứng là `["d", "e", "f"]`. Probe chỉ import hàm layout; nó không khởi động VM, sửa config desktop hay mở ứng dụng GUI.

## Phần chưa thể kết luận từ các test này

Một layout thuần dữ liệu dễ kiểm tra hơn ảnh chụp desktop, nhưng trải nghiệm sử dụng còn cần lớp kiểm chứng khác. Test hàm không cho biết focus chuột có đúng không, drag có bị giật không hay animation có mượt không. Source [Window.tsx](https://github.com/AtelierMizumi/riceboot/blob/be164eebab91ed53ef0a0ccfb0489bbc42677e73/src/compositor/Window.tsx) nối state với DOM và pointer events; phần đó cần browser checks riêng trước khi đưa ra kết luận về thao tác thực tế.

Phần hữu ích của ví dụ này là có thể bắt đầu từ một bảng tọa độ, viết rõ điều kiện nào cần đúng và kiểm tra chúng trước khi thêm animation. Trường hợp vùng quá nhỏ cũng cho thấy một bộ test xanh đang bảo vệ contract nào, thay vì bảo vệ mọi điều người đọc có thể tự suy ra từ chữ "window manager".
