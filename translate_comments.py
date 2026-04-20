#!/usr/bin/env python3
"""
Script tự động dịch tất cả comments từ Tiếng Anh/Tiếng Trung sang Tiếng Việt
Tác giả: AI Assistant
Ngày tạo: 2026-04-20
"""

import os
import re
import sys
from pathlib import Path
from typing import Tuple, List
import json

try:
    from google.cloud import translate_v2
    GOOGLE_TRANSLATE_AVAILABLE = True
except ImportError:
    GOOGLE_TRANSLATE_AVAILABLE = False
    print("⚠️  google-cloud-translate chưa được cài đặt. Cài đặt: pip install google-cloud-translate")

try:
    from deep_translator import GoogleTranslator
    DEEP_TRANSLATOR_AVAILABLE = True
except ImportError:
    DEEP_TRANSLATOR_AVAILABLE = False
    print("⚠️  deep-translator chưa được cài đặt. Cài đặt: pip install deep-translator")


class CommentTranslator:
    """Lớp để dịch comments trong mã nguồn"""
    
    def __init__(self, use_deep_translator=True):
        """
        Khởi tạo dịch giả comments
        Args:
            use_deep_translator: Sử dụng deep-translator (không cần API key)
        """
        self.use_deep_translator = use_deep_translator
        self.translation_cache = {}
        self.load_cache()
        
    def load_cache(self):
        """Tải bộ nhớ dịch từ file"""
        cache_file = "translation_cache.json"
        if os.path.exists(cache_file):
            try:
                with open(cache_file, 'r', encoding='utf-8') as f:
                    self.translation_cache = json.load(f)
                print(f"✓ Đã tải {len(self.translation_cache)} bản dịch từ bộ nhớ")
            except Exception as e:
                print(f"⚠️  Lỗi tải bộ nhớ: {e}")
    
    def save_cache(self):
        """Lưu bộ nhớ dịch vào file"""
        cache_file = "translation_cache.json"
        try:
            with open(cache_file, 'w', encoding='utf-8') as f:
                json.dump(self.translation_cache, f, ensure_ascii=False, indent=2)
            print(f"✓ Đã lưu bộ nhớ dịch ({len(self.translation_cache)} mục)")
        except Exception as e:
            print(f"⚠️  Lỗi lưu bộ nhớ: {e}")
    
    def translate_text(self, text: str) -> str:
        """
        Dịch văn bản từ Tiếng Anh/Tiếng Trung sang Tiếng Việt
        Args:
            text: Văn bản cần dịch
        Returns:
            Văn bản đã dịch
        """
        if not text or len(text.strip()) == 0:
            return text
        
        # Kiểm tra bộ nhớ
        if text in self.translation_cache:
            return self.translation_cache[text]
        
        try:
            if self.use_deep_translator and DEEP_TRANSLATOR_AVAILABLE:
                translated = self._translate_with_deep_translator(text)
            elif GOOGLE_TRANSLATE_AVAILABLE:
                translated = self._translate_with_google(text)
            else:
                print("❌ Không có dịch giả khả dụng. Cài đặt: pip install deep-translator")
                return text
            
            # Lưu vào bộ nhớ
            self.translation_cache[text] = translated
            return translated
        except Exception as e:
            print(f"⚠️  Lỗi dịch '{text}': {e}")
            return text
    
    def _translate_with_deep_translator(self, text: str) -> str:
        """Dịch sử dụng deep-translator"""
        try:
            translator = GoogleTranslator(source='auto', target='vi')
            result = translator.translate(text)
            return result
        except Exception as e:
            print(f"⚠️  Lỗi deep-translator: {e}")
            return text
    
    def _translate_with_google(self, text: str) -> str:
        """Dịch sử dụng Google Cloud Translate"""
        try:
            client = translate_v2.Client()
            result = client.translate_text(text, target_language='vi')
            return result['translatedText']
        except Exception as e:
            print(f"⚠️  Lỗi Google Translate: {e}")
            return text
    
    def is_vietnamese(self, text: str) -> bool:
        """Kiểm tra xem văn bản đã là Tiếng Việt chưa"""
        vietnamese_chars = 'àáảãạăằắẳẵặâầấẩẫậèéẻẽẹêềếểễệìíỉĩịòóỏõọôồốổỗộơờớởỡợùúủũụưừứửữựỳýỷỹỵđ'
        count = sum(1 for c in text.lower() if c in vietnamese_chars)
        return count > len(text) * 0.3
    
    def translate_line_comment(self, comment: str) -> str:
        """
        Dịch comment dòng (// comment)
        Args:
            comment: Comment text (không bao gồm //)
        Returns:
            Comment đã dịch
        """
        comment = comment.strip()
        
        # Bỏ qua các comment đặc biệt
        if self._should_skip(comment):
            return comment
        
        # Nếu đã là Tiếng Việt, bỏ qua
        if self.is_vietnamese(comment):
            return comment
        
        translated = self.translate_text(comment)
        return translated
    
    def translate_block_comment(self, comment: str) -> str:
        """
        Dịch comment khối (/* comment */)
        Args:
            comment: Comment text (không bao gồm /* */)
        Returns:
            Comment đã dịch
        """
        comment = comment.strip()
        
        if self._should_skip(comment):
            return comment
        
        if self.is_vietnamese(comment):
            return comment
        
        translated = self.translate_text(comment)
        return translated
    
    def _should_skip(self, comment: str) -> bool:
        """Kiểm tra xem comment có nên bỏ qua không"""
        skip_patterns = [
            r'^\s*SPDX-License-Identifier:',
            r'^\s*\(c\)',
            r'^\s*Copyright',
            r'^\s*Author:',
            r'^\s*Date:',
            r'^\s*\d+\.\d+',  # Số phiên bản
            r'^[a-zA-Z0-9_\-\.]+$',  # Chỉ là tên biến/hàm
            r'^\s*}',  # Chỉ là dấu ngoặc
            r'^\s*{\s*$',
        ]
        
        for pattern in skip_patterns:
            if re.match(pattern, comment):
                return True
        return False
    
    def process_file(self, filepath: str, backup=True) -> Tuple[bool, str]:
        """
        Xử lý một file - dịch tất cả comments
        Args:
            filepath: Đường dẫn file
            backup: Có tạo file backup không
        Returns:
            (Thành công, Thông báo)
        """
        if not filepath.endswith(('.cc', '.h')):
            return False, "File không phải .cc hoặc .h"
        
        try:
            # Đọc file
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()
            
            original_content = content
            
            # Dịch line comments
            content = self._translate_line_comments(content)
            
            # Dịch block comments
            content = self._translate_block_comments(content)
            
            # Kiểm tra có thay đổi không
            if content == original_content:
                return True, "Không cần thay đổi"
            
            # Tạo backup
            if backup:
                backup_path = filepath + '.bak'
                with open(backup_path, 'w', encoding='utf-8') as f:
                    f.write(original_content)
            
            # Ghi file mới
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)
            
            return True, "Dịch thành công"
        
        except Exception as e:
            return False, f"Lỗi: {str(e)}"
    
    def _translate_line_comments(self, content: str) -> str:
        """Dịch tất cả line comments"""
        pattern = r'//(.+?)$'
        
        def replacer(match):
            comment_text = match.group(1)
            translated = self.translate_line_comment(comment_text)
            return f"// {translated}"
        
        result = re.sub(pattern, replacer, content, flags=re.MULTILINE)
        return result
    
    def _translate_block_comments(self, content: str) -> str:
        """Dịch tất cả block comments"""
        pattern = r'/\*(.*?)\*/'
        
        def replacer(match):
            comment_text = match.group(1)
            translated = self.translate_block_comment(comment_text)
            return f"/* {translated} */"
        
        result = re.sub(pattern, replacer, content, flags=re.DOTALL)
        return result
    
    def process_directory(self, directory: str, recursive=True, backup=True) -> List[Tuple[str, bool, str]]:
        """
        Xử lý toàn bộ thư mục
        Args:
            directory: Đường dẫn thư mục
            recursive: Xử lý đệ quy tất cả thư mục con
            backup: Tạo file backup
        Returns:
            Danh sách (filepath, thành công, thông báo)
        """
        results = []
        
        # Tìm tất cả file .cc và .h
        if recursive:
            pattern = "**/*.{cc,h}"
        else:
            pattern = "*.{cc,h}"
        
        path_obj = Path(directory)
        
        # Sử dụng glob để tìm file
        files = []
        if recursive:
            files.extend(path_obj.glob("**/*.cc"))
            files.extend(path_obj.glob("**/*.h"))
        else:
            files.extend(path_obj.glob("*.cc"))
            files.extend(path_obj.glob("*.h"))
        
        print(f"📁 Tìm thấy {len(files)} file trong {directory}")
        
        for i, filepath in enumerate(sorted(files), 1):
            print(f"[{i}/{len(files)}] Xử lý: {filepath.relative_to(path_obj.parent)}", end="... ")
            success, message = self.process_file(str(filepath), backup=backup)
            print(f"{'✓' if success else '✗'} {message}")
            results.append((str(filepath), success, message))
        
        return results


def main():
    """Hàm chính"""
    print("=" * 60)
    print("🌐 Script Dịch Comments Tự Động")
    print("=" * 60)
    print()
    
    # Cài đặt
    if not DEEP_TRANSLATOR_AVAILABLE and not GOOGLE_TRANSLATE_AVAILABLE:
        print("❌ Lỗi: Không có thư viện dịch nào khả dụng!")
        print("\n💡 Cài đặt một trong các thư viện sau:")
        print("  1. pip install deep-translator  (Khuyên dùng, không cần API key)")
        print("  2. pip install google-cloud-translate")
        return
    
    # Chọn thư mục
    project_root = os.path.dirname(os.path.abspath(__file__))
    main_dir = os.path.join(project_root, "main")
    
    if not os.path.isdir(main_dir):
        print(f"❌ Lỗi: Thư mục {main_dir} không tồn tại!")
        return
    
    print(f"📂 Thư mục dự án: {project_root}")
    print(f"📂 Thư mục chính: {main_dir}")
    print()
    
    # Yêu cầu xác nhận
    response = input("🔍 Bắt đầu dịch tất cả comments? (y/n): ").strip().lower()
    if response != 'y':
        print("❌ Đã hủy")
        return
    
    print()
    
    # Tạo dịch giả
    translator = CommentTranslator(use_deep_translator=DEEP_TRANSLATOR_AVAILABLE)
    
    # Xử lý thư mục
    results = translator.process_directory(main_dir, recursive=True, backup=True)
    
    # Lưu bộ nhớ dịch
    translator.save_cache()
    
    # Thống kê
    print()
    print("=" * 60)
    print("📊 Thống Kê Kết Quả")
    print("=" * 60)
    success_count = sum(1 for _, success, _ in results if success)
    print(f"✓ Thành công: {success_count}/{len(results)}")
    print(f"✗ Thất bại: {len(results) - success_count}/{len(results)}")
    print()
    
    # Hiển thị các file thất bại
    failed_files = [(f, m) for f, s, m in results if not s]
    if failed_files:
        print("⚠️  Các file thất bại:")
        for filepath, message in failed_files:
            print(f"  - {filepath}: {message}")
        print()
    
    print("✅ Hoàn tất!")
    print()
    print("💡 Mẹo:")
    print("  - Các file gốc được sao lưu với phần mở rộng .bak")
    print("  - Bộ nhớ dịch được lưu trong translation_cache.json")
    print("  - Chạy lại script để dịch thêm các file mới")


if __name__ == "__main__":
    main()
