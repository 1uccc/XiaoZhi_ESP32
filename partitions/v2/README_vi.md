# Bảng Phân Vùng Phiên Bản 2

Phiên bản này giới thiệu những cải thiện đáng kể so với v1 bằng cách thêm phân vùng `assets` để hỗ trợ nội dung có thể tải từ mạng và tối ưu hóa bố cục phân vùng cho các kích thước flash khác nhau.

## Thay Đổi Chính Từ v1

### Những Cải Thiện Chính
1. **Thêm Phân Vùng Tài Sản**: Phân vùng `assets` mới cho nội dung có thể tải từ mạng
2. **Thay Thế Phân Vùng Mô Hình**: Phân vùng `model` cũ (960KB) được thay thế bằng phân vùng `assets` lớn hơn
3. **Tối Ưu Hóa Phân Vùng Ứng Dụng**: Giảm kích thước phân vùng ứng dụng để phù hợp với tài sản
4. **Tính Linh Hoạt Nâng Cao**: Hỗ trợ cập nhật nội dung động mà không cần phải flash lại

### Tính Năng Phân Vùng Tài Sản
Phân vùng `assets` lưu trữ:
- **Mô hình từ khóa tỉnh dậy**: Mô hình từ khóa tỉnh dậy có thể tùy chỉnh có thể được tải từ mạng
- **Tệp chủ đề**: Hệ thống chủ đề hoàn chỉnh bao gồm:
  - Phông chữ (phông chữ văn bản và biểu tượng)
  - Hiệu ứng âm thanh và tệp âm thanh
  - Hình nền và các phần tử giao diện người dùng
  - Gói emoji tùy chỉnh
  - Tệp cấu hình ngôn ngữ
- **Nội dung Động**: Tất cả nội dung có thể được cập nhật qua hàng không thông qua các tải xuống HTTP

## So Sánh Bố Cục Phân Vùng

### Bố Cục v1 (16MB)
- `nvs`: 16KB (bộ nhớ không thay đổi)
- `otadata`: 8KB (dữ liệu OTA)
- `phy_init`: 4KB (dữ liệu khởi tạo PHY)
- `model`: 960KB (lưu trữ mô hình - nội dung cố định)
- `ota_0`: 6MB (phân vùng ứng dụng 0)
- `ota_1`: 6MB (phân vùng ứng dụng 1)

### Bố Cục v2 (16MB)
- `nvs`: 16KB (bộ nhớ không thay đổi)
- `otadata`: 8KB (dữ liệu OTA)
- `phy_init`: 4KB (dữ liệu khởi tạo PHY)
- `ota_0`: 4MB (phân vùng ứng dụng 0)
- `ota_1`: 4MB (phân vùng ứng dụng 1)
- `assets`: 8MB (tài sản có thể tải từ mạng)

## Cấu Hình Có Sẵn

### Thiết Bị Flash 8MB (`8m.csv`)
- `nvs`: 16KB
- `otadata`: 8KB
- `phy_init`: 4KB
- `ota_0`: 3MB
- `ota_1`: 3MB
- `assets`: 2MB

### Thiết Bị Flash 16MB (`16m.csv`) - Tiêu Chuẩn
- `nvs`: 16KB
- `otadata`: 8KB
- `phy_init`: 4KB
- `ota_0`: 4MB
- `ota_1`: 4MB
- `assets`: 8MB

### Thiết Bị Flash 16MB (`16m_c3.csv`) - Tối Ưu Hóa ESP32-C3
- `nvs`: 16KB
- `otadata`: 8KB
- `phy_init`: 4KB
- `ota_0`: 4MB
- `ota_1`: 4MB
- `assets`: 4MB (4000K - bị giới hạn bởi các trang mmap có sẵn)

### Thiết Bị Flash 32MB (`32m.csv`)
- `nvsfactory`: 200KB
- `nvs`: 840KB
- `otadata`: 8KB
- `phy_init`: 4KB
- `ota_0`: 4MB
- `ota_1`: 4MB
- `assets`: 16MB

## Lợi Ích

1. **Quản Lý Nội Dung Động**: Người dùng có thể tải xuống và cập nhật các mô hình từ khóa tỉnh dậy, chủ đề và các tài sản khác mà không cần flash lại thiết bị
2. **Kích Thước Ứng Dụng Giảm**: Phân vùng ứng dụng được tối ưu hóa, cho phép có nhiều không gian hơn cho nội dung động
3. **Tùy Chỉnh Nâng Cao**: Hỗ trợ chủ đề tùy chỉnh, từ khóa tỉnh dậy và gói ngôn ngữ nâng cao trải nghiệm người dùng
4. **Tính Linh Hoạt Mạng**: Tài sản có thể được cập nhật độc lập với firmware ứng dụng chính
5. **Sử Dụng Tài Nguyên Tốt Hơn**: Sử dụng hiệu quả bộ nhớ flash với lưu trữ tài sản có thể cấu hình
6. **Cập Nhật Tài Sản OTA**: Tài sản có thể được cập nhật qua hàng không thông qua các tải xuống HTTP

## Chi Tiết Kỹ Thuật

- **Loại Phân Vùng**: Phân vùng tài sản sử dụng kiểu con `spiffs` cho khả năng tương thích hệ thống tệp SPIFFS
- **Ánh Xạ Bộ Nhớ**: Tài sản được ánh xạ bộ nhớ để truy cập hiệu quả trong thời gian chạy
- **Xác Thực Tổng Kiểm Tra**: Kiểm tra toàn vẹn tích hợp đảm bảo tính hợp lệ dữ liệu tài sản
- **Tải Xuống Tiến Bộ**: Tài sản có thể được tải xuống theo tiến độ với theo dõi tiến trình
- **Hỗ Trợ Dự Phòng**: Dự phòng toàn bộ thành tài sản mặc định nếu cập nhật mạng không thành công

## Di Chuyển Từ v1

Khi nâng cấp từ v1 lên v2:
1. **Sao Lưu Dữ Liệu Quan Trọng**: Đảm bảo bất kỳ dữ liệu quan trọng nào trong phân vùng `model` cũ được sao lưu
2. **Flash Bảng Phân Vùng Mới**: Sử dụng bảng phân vùng v2 thích hợp cho kích thước flash của bạn
3. **Tải Xuống Tài Sản**: Thiết bị sẽ tự động tải xuống các tài sản cần thiết vào lần khởi động đầu tiên
4. **Xác Minh Chức Năng**: Đảm bảo tất cả các tính năng hoạt động chính xác với bố cục phân vùng mới
