---
layout: post
title: "Tìm beatmap khi mất mạng: cache SQLite đổi tốc độ lấy độ tươi thế nào?"
date: 2026-11-09
categories: ['tool', 'python', 'case-study']
tags: ['sqlite', 'offline', 'osu', 'cache', 'python']
description: "Cache beatmap của osu-farm-downloader lưu metadata khi online để tìm kiếm không cần mạng; cùng lúc nó cũng lưu độ tươi và giá trị mặc định cần được chú thích."
lang: vi-VN
comments: false
toc: true
source_revision: afdf0a56a9ade4d2c1be382fe6009ce8224daa2f
verified_on: 2026-11-09
editorial_status: published
---

Cách dùng cache đơn giản nhất là lưu một catalog theo kiểu write-once rồi quên nó. Nhưng cache thường được nạp vào lúc không có mạng; ở thời điểm đó, nó còn lưu lại thời điểm dữ liệu được thấy lần cuối, tức là “độ tươi” của kết quả. Nếu một bản phải tìm kiếm khi offline, nó trả về kết quả đúng theo dữ liệu cũ chứ không phải theo catalog hôm nay.

Trong `osu-farm-downloader` ở commit `afdf0a56a9ade4d2c1be382fe6009ce8224daa2f`, module [beatmap_cache.py](https://github.com/AtelierMizumi/osu-farm-downloader/blob/afdf0a56a9ade4d2c1be382fe6009ce8224daa2f/beatmap_cache.py) làm đúng việc đó. Nó nạp metadata khi quét online và truy vấn offline sau đó. Bài này đọc cách nó làm việc và đối chiếu với fixture tự tạo.

Phạm vi kiểm chứng là đọc source và chạy module với fixture tạm; không dùng dữ liệu của người khác và không gọi API bên ngoài.

## Lần đầu online, lần sau offline

Comment ở đầu file nói rõ flow: metadata được lưu ở `~/.cache/osu-farm-downloader/beatmaps.db` để cho phép tìm kiếm/lọc tức thì không cần mạng. Comment của module cũng viết “dưới 0.1s”, nhưng đó là mục tiêu có điều kiện, không phải kết quả đo. Bài này không lấy con số đó làm kết quả.

Hai bảng chính:

- `sets`: có `set_id`, tiêu đề, nghệ sĩ, creator, `searchable`, `status_id`, `last_seen`.
- `diffs`: có các trường số, Star Rating/AR/BPM cho ba biến thể mod, độ dài, playcount/passcount, điểm farm và tags.

Mỗi lần `upsert_sets()` được gọi, nó thay thế các diff của set đó và ghi `last_seen` mới. `search_cached()` dùng status preset và cột `sr/farm` theo mod đã chọn. `get_stats()` cũng cho thấy sets/diffs và lần nạp gần nhất.

## Tuổi của cache không có nghĩa là TTL

`last_seen` là một dấu thời gian cho biết set được ghi lần cuối; nó không phải chính sách hết hạn. Source không có TTL/eviction. Khi offline, tìm kiếm trả về dữ liệu theo các record đã có, ngay cả khi record đó đã cũ nếu lần nạp cuối cùng đã lâu.

Trả lời chi tiết hơn cho câu hỏi “cache nhanh lấy độ tươi như thế nào?” là nó trả về kết quả theo dữ liệu đã ghi mà không tự làm tươi dữ liệu. Người dùng offline nên thấy rõ: kết quả của cache được quan sát từ thời điểm nạp, không phải từ thời điểm truy vấn.

## Giá trị mặc định và dữ liệu thiếu

Một điểm cần chú ý là các giá trị mặc định trong code _diff_variants_:

- DifficultyRating mặc định `0`.
- AR mặc định `9.0`.
- BPM mặc định `180`.
- HitLength mặc định `90`.
- CS mặc định `4.0`.

Đây không phải là khi dữ liệu thiếu thì cache sẽ bỏ qua record. Nó có thể làm nó trở thành một record hợp lệ trong result set do các giá trị mặc định đó. Nếu người dùng đang lọc theo SR hoặc Farm Score, kết quả offline vẫn có thể hiển thị các map có dữ liệu thiếu. Đó không phải lỗi của API hay máy, mà là hệ quả của việc gán giá trị mặc định trong implementation.

## Kiểm chứng fixture tự tạo

Fixture lần này gồm một set và một diff tự tạo. Gọi `upsert_sets()` với dữ liệu giả và tìm lại bằng `search_cached()`. Kết quả trả về một set và một diff đúng với dữ liệu fixture, không chứa thông tin tài khoản hay dữ liệu external.

Kiểm chứng này cho thấy:

1. `upsert_sets()` lưu set và diff tự tạo.
2. `search_cached()` tìm lại được chúng.
3. Giá trị mặc định và độ tươi dữ liệu vẫn nằm trong hành vi của hành xử lý.

Nó không chứng minh bất kỳ tốc độ nào hay tuổi của catalog thật.

## Kết luận hẹp

Cache SQLite của tool này trả lời được một câu hỏi cụ thể: khi không có mạng, app vẫn có thể truy vấn metadata đã lưu. Nhưng “có thể truy vấn” và “dữ liệu vẫn tươi” là hai thứ khác nhau. Bài này không đưa TTL hay tốc độ sub-second vào làm lời hứa, vì implementation không chứa các tuyên bố đó.
