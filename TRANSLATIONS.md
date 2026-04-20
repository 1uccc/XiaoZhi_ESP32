# 📝 Bản Dịch Tiếng Việt - Dự Án XiaoZhi ESP32

Dự án đã được dịch sang **Tiếng Việt**. Đây là danh sách các file README đã dịch:

## 📄 File README Chính

| File | Tiếng Anh | Tiếng Việt | Trạng Thái |
|------|-----------|-----------|-----------|
| Trang Chủ | [README.md](README.md) | [README_vi.md](README_vi.md) | ✅ |
| Tiếng Trung | [README_zh.md](README_zh.md) | - | Gốc |
| Tiếng Nhật | [README_ja.md](README_ja.md) | - | Gốc |

## 🔊 Tài Liệu Âm Thanh

| File | Tiếng Anh | Tiếng Việt | Trạng Thái |
|------|-----------|-----------|-----------|
| Kiến Trúc Âm Thanh | [main/audio/README.md](main/audio/README.md) | [main/audio/README_vi.md](main/audio/README_vi.md) | ✅ |

## 📦 Tài Liệu Phân Vùng

| File | Tiếng Anh | Tiếng Việt | Trạng Thái |
|------|-----------|-----------|-----------|
| Phân Vùng v2 | [partitions/v2/README.md](partitions/v2/README.md) | [partitions/v2/README_vi.md](partitions/v2/README_vi.md) | ✅ |

## 🛠️ Công Cụ Hỗ Trợ

### Script Dịch Comments Tự Động
Để dịch tất cả comments trong mã nguồn, sử dụng script:
- 📄 [translate_comments.py](translate_comments.py) - Script chính
- 📋 [TRANSLATE_README.md](TRANSLATE_README.md) - Hướng dẫn nhanh
- 📚 [translate_guide.md](translate_guide.md) - Hướng dẫn chi tiết
- 💾 [requirements.txt](requirements.txt) - Thư viện cần thiết
- 🪟 [run_translate.bat](run_translate.bat) - Runner cho Windows
- 🐧 [run_translate.sh](run_translate.sh) - Runner cho Linux/Mac

## 🚀 Cách Sử Dụng

### 1️⃣ Đọc Tài Liệu Tiếng Việt
Tất cả các file README hiện có phiên bản Tiếng Việt. Chỉ cần tìm file `README_vi.md`:
```bash
# Trang chủ
cat README_vi.md

# Âm thanh
cat main/audio/README_vi.md

# Phân vùng
cat partitions/v2/README_vi.md
```

### 2️⃣ Dịch Comments Trong Code
Chạy script dịch comments:
```bash
# Windows
python translate_comments.py

# Linux/Mac
python3 translate_comments.py
```

## 📊 Thống Kê

- **Tổng file README dịch**: 3
- **Tệp hỗ trợ**: 6
- **Ngôn ngữ hỗ trợ**: 
  - ✅ Tiếng Anh (English)
  - ✅ Tiếng Trung (中文)
  - ✅ Tiếng Nhật (日本語)
  - ✅ **Tiếng Việt (Việt Nam)** - MỚI!

## 🌐 Cộng Đồng

Nếu bạn tìm thấy bất kỳ lỗi dịch nào, vui lòng:
1. Báo cáo lỗi qua [Issues](../../issues)
2. Tham gia [Discord](https://discord.gg/x3S4jgXHk3)
3. Tham gia nhóm QQ: 994694848

## 📝 Ghi Chú Dịch

- Các file README chính đã được dịch sang Tiếng Việt
- Dịch bằng công nghệ dịch tự động với kiểm tra chất lượng
- Các thuật ngữ kỹ thuật được giữ nguyên để dễ hiểu
- Tất cả các liên kết và hình ảnh được giữ nguyên

## ✨ Tính Năng Tiếp Theo

Sắp tới sẽ dịch:
- [ ] Tất cả file README trong các thư mục con
- [ ] Tài liệu trong thư mục `docs/`
- [ ] Comments trong toàn bộ mã nguồn

---

**Cập nhật lần cuối:** 2026-04-20  
**Phiên bản:** 1.0.0  
**Trạng thái:** Hoạt động
