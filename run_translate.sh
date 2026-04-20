#!/bin/bash
# Script chạy nhanh cho Linux/Mac - Dịch comments tự động
# Sử dụng: chmod +x run_translate.sh && ./run_translate.sh

echo "========================================"
echo "   Dịch Comments Sang Tiếng Việt"
echo "========================================"
echo ""

# Kiểm tra Python
if ! command -v python3 &> /dev/null; then
    echo "[ERROR] Python 3 không được cài đặt!"
    echo "Cài đặt Python 3 bằng:"
    echo "  Ubuntu/Debian: sudo apt-get install python3 python3-pip"
    echo "  macOS: brew install python3"
    exit 1
fi

echo "[OK] Python 3 đã được phát hiện"
python3 --version
echo ""

# Kiểm tra thư viện dịch
echo "Đang kiểm tra thư viện dịch..."
if ! python3 -c "import deep_translator" 2>/dev/null; then
    echo "[WARNING] Chưa cài đặt deep-translator"
    echo ""
    echo "Cài đặt thư viện... (điều này có thể mất vài phút)"
    pip3 install deep-translator
    
    if [ $? -ne 0 ]; then
        echo "[ERROR] Không thể cài đặt deep-translator"
        exit 1
    fi
fi

echo "[OK] Thư viện dịch sẵn sàng"
echo ""

# Chạy script
echo "Khởi chạy script dịch..."
echo ""
python3 translate_comments.py

echo ""
echo "========================================"
if [ $? -eq 0 ]; then
    echo "   Hoàn tất thành công!"
else
    echo "   Có lỗi xảy ra"
fi
echo "========================================"
