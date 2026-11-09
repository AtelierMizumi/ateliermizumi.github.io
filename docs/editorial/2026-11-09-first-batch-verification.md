# Đợt nội dung đầu tiên: errata và bài window manager

Ngày thực hiện: 2026-11-09. Bài window manager được duyệt tại local, chuyển sang `_posts/2026-11-09-window-manager-typescript-state-to-rectangles.md` và được kiểm tra trong bản production-manifest trước khi chuẩn bị push.

## Phạm vi đã làm

- Bài Btrfs: thêm safety correction; rút lại các recipe xóa, sửa boot hooks và rollback root chưa được diễn tập; sửa ghi chú Arch từ AUR sang Extra theo package index đã đọc. Không thay bằng một lệnh phá hủy khác.
- Bài Anaconda/Jupyter: tách Conda khỏi Anaconda Distribution; rút đường cài AUR chưa xác minh; đánh dấu version pins cũ là ví dụ lịch sử; bỏ bước init sai và mô tả đúng cách khởi chạy JupyterLab/Notebook.
- Bài Data Analytics: sửa typo tiêu đề và địa chỉ HTTPS localhost cố định, thêm revision note.
- Duyệt bài window manager bằng tiếng Việt, bỏ trạng thái draft và đưa vào `_posts/2026-11-09-window-manager-typescript-state-to-rectangles.md`. Bài đã xuất hiện trong output production và Atom feed sau lần kiểm tra cuối.

Không có storage/boot command nào trong bài cũ được thực thi. Không cài hoặc tạo Conda environment. Các thay đổi này là sửa tài liệu, không chứng nhận toàn bộ hướng dẫn cũ đã được kiểm nghiệm lại.

## Bằng chứng cho draft

Source riceboot được đối chiếu với commit `be164eebab91ed53ef0a0ccfb0489bbc42677e73`:

- `src/compositor/layout/dwindle.ts` và `dwindle.test.ts`.
- `src/compositor/store.ts` và `store.test.ts`.
- `src/compositor/Window.tsx` và `vite.config.ts`.

Các file được trích dẫn khớp revision đó; không sửa source/test của riceboot. Thay đổi có sẵn ở `wrangler.jsonc` được giữ nguyên.

| Kiểm tra thực tế | Kết quả |
|---|---|
| `pnpm test` trong riceboot | 33 file, 182 test pass; không phải Playwright/E2E |
| Chạy chính Node probe trong draft | Pass; bảng tọa độ trong bài khớp output |
| Case area 50×40, inner 10, outer 20 | Tái hiện các ô `d`, `e`, `f` nằm ngoài bounds; kích thước vẫn dương |
| Markdown/front matter guards | Code fences cân bằng; bài đã bỏ `published: false` để xuất hiện ở production |
- Bản render Jekyll | Bài render ở production và preview, đúng ba content tables; các bảng line-number của Rouge không bị tính lẫn |
- Production check | Có bài và link feed trong output production; About hiển thị email công khai mới |

Kết quả overflow là probe pure function, không phải sự cố UI được quan sát bằng browser. Draft phân biệt rõ điều kiện mà test hiện tại bảo vệ với điều kiện bounds chưa được assertion đó kiểm tra. Không sửa hoặc skip test để làm xanh.

## Kiểm tra blog trong môi trường tách biệt

Máy host thiếu Ruby/Bundler và node_modules của blog. Verification dùng snapshot trong scratch; không cài toolchain vào repo hay hệ điều hành host.

- `npm install --ignore-scripts --no-audit --no-fund` trong snapshot frontend.
- `npm run build`: **pass** trong snapshot.
- `npm run test`: **fail**, một lỗi tại `_sass/pages/_post.scss:130`, rule `scss/dollar-variable-no-missing-interpolation`.
- Đã đối chiếu file SCSS với HEAD và snapshot: file không đổi. Không tự sửa stylesheet ngoài phạm vi bài viết. Không khẳng định lỗi này từng xảy ra với dependency versions cũ; snapshot đã resolve dependencies hiện hành.
- Ruby 3.3 trong container tách biệt; `bundle install`, Jekyll production build và draft build (`--drafts --unpublished`): **pass**.
- HTML-Proofer với `--disable-external`: **pass** cho cả production và draft preview. Đây là kiểm tra links/images/scripts nội bộ, không chứng nhận mọi external URL còn hoạt động.
- Snapshot Git không có commit HEAD nên build có các thông báo tra cứu Git-history thất bại; lệnh Jekyll và HTML-Proofer vẫn trả exit 0. Đây không phải bằng chứng metadata lịch sử Git đã được render giống một checkout đầy đủ.
- Chưa kiểm tra browser interaction, mobile layout, Playwright, live deployment, rollback Btrfs hoặc cài mới Conda.

Container `hermes-blog-check-20261109` đã dừng, exit 0; không có cổng preview nào được mở. Container và dependency cache được giữ để có thể kiểm tra lại, không tự cleanup tài nguyên của người dùng.

## Nguồn cho errata

Các trang dưới đã được tải nội dung trực tiếp trong ngày kiểm tra, không chỉ dựa vào search snippet:

- [Arch package: btrfs-assistant, repository Extra](https://archlinux.org/packages/extra/x86_64/btrfs-assistant/).
- [Btrfs subvolume manual](https://btrfs.readthedocs.io/en/stable/btrfs-subvolume.html).
- [Conda documentation](https://docs.conda.io/projects/conda/en/stable/).
- [Anaconda install guide selector](https://www.anaconda.com/docs/getting-started/installation).
- [Jupyter installation and launch instructions](https://jupyter.org/install).
- [JupyterLab extension rebuild workflow](https://jupyterlab.readthedocs.io/en/stable/user/extensions.html).

## Evidence và đường quay lại

Log và probe nằm ở `docs/editorial/evidence/2026-11-09/`. Các log chỉ ghi kết quả thực thi; không thay cho review nội dung hoặc xác nhận của tác giả.

Bản gốc ba bài đã được sao lưu trước khi sửa tại:

`~/.hermes/cache/scratch/blog-content-2026-11-09/backups/`

Đã kiểm tra byte-for-byte: các backup khớp với file tương ứng ở blog commit `98d4af79e7657adf18239ce03da7e77ed522bced`. Scratch có thể được dọn theo chính sách môi trường; revision Git này là mốc nội dung gốc có thể tra lại. Không chạy reset/checkout để khôi phục một cách tự động.

## Còn chờ tác giả

- Trạng thái xuất bản của bài window manager đã được tác giả chấp nhận; nội dung còn lại cần review cho từng đợt tiếp theo.
- Quyết định có tạo task sửa lỗi layout bounds trong riceboot hay chỉ giữ nó như known limitation được công bố rõ.
- Quyết định xử lý lint SCSS bằng task riêng; đợt này không làm build toàn repo thành “all green”.
- Quyền tái phân phối học liệu và asset của các series khác vẫn chưa được xác nhận; không tự gỡ hoặc đăng lại.
