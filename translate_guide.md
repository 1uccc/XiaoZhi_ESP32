# 🌐 Hướng Dẫn Sử Dụng Script Dịch Comments Tự Động

## 📋 Tổng Quan

Script `translate_comments.py` tự động dịch tất cả comments (chú thích) trong mã nguồn từ **Tiếng Anh/Tiếng Trung sang Tiếng Việt**.

### ✨ Tính Năng

- ✅ Dịch tất cả comments dòng (`// comment`)
- ✅ Dịch tất cả comments khối (`/* comment */`)
- ✅ Bỏ qua comments đã là Tiếng Việt
- ✅ Bỏ qua comments đặc biệt (SPDX, Copyright, tên biến, v.v.)
- ✅ Tự động tạo file backup (.bak)
- ✅ Lưu bộ nhớ dịch để tránh dịch lại
- ✅ Xử lý đệ quy tất cả thư mục con
- ✅ Không cần API key (nếu dùng deep-translator)

---

## 🚀 Cài Đặt & Sử Dụng

### 1️⃣ Cài Đặt Thư Viện Dịch

**Cách 1: Sử dụng deep-translator (Khuyên dùng)**
```bash
pip install deep-translator
```

**Cách 2: Sử dụng Google Cloud Translate**
```bash
pip install google-cloud-translate
```

### 2️⃣ Chạy Script

```bash
python translate_comments.py
```

### 3️⃣ Xác Nhận & Chờ

Script sẽ:
- Tìm tất cả file `.cc` và `.h`
- Yêu cầu xác nhận trước khi bắt đầu
- Dịch từng comment một
- Tạo file backup
- Lưu kết quả

---

## 📊 Ví Dụ Trước & Sau

### Trước Dịch
```cpp
// Initialize the display
auto display = board.GetDisplay();

// Print board name/version info
display->SetChatMessage("system", SystemInfo::GetUserAgent().c_str());

/* Setup the audio service */
auto codec = board.GetAudioCodec();
```

### Sau Dịch
```cpp
// Khởi tạo màn hình
auto display = board.GetDisplay();

// In thông tin tên bo mạch và phiên bản
display->SetChatMessage("system", SystemInfo::GetUserAgent().c_str());

/* Thiết lập dịch vụ âm thanh */
auto codec = board.GetAudioCodec();
```

---

## 🔧 Tùy Chọn & Cấu Hình

### Chỉnh Sửa Script (Nâng Cao)

Mở `translate_comments.py` và sửa:

```python
# Thay đổi thư mục
main_dir = os.path.join(project_root, "main")

# Thay đổi ngôn ngữ đích (mặc định: 'vi' = Tiếng Việt)
target_language = 'vi'

# Tắt/Bật backup
backup = True  # True: tạo backup, False: không tạo
```

---

## 📁 File & Thư Mục

### Tạo Ra
- `translation_cache.json` - Bộ nhớ dịch
- `*.bak` - Các file backup

### Cấu Trúc
```
xiaozhi-esp32-main/
├── translate_comments.py      # Script chính
├── translate_guide.md          # File hướng dẫn này
├── main/
│   ├── application.cc          # Sẽ được dịch
│   ├── application.cc.bak      # Backup
│   ├── main.cc
│   ├── audio/
│   ├── display/
│   └── ...
└── translation_cache.json      # Bộ nhớ dịch
```

---

## ⚠️ Lưu Ý Quan Trọng

### 1. Backup
- Script **tự động tạo file .bak** trước khi sửa
- Nếu có sự cố, khôi phục bằng: `mv file.cc.bak file.cc`

### 2. Bộ Nhớ Dịch
- Lần dịch đầu tiên sẽ chậm (phải dịch từ API)
- Lần tiếp theo nhanh hơn (từ bộ nhớ)
- Xóa `translation_cache.json` để dịch lại

### 3. Chất Lượng Dịch
- Dịch tự động có thể không hoàn hảo
- Nên **kiểm tra lại** một số comments quan trọng
- Có thể chỉnh sửa thủ công nếu cần

### 4. Comments Bị Bỏ Qua
Script sẽ **không dịch** những comments sau:
- Đã là Tiếng Việt
- SPDX License Identifier
- Copyright/Author
- Chỉ là tên biến hoặc số
- Dấu ngoặc đơn

---

## 🎯 Các Trường Hợp Sử Dụng

### 1. Dịch Lần Đầu
```bash
python translate_comments.py
```

### 2. Dịch Thêm File Mới
```bash
# Chỉ các file mới sẽ được dịch
python translate_comments.py
```

### 3. Khôi Phục File Gốc
```bash
# Từ PowerShell
Get-Item *.bak | ForEach-Object { 
    mv $_.FullName $_.FullName.Replace('.bak', '')
}
```

### 4. Xóa Bộ Nhớ (Dịch Lại Toàn Bộ)
```bash
rm translation_cache.json
python translate_comments.py
```

---

## 🐛 Xử Lý Lỗi

### Lỗi: "No module named 'deep_translator'"
```bash
pip install deep-translator
```

### Lỗi: "No module named 'google'"
```bash
pip install google-cloud-translate
```

### Script Chạy Quá Chậm
- Chạy lần thứ 2 sẽ nhanh hơn (từ bộ nhớ)
- Kiểm tra kết nối Internet
- Thử dùng deep-translator thay vì Google Translate

### Dịch Không Đúng
- Xóa `translation_cache.json` để dịch lại
- Chỉnh sửa thủ công trong file
- Kiểm tra comment có bị bỏ qua không

---

## 📝 Ví Dụ Comment Bị Bỏ Qua

```cpp
// SPDX-License-Identifier: MIT  // ← Bỏ qua
// Copyright (c) 2024           // ← Bỏ qua
// version 1.0                   // ← Bỏ qua
// event_group_                  // ← Bỏ qua (tên biến)
// }                             // ← Bỏ qua (dấu ngoặc)
// Khởi tạo                      // ← Không bỏ qua (đã Tiếng Việt)
// Initialize                    // ← Dịch → "Khởi tạo"
```

---

## 🎓 Mẹo & Thủ Thuật

### 1. Kiểm Tra Trước Khi Chạy
```bash
# Xem file nào sẽ được xử lý
Get-ChildItem -Recurse -Include *.cc, *.h | Select-Object FullName
```

### 2. Xem Các Thay Đổi
```bash
# Xem diff giữa file gốc và bản dịch
diff file.cc file.cc.bak
```

### 3. Dịch Một Thư Mục Cụ Thể
Chỉnh sửa `main_dir` trong script:
```python
main_dir = os.path.join(project_root, "main/audio")  # Chỉ dịch audio/
```

### 4. Lưu Bộ Nhớ Dịch
```bash
# Sao lưu bộ nhớ dịch
cp translation_cache.json translation_cache.json.backup
```

---

## 📊 Thống Kê

Sau khi chạy, bạn sẽ thấy:
```
✓ Thành công: 153/153
✗ Thất bại: 0/153
✅ Hoàn tất!
```

---

## 🔄 Quy Trình Công Việc Đề Xuất

1. **Sao lưu dự án** (tùy chọn)
   ```bash
   cp -r xiaozhi-esp32-main xiaozhi-esp32-main.backup
   ```

2. **Chạy script dịch**
   ```bash
   python translate_comments.py
   ```

3. **Kiểm tra kết quả**
   - Mở vài file và xem comments
   - Chỉnh sửa thủ công nếu cần

4. **Commit thay đổi**
   ```bash
   git add main/
   git commit -m "Dịch comments sang Tiếng Việt"
   ```

5. **Xóa file backup** (sau khi chắc chắn)
   ```bash
   Get-ChildItem -Recurse -Include *.bak | Remove-Item
   ```

---

## 💡 Câu Hỏi Thường Gặp (FAQ)

**Q: Script có xóa code không?**
A: Không, script chỉ dịch comments. Code không thay đổi.

**Q: Có thể dịch back sang Tiếng Anh không?**
A: Có, chỉnh sửa script sửa `target_language = 'en'`.

**Q: Cần Internet không?**
A: Có, để gọi API dịch. Lần thứ 2 không cần nếu dùng bộ nhớ.

**Q: Tốc độ dịch bao lâu?**
A: Tùy số file và tốc độ Internet. Khoảng 10-30 phút cho 150 files.

**Q: Có thể dùng dịch giả khác không?**
A: Có, tự chỉnh sửa hàm `translate_text()` trong script.

---

## 📧 Hỗ Trợ & Phản Hồi

Nếu gặp vấn đề:
1. Kiểm tra các lỗi phổ biến ở trên
2. Kiểm tra log khi chạy script
3. Thử khôi phục từ file .bak
4. Kiểm tra kết nối Internet

---

**Tạo bởi:** AI Assistant  
**Ngày tạo:** 2026-04-20  
**Phiên bản:** 1.0.0
