import os

def scan_folder_structure_and_code(root_folder, extensions=(".h", ".cpp", ".txt",".vert",".frag")):
    folder_structure = []
    code_files = []

    # Liste der zu ignorierenden Ordner
    ignore_dirs = {'node_modules', 'AntiQua-Node', 'x64','libs','.git' }

    for dirpath, dirnames, filenames in os.walk(root_folder):
        # Entferne die zu ignorierenden Ordner aus den Verzeichnissen
        dirnames[:] = [d for d in dirnames if d not in ignore_dirs]

        # Ebene des aktuellen Ordners
        level = dirpath.replace(root_folder, '').count(os.sep)
        indent = '│   ' * (level - 1) + '├── ' if level > 0 else ''
        folder_structure.append(f"{indent}{os.path.basename(dirpath)}")

        # Verarbeite die Dateien im aktuellen Ordner
        sub_indent = '│   ' * level + '├── '
        for filename in filenames:
            # Ignoriere `package-lock.json`
            if filename == "package-lock.json":
                continue

            # Prüfe, ob die Datei die gewünschte Endung hat
            if filename.endswith(extensions):
                file_path = os.path.join(dirpath, filename)
                code_files.append(file_path)

            # Speichere den Dateinamen in der Struktur
            folder_structure.append(f"{sub_indent}{filename}")

    return folder_structure, code_files

def save_structure_and_code_to_file(folder_structure, code_files, output_file):
    with open(output_file, 'w', encoding='utf-8') as f:
        # Speichere die Ordnerstruktur
        f.write("# Ordnerstruktur\n\n")
        for line in folder_structure:
            f.write(f"{line}\n")
        
        f.write("\n# Inhalte der Code-Dateien\n\n")

        # Speichere den Inhalt der Code-Dateien
        for file_path in code_files:
            file_name = os.path.basename(file_path)
            f.write(f"# {file_name} - {file_path}\n")

            try:
                # Lies den Inhalt der Datei
                with open(file_path, 'r', encoding='utf-8') as code_file:
                    code_content = code_file.read()
                    f.write(f"{code_content}\n\n")
            except Exception as e:
                # Fehler beim Lesen der Datei protokollieren
                f.write(f"# Fehler beim Lesen der Datei: {e}\n\n")

def main():
    choice = input("Möchten Sie den aktuellen Ordner scannen (J/N)? ").strip().lower()
    if choice == 'j':
        root_folder = os.getcwd()
    else:
        root_folder = input("Geben Sie den Pfad zum Ordner ein: ")

    if not os.path.isdir(root_folder):
        print("Der eingegebene Pfad ist kein gültiger Ordner.")
        return

    output_file = "CodeOfFolder.txt"
    folder_structure, code_files = scan_folder_structure_and_code(root_folder)
    save_structure_and_code_to_file(folder_structure, code_files, output_file)

    print(f"Ordnerstruktur und Code-Inhalte erfolgreich in '{output_file}' gespeichert.")

if __name__ == "__main__":
    main()
