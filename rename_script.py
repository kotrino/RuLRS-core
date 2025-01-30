import os
import re
from pathlib import Path

def process_file(file_path):
    # Список расширений файлов для обработки
    allowed_extensions = {'.cpp', '.h', '.hpp', '.ino', '.txt', '.md', '.html', '.js', '.py', '.json', '.ini'}
    
    if file_path.suffix not in allowed_extensions:
        return

    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            content = file.read()

        # Словарь замен
        replacements = {
            'RuLRS': 'RuLRS',
            'RULRS_': 'RULRS_',
            'rulrs-': 'rulrs-',
            'RULRS/': 'RULRS/',
            'rulrs/': 'rulrs/',
            'rulrs': 'rulrs',
            'RULRS': 'RULRS'
        }

        new_content = content
        for old, new in replacements.items():
            new_content = new_content.replace(old, new)

        if new_content != content:
            with open(file_path, 'w', encoding='utf-8') as file:
                file.write(new_content)
            print(f'Обновлен файл: {file_path}')

    except Exception as e:
        print(f'Ошибка при обработке {file_path}: {str(e)}')

def rename_directories():
    # Переименование директорий
    dir_replacements = {
        'RULRS': 'RULRS',
        'RuLRS': 'RuLRS',
        'rulrs': 'rulrs'
    }
    
    for root, dirs, files in os.walk('.', topdown=False):
        for dir_name in dirs:
            old_path = Path(root) / dir_name
            for old, new in dir_replacements.items():
                if old.lower() in dir_name.lower():
                    new_path = Path(root) / dir_name.replace(old, new)
                    try:
                        old_path.rename(new_path)
                        print(f'Переименована директория: {old_path} -> {new_path}')
                    except Exception as e:
                        print(f'Ошибка при переименовании {old_path}: {str(e)}')

def main():
    # Обработка всех файлов
    for path in Path('.').rglob('*'):
        if path.is_file():
            process_file(path)
    
    # Переименование директорий
    rename_directories()

if __name__ == '__main__':
    main() 