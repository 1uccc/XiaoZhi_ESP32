# 🌐 Công Cụ Dịch Comments Tự Động

Dịch tất cả comments (chú thích) trong mã nguồn từ **Tiếng Anh/Tiếng Trung sang Tiếng Việt**.

## ⚡ Bắt Đầu Nhanh

### Windows
```bash
# Cách 1: Kép vào file
run_translate.bat

# Cách 2: Chạy trực tiếp
python translate_comments.py
```

### Linux / macOS
```bash
chmod +x run_translate.sh
./run_translate.sh
```

## 📦 Cài Đặt Thư Viện

```bash
# Cách 1: Tự động (script sẽ cài)
python translate_comments.py

# Cách 2: Thủ công
pip install -r requirements.txt
```

## 📚 Hướng Dẫn Chi Tiết

Đọc [translate_guide.md](translate_guide.md) để:
- Cài đặt chi tiết
- Tùy chỉnh & cấu hình
- Xử lý lỗi
- FAQ & mẹo

## ✨ Tính Năng

- ✅ Dịch comments dòng (`// ...`)
- ✅ Dịch comments khối (`/* ... */`)
- ✅ Tự động tạo backup
- ✅ Bộ nhớ dịch (cache)
- ✅ Bỏ qua comments đặc biệt
- ✅ Xử lý đệ quy tất cả file
- ✅ Không cần API key

## 📂 File

- `translate_comments.py` - Script chính
- `translate_guide.md` - Hướng dẫn chi tiết
- `requirements.txt` - Thư viện cần thiết
- `run_translate.bat` - Runner cho Windows
- `run_translate.sh` - Runner cho Linux/Mac

## 🎯 Ví Dụ

**Trước:**
```cpp
// Initialize the display
auto display = board.GetDisplay();
```

**Sau:**
```cpp
// Khởi tạo màn hình
auto display = board.GetDisplay();
```

## ⚠️ Lưu Ý

- ✓ Tự động tạo file `.bak` (backup)
- ✓ Chỉ dịch comments, không đụng code
- ✓ Có thể chỉnh sửa thủ công nếu cần

## 🆘 Lỗi Thường Gặp

**"No module named 'deep_translator'"**
```bash
pip install deep-translator
```

**Script chạy quá chậm**
- Lần thứ 2 nhanh hơn (từ bộ nhớ)
- Kiểm tra kết nối Internet

**Muốn khôi phục file gốc**
```bash
mv file.cc.bak file.cc
```

## 📝 Ví Dụ Chi Tiết

```bash
# 1. Cài đặt
pip install deep-translator

# 2. Chạy
python translate_comments.py

# 3. Xác nhận
# Nhập: y

# 4. Chờ hoàn tất
# ✓ Thành công: 153/153
# ✅ Hoàn tất!
```

## 🔗 Liên Kết

- [Google Translate API](https://cloud.google.com/translate)
- [Deep Translator](https://github.com/nidhaloff/deep_translator)
- [Python](https://www.python.org/)

---

**Tạo bởi:** AI Assistant  
**Ngày tạo:** 2026-04-20  
**Phiên bản:** 1.0.0

Chúc bạn sử dụng vui vẻ! 🎉
