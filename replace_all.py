import os
from pathlib import Path

def replace_in_file(file_path):
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            content = file.read()
        
        replacements = {
            'RuLRS': 'RuLRS',
            'RULRS_': 'RULRS_',
            'rulrs-': 'rulrs-',
            'RULRS/': 'RULRS/',
            'rulrs/': 'rulrs/',
            'rulrs': 'rulrs',
            'RULRS': 'RULRS',
            '/RULRS': '/RULRS',
            '_RULRS': '_RULRS',
            'RULRS.': 'RULRS.',
            'rulrs_': 'rulrs_',
            'rulrs.': 'rulrs.',
            '"RULRS"': '"RULRS"',
            "'RULRS'": "'RULRS'",
            'rulrsmock': 'rulrsmock',
            'rulrsV': 'rulrsV',
            'RULRS ': 'RULRS ',
            ' RULRS': ' RULRS',
            '(RULRS)': '(RULRS)',
            '[RULRS]': '[RULRS]'
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

def rename_files_and_dirs():
    renames = {
        'elrs': 'rulrs',
        'RULRS': 'RULRS',
        'RuLRS': 'RuLRS'
    }
    
    # Переименование файлов
    for root, dirs, files in os.walk('.', topdown=False):
        # Переименование файлов
        for file in files:
            old_path = Path(root) / file
            new_name = file
            for old, new in renames.items():
                if old.lower() in file.lower():
                    new_name = file.replace(old, new).replace(old.lower(), new.lower())
            if new_name != file:
                try:
                    new_path = Path(root) / new_name
                    old_path.rename(new_path)
                    print(f'Переименован файл: {old_path} -> {new_path}')
                except Exception as e:
                    print(f'Ошибка при переименовании {old_path}: {str(e)}')
        
        # Переименование директорий
        for dir_name in dirs:
            old_path = Path(root) / dir_name
            new_name = dir_name
            for old, new in renames.items():
                if old.lower() in dir_name.lower():
                    new_name = dir_name.replace(old, new).replace(old.lower(), new.lower())
            if new_name != dir_name:
                try:
                    new_path = Path(root) / new_name
                    old_path.rename(new_path)
                    print(f'Переименована директория: {old_path} -> {new_path}')
                except Exception as e:
                    print(f'Ошибка при переименовании директории {old_path}: {str(e)}')

def process_all_files():
    extensions = {'.cpp', '.h', '.hpp', '.ino', '.txt', '.md', '.html', '.js', '.py', '.lua', '.ini'}
    
    for path in Path('.').rglob('*'):
        if path.is_file() and path.suffix in extensions:
            replace_in_file(path)

def main():
    print("Начинаем замену...")
    # Сначала переименовываем файлы и директории
    rename_files_and_dirs()
    # Затем заменяем содержимое
    process_all_files()
    print("Замена завершена!")

if __name__ == '__main__':
    main() 