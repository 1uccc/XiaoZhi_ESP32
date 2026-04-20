@echo off
:: Script chạy nhanh cho Windows - Dịch comments tự động
:: Sử dụng: Kép vào file này hoặc gõ: run_translate.bat

echo ========================================
echo    Dich Comments Sang Tieng Viet
echo ========================================
echo.

:: Kiểm tra Python
python --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Python khong duoc cai dat!
    echo Tai Python tu: https://www.python.org/downloads/
    pause
    exit /b 1
)

echo [OK] Python da duoc phat hien
echo.

:: Kiểm tra thư viện dịch
echo Dang kiem tra thu vien dich...
python -c "import deep_translator" >nul 2>&1
if errorlevel 1 (
    echo [WARNING] Chua cai dat deep-translator
    echo.
    echo Cai dat thu vien... (dieu nay co the mat vai phut)
    pip install deep-translator
    if errorlevel 1 (
        echo [ERROR] Khong the cai dat deep-translator
        pause
        exit /b 1
    )
)

echo [OK] Thu vien dich san sang
echo.

:: Chạy script
echo Khoi chay script dich...
echo.
python translate_comments.py

echo.
echo ========================================
if errorlevel 0 (
    echo    Hoan tat thanh cong!
) else (
    echo    Co loi xay ra
)
echo ========================================
pause
