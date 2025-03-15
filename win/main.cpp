#include <iostream>
#include <limits>
#include <fileapi.h>
#include <vector>
#include <windows.h>
#include <algorithm>
#include <fcntl.h>
#include <io.h>

#define MENU_HELP 0
#define MENU_OUT_DRIVES 1
#define MENU_CREATE_DELETE_DIRECTORY 2
#define MENU_MOVE_FILE 3
#define MENU_CHANGE_ATTRIBUTES 4
#define MENU_EXIT 5

#define CREATE_DIR 1
#define DELETE_DIR 2
#define NAV_TO_DIR 3
#define GO_TO_MENU 4

#define NAVIGATE_DEEPER 1
#define SELECT_FILE 2
#define RESTART_SELECTION 3

#define ATTR_READONLY 1
#define ATTR_HIDDEN 2
#define ATTR_SYSTEM 3
#define ATTR_ARCHIVE 4
#define APPLY_CHANGES 5

#define DEFAULT_ERROR L"Ошибка! Приложение требует перезагрузки\n"

using namespace std;

void help();

void menu();

int get_Valid_Input(const vector<int>& validOptions);

wstring get_Valid_String_Input();

int users_button();

wchar_t which_Driver(const vector<wchar_t>& vector_Drives);

void output_Drivers_Type_Information(wchar_t driver_Letter);

void output_Drivers_Volume_Free_Information(wchar_t driver_Letter);

void output_Drives();

vector<wstring> List_Files_And_Directories(const wstring& directory);

void create_Directory(const wstring& path);

bool directory_Has_Files(const wstring& directory);

void delete_All_Files_And_Subdirectories(const wstring& directory);

void log_Delete_Error();

void delete_Directory(const wstring& path);

void create_File(const wstring& directory);

void navigate_Directory(wstring& current_Directory);

void create_Delete_Directory();

bool is_Directory(wstring& path);

bool is_File(wstring& path);

bool file_Exists(wstring& path);

void move_The_File();

wstring get_Disk_Directory();

wstring select_File(wstring directory);

wstring select_Destination(wstring current_Directory);

void move_The_File();

void change_File_Attributes();

void display_File_Info(wstring& file_Path);

void set_File_Attribute(wstring& file_Path, DWORD attribute, bool enable);

int main() {
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);

    menu();
    return 0;
}

void help() {
    wcout << L"**************************************************\n";
    wcout << L"*                ПОМОЩЬ ФАЙЛОВОГО МЕНЕДЖЕРА      *\n";
    wcout << L"**************************************************\n\n";

    wcout << L"Добро пожаловать в Файловый Менеджер! Эта программа позволяет:\n";
    wcout << L"- Просматривать доступные диски и их свойства\n";
    wcout << L"- Создавать и удалять директории + создавать файлы в новых директориях\n";
    wcout << L"- Перемещать файлы между директориями\n";
    wcout << L"- Изменять атрибуты файлов (Только для Чтения, Скрытый, Системный, Архивный)\n\n";

    wcout << L"**************************************************\n";
    wcout << L"*                  ГЛАВНОЕ МЕНЮ                  *\n";
    wcout << L"**************************************************\n";
    wcout << MENU_HELP << L" - Помощь: Просмотр текущего руководства\n";
    wcout << MENU_OUT_DRIVES << L" - Список всех дисков: Просмотр информации о системных дисках\n";
    wcout << MENU_CREATE_DELETE_DIRECTORY << L" - Управление директориями: Создание или удаление папок + создавание файлов в новых директориях\n";
    wcout << MENU_MOVE_FILE << L" - Перемещение файлов: Выбор и перемещение файлов между папками\n";
    wcout << MENU_CHANGE_ATTRIBUTES << L" - Изменение атрибутов файлов: Настройка свойств файла\n";
    wcout << MENU_EXIT << L" - Выход: Закрытие программы\n\n";

    wcout << L"**************************************************\n";
    wcout << L"*           УПРАВЛЕНИЕ ДИРЕКТОРИЯМИ              *\n";
    wcout << L"**************************************************\n";
    wcout << CREATE_DIR << L" - Создать директорию: Введите имя и создайте папку + создать файлы в новых директориях\n";
    wcout << DELETE_DIR << L" - Удалить директорию: Выберите папку для удаления\n";
    wcout << NAV_TO_DIR << L" - Навигация по директории: Просмотр содержимого\n";
    wcout << GO_TO_MENU << L" - Вернуться в главное меню\n\n";

    wcout << L"**************************************************\n";
    wcout << L"*             ПЕРЕМЕЩЕНИЕ ФАЙЛОВ                 *\n";
    wcout << L"**************************************************\n";
    wcout << L"1 - Выберите файл для перемещения\n";
    wcout << L"2 - Выберите папку назначения\n";
    wcout << L"3 - Подтвердите перемещение\n\n";

    wcout << L"**************************************************\n";
    wcout << L"*        ИЗМЕНЕНИЕ АТРИБУТОВ ФАЙЛА               *\n";
    wcout << L"**************************************************\n";
    wcout << L"1 - Выберите файл для изменения\n";
    wcout << L"2 - Выберите атрибут для изменения:\n";
    wcout << L"   - " << ATTR_READONLY << L" Только для чтения\n";
    wcout << L"   - " << ATTR_HIDDEN << L" Скрытый\n";
    wcout << L"   - " << ATTR_SYSTEM << L" Системный\n";
    wcout << L"   - " << ATTR_ARCHIVE << L" Архивный\n";
    wcout << L"3 - Включить - 1 или отключить - 0 атрибут\n";
    wcout << L"4 - Применить изменения и выйти\n\n";
}

void menu() {
    int from_user;

    do {
        wcout << L"Выберите номер из меню\n\n";
        from_user = users_button();

        switch(from_user) {
            case MENU_HELP:
                help();
                break;
            case MENU_OUT_DRIVES:
                output_Drives();
                break;
            case MENU_CREATE_DELETE_DIRECTORY:
                create_Delete_Directory();
                break;
            case MENU_MOVE_FILE:
                move_The_File();
                break;
            case MENU_CHANGE_ATTRIBUTES:
                change_File_Attributes();
                break;
            case MENU_EXIT:
                wcout << L"Выход...\n";
                return;
            default:
                wcout << DEFAULT_ERROR;
        }
    } while (from_user != MENU_EXIT);
}

int get_Valid_Input(const vector<int>& validOptions) {
    int input;
    while (true) {
        if (wcin >> input && find(validOptions.begin(), validOptions.end(), input) != validOptions.end()) {
            return input;
        }
        wcout << L"Некорректный ввод, попробуйте снова!\n";
        wcin.clear();
        wcin.ignore(numeric_limits<streamsize>::max(), L'\n');
    }
}

wstring get_Valid_String_Input() {
    wstring input;
    wstring invalid_Chars;

    invalid_Chars = L"\\/:*?\"<>|";

    while (true) {
        wcin >> input;

        if (any_of(input.begin(), input.end(), ::iswalpha) || input.find_first_of(invalid_Chars) != wstring::npos) {
            return input;
        }

        wcout << L"Некорректный ввод, попробуйте снова!\n";
    }
}

int users_button() {
    wcout << MENU_HELP << L" - Помощь\n";
    wcout << MENU_OUT_DRIVES << L" - Список всех дисков\n";
    wcout << MENU_CREATE_DELETE_DIRECTORY << L" - Создать новую директорию или удалить существующую\n";
    wcout << MENU_MOVE_FILE << L" - Переместить файлы\n";
    wcout << MENU_CHANGE_ATTRIBUTES << L" - Изменить атрибуты файла\n";
    wcout << MENU_EXIT << L" - Выход\n";

    return get_Valid_Input({MENU_HELP, MENU_OUT_DRIVES, MENU_CREATE_DELETE_DIRECTORY, MENU_MOVE_FILE, MENU_CHANGE_ATTRIBUTES, MENU_EXIT});
}

wchar_t which_Driver(const vector<wchar_t>& vector_Drives) {
    int drive_Number;
    vector<int> drives;

    while (true) {
        wcout << L"Список всех дисков в системе:\n";
        for (int i = 0; i < vector_Drives.size(); i++) {
            wcout << i + 1 << L" - " << vector_Drives[i] << L":\\" << L"\n";
            drives.push_back(i + 1);
        }

        wcout << L"\nВыберите номер диска (1, 2, ...):\n";
        drive_Number = get_Valid_Input(drives);
        return vector_Drives[drive_Number - 1];
    }
}

void output_Drivers_Type_Information(wchar_t driver_Letter) {
    wstring root_Path;
    UINT type_Of_Driver;

    root_Path = wstring(1, driver_Letter) + L":\\";
    type_Of_Driver = GetDriveTypeW(root_Path.c_str());

    switch (type_Of_Driver) {
        case DRIVE_REMOVABLE:
            wcout << L"Диск имеет сменный носитель (USB, дискета и т. д.)\n";
            break;
        case DRIVE_FIXED:
            wcout << L"Диск является жестким диском (HDD, SSD)\n";
            break;
        case DRIVE_REMOTE:
            wcout << L"Диск является сетевым диском\n";
            break;
        case DRIVE_CDROM:
            wcout << L"Диск является CD/DVD приводом\n";
            break;
        case DRIVE_RAMDISK:
            wcout << L"Диск является RAM-диском\n";
            break;
        default:
            wcout << DEFAULT_ERROR;
    }
}

void output_Drivers_Volume_Free_Information(wchar_t driver_Letter) {
    wchar_t file_System_Name[MAX_PATH] = {0};
    DWORD volume_Serial_Number, maximum_Component_Length, file_System_Flags;
    ULARGE_INTEGER free_Bytes_Available, total_Number_Of_Bytes, total_Number_Of_Free_Bytes;
    wstring root_Path;

    root_Path = wstring(1, driver_Letter) + L":\\";

    if (GetVolumeInformationW(root_Path.c_str(), NULL, MAX_PATH, &volume_Serial_Number,
                              &maximum_Component_Length, &file_System_Flags, file_System_Name, MAX_PATH)) {
        wcout << L"\nИнформация о томе:\n";
        wcout << L"Файловая система: [" << file_System_Name << L"]\n";
        wcout << L"Серийный номер: [" << volume_Serial_Number << L"]\n";
        wcout << L"Максимальная длина имени компонента: [" << maximum_Component_Length << L"]\n";
        wcout << L"Расшифровка флагов файловой системы:\n";

        if (file_System_Flags & FILE_CASE_SENSITIVE_SEARCH)
            wcout << L"  - Поддерживается чувствительный к регистру поиск\n";
        if (file_System_Flags & FILE_CASE_PRESERVED_NAMES)
            wcout << L"  - Сохраняются регистры имен файлов\n";
        if (file_System_Flags & FILE_UNICODE_ON_DISK)
            wcout << L"  - Поддерживаются имена файлов в Unicode\n";
        if (file_System_Flags & FILE_PERSISTENT_ACLS)
            wcout << L"  - Поддерживаются ACL (списки управления доступом)\n";
        if (file_System_Flags & FILE_FILE_COMPRESSION)
            wcout << L"  - Поддерживается сжатие файлов\n";
        if (file_System_Flags & FILE_VOLUME_IS_COMPRESSED)
            wcout << L"  - Том сжат целиком\n";
        if (file_System_Flags & FILE_SUPPORTS_ENCRYPTION)
            wcout << L"  - Поддерживается шифрование файлов\n";
        if (file_System_Flags & FILE_SUPPORTS_SPARSE_FILES)
            wcout << L"  - Поддерживаются разреженные файлы\n";
        if (file_System_Flags & FILE_SUPPORTS_REPARSE_POINTS)
            wcout << L"  - Поддерживаются точки повторной обработки (Reparse Points)\n";
        if (file_System_Flags & FILE_SUPPORTS_REMOTE_STORAGE)
            wcout << L"  - Поддерживается удаленное хранение\n";
        if (file_System_Flags & FILE_SUPPORTS_TRANSACTIONS)
            wcout << L"  - Поддерживаются транзакционные файловые операции\n";


        if (GetDiskFreeSpaceExW(root_Path.c_str(), &free_Bytes_Available, &total_Number_Of_Bytes, &total_Number_Of_Free_Bytes)) {
            wcout << L"\nИнформация о свободном пространстве на диске:\n";
            wcout << L"Доступно свободных байтов: [" << free_Bytes_Available.QuadPart / (1024 * 1024) << L"] MB\n";
            wcout << L"Общий объем байтов: [" << total_Number_Of_Bytes.QuadPart / (1024 * 1024) << L"] MB\n";
            wcout << L"Всего свободных байтов: [" << total_Number_Of_Free_Bytes.QuadPart / (1024 * 1024) << L"] MB\n";
        } else {
            wcout << L"Ошибка получения информации о свободном пространстве\n";
        }
    } else {
        wcout << L"Ошибка получения информации о томе\n";
    }
}

void output_Drives() {
    DWORD all_Drives;
    vector<wchar_t> vector_Drives;
    wchar_t driver_Letter;

    all_Drives = GetLogicalDrives();

    for (int i = 0; i < 26; ++i) {
        if (all_Drives & (1 << i)) {
            vector_Drives.push_back(L'A' + i);
        }
    }

    driver_Letter = which_Driver(vector_Drives);
    output_Drivers_Type_Information(driver_Letter);
    output_Drivers_Volume_Free_Information(driver_Letter);
}

vector<wstring> List_Files_And_Directories(const wstring& directory) {
    vector<wstring> entries;
    WIN32_FIND_DATAW find_File_Data;
    HANDLE find;
    wstring file_Or_Dir_Name;
    int index;

    index = 1;

    wcout << L"Содержимое папки: " << directory << L"\n";

    find = FindFirstFileW((directory + L"\\*").c_str(), &find_File_Data);

    do {
        file_Or_Dir_Name = find_File_Data.cFileName;

        if (file_Or_Dir_Name != L"." && file_Or_Dir_Name != L"..") {
            entries.push_back(file_Or_Dir_Name);
            wcout << index++ << L". "
                  << ((find_File_Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? L"[ДИРЕКТОРИЯ] " : L"[ФАЙЛ] ")
                  << file_Or_Dir_Name << L"\n";
        }
    } while (FindNextFileW(find, &find_File_Data) != 0);

    FindClose(find);

    if (entries.empty()) {
        wcout << L"Эта директория пуста\n";
    }

    return entries;
}

void create_Directory(const wstring& path) {
    if (CreateDirectoryW(path.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
        wcout << L"Директория успешно создана\n";
    } else {
        wcout << L"Не удалось создать директорию\n";
    }
}

bool directory_Has_Files(const wstring& directory) {
    wstring search_Path;
    WIN32_FIND_DATAW find_Data;
    HANDLE find;
    wstring name;

    search_Path = directory + L"\\*";

    find = FindFirstFileW(search_Path.c_str(), &find_Data);

    if (find != INVALID_HANDLE_VALUE) {
        do {
            name = find_Data.cFileName;
            if (name != L"." && name != L"..") {
                FindClose(find);
                return true;
            }
        } while (FindNextFileW(find, &find_Data));

        FindClose(find);
    }
    return false;
}

void delete_All_Files_And_Subdirectories(const wstring& directory) {
    WIN32_FIND_DATAW find_Data;
    vector<wstring> directoriesToDelete;
    HANDLE find;

    find = FindFirstFileW((directory + L"\\*").c_str(), &find_Data);

    do {
        if (wcscmp(find_Data.cFileName, L".") == 0 || wcscmp(find_Data.cFileName, L"..") == 0) {
            continue;
        }

        if (find_Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            delete_All_Files_And_Subdirectories(directory + L"\\" + find_Data.cFileName);
            directoriesToDelete.push_back(directory + L"\\" + find_Data.cFileName);
        } else {
            if (!DeleteFileW((directory + L"\\" + find_Data.cFileName).c_str())) {
                wcout << L"Не удалось удалить файл\n";
            }
        }
    } while (FindNextFileW(find, &find_Data) != 0);

    FindClose(find);
}


void log_Delete_Error() {
    switch (GetLastError()) {
        case ERROR_FILE_NOT_FOUND:
            wcout << L"Ошибка! Директория не найдена\n";
            break;
        case ERROR_ACCESS_DENIED:
            wcout << L"Ошибка! Доступ запрещен\n";
            break;
        case ERROR_DIR_NOT_EMPTY:
            wcout << L"Ошибка! Директория не пуста\n";
            break;
        default:
            wcout << DEFAULT_ERROR;
    }
}

void delete_Directory(const wstring& path) {
    int choice;

    if (directory_Has_Files(path)) {
        wcout << L"Внутри есть файлы, вы уверены, что хотите удалить эту директорию?\n";
        wcout << TRUE << L" - Да\n";
        wcout << FALSE << L" - Нет\n";

        choice = get_Valid_Input({TRUE, FALSE});

        switch (choice) {
            case TRUE:
                delete_All_Files_And_Subdirectories(path);
                break;
            case FALSE:
                wcout << L"Удаление отменено\n";
                return;
            default:
                wcout << DEFAULT_ERROR;
        }
    }

    if (RemoveDirectoryW(path.c_str())) {
        wcout << L"Директория успешно удалена\n";
    } else {
        wcout << L"Не удалось удалить директорию\n";
        log_Delete_Error();
    }
}

void create_File(const wstring& directory) {
    wstring name_Of_File;
    HANDLE file;

    wcout << L"Введите имя файла, который хотите создать:\n";
    name_Of_File = get_Valid_String_Input();

    file = CreateFileW((directory + L"\\" + name_Of_File + L".txt").c_str(), GENERIC_WRITE, FILE_SHARE_DELETE, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);

    wcout << L"Файл успешно создан\n";
    CloseHandle(file);
}

void navigate_Directory(wstring& current_Directory) {
    vector<wstring> entries;
    vector<int> arr_Of_All_Numbers;
    wstring new_Dir_Name;
    int bool_create_File, choice, index;

    do {
        entries = List_Files_And_Directories(current_Directory);

        for (int i = 0; i < entries.size(); i++) {
            arr_Of_All_Numbers.push_back(i + 1);
        }

        wcout << L"\nОпции:\n";
        wcout << CREATE_DIR << L" - Создать директорию\n";
        wcout << DELETE_DIR << L" - Удалить директорию\n";
        wcout << NAV_TO_DIR << L" - Перейти в директорию\n";
        wcout << GO_TO_MENU << L" - Вернуться в меню\n";

        choice = get_Valid_Input({CREATE_DIR, DELETE_DIR, NAV_TO_DIR, GO_TO_MENU});

        switch (choice) {
            case CREATE_DIR: {
                wcout << L"Введите имя новой директории:\n";
                new_Dir_Name = get_Valid_String_Input();
                create_Directory(current_Directory + new_Dir_Name + L"\\");

                wcout << L"Хотите создать файл в этой директории?\n";
                wcout << TRUE << L" - Да\n";
                wcout << FALSE << L" - Нет\n";

                bool_create_File = get_Valid_Input({TRUE, FALSE});

                switch (bool_create_File) {
                    case TRUE:
                        create_File(current_Directory + new_Dir_Name + L"\\");
                        break;
                    case FALSE:
                        break;
                    default:
                        wcout << DEFAULT_ERROR;
                }
                break;
            }
            case DELETE_DIR: {
                if (entries.empty()) {
                    wcout << L"Нет доступных директорий для удаления\n";
                    break;
                }

                wcout << L"Введите номер директории для удаления\n";
                index = get_Valid_Input(arr_Of_All_Numbers) - 1;
                delete_Directory(current_Directory + entries[index] + L"\\");
                break;
            }
            case NAV_TO_DIR: {
                if (entries.empty()) {
                    wcout << L"Нет доступных директорий для перехода\n";
                    break;
                }

                wcout << L"Введите номер директории для перехода:\n";
                index = get_Valid_Input(arr_Of_All_Numbers) - 1;
                current_Directory += entries[index] + L"\\";
                break;
            }
            case GO_TO_MENU: {
                menu();
                break;
            }
            default:
                wcout << DEFAULT_ERROR;
        }
    } while (choice != GO_TO_MENU);
}

wstring get_Disk_Directory() {
    DWORD all_Drives;
    vector<wchar_t> vector_Drives;
    wchar_t driver_Letter;

    all_Drives = GetLogicalDrives();

    for (int i = 0; i < 26; ++i) {
        if (all_Drives & (1 << i)) {
            vector_Drives.push_back(L'A' + i);
        }
    }

    wcout << L"Выберите диск\n";
    driver_Letter = which_Driver(vector_Drives);

    wstring currentDirectory(1, driver_Letter);
    currentDirectory += L":\\";

    return currentDirectory;
}

void create_Delete_Directory() {
    wstring current_Directory;

    current_Directory = get_Disk_Directory();
    navigate_Directory(current_Directory);
}

bool file_Exists(wstring& path) {
    DWORD attributes;

    attributes = GetFileAttributesW(path.c_str());
    return (attributes != INVALID_FILE_ATTRIBUTES);
}

void move_The_File() {
    wstring tmp_Directory, current_Directory, selected_File, destination_Path;

    tmp_Directory = get_Disk_Directory();
    current_Directory = tmp_Directory;
    selected_File = select_File(current_Directory);
    destination_Path = select_Destination(current_Directory) + selected_File.substr(selected_File.find_last_of(L"\\") + 1);

    if (file_Exists(destination_Path)) {
        wcout << L"Ошибка! Файл с таким же именем уже существует в месте назначения\n";
        return;
    }

    if (MoveFileExW(selected_File.c_str(), destination_Path.c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING)) {
        wcout << L"Файл успешно перемещен\n";
    } else {
        wcout << L"Ошибка перемещения!\n";
    }
}

wstring select_File(wstring directory) {
    vector<wstring> entries;
    vector<int> indices;
    wstring selected_Path;
    int choice, index;

    while (true) {
        entries = List_Files_And_Directories(directory);
        indices.clear();

        for (int i = 0; i < entries.size(); i++) {
            indices.push_back(i + 1);
        }

        wcout << L"\nВыберите действие:\n";
        wcout << NAVIGATE_DEEPER << L" - Перейти в директорию\n";
        wcout << SELECT_FILE << L" - Выбрать файл\n";
        wcout << RESTART_SELECTION << L" - Перезапустить выбор\n";

        choice = get_Valid_Input({NAVIGATE_DEEPER, SELECT_FILE, RESTART_SELECTION});

        switch (choice) {
            case NAVIGATE_DEEPER: {
                wcout << L"Введите номер директории для перехода:\n";
                index = get_Valid_Input(indices) - 1;
                selected_Path = directory + L"\\" + entries[index];

                if (is_Directory(selected_Path)) {
                    return select_File(selected_Path);
                }
                wcout << L"Это не директория\n";
                break;
            }
            case SELECT_FILE: {
                wcout << L"Введите номер файла для выбора:\n";
                index = get_Valid_Input(indices) - 1;
                selected_Path = directory + L"\\" + entries[index];

                if (is_File(selected_Path)) {
                    return selected_Path;
                }
                wcout << L"Это директория, а не файл\n";
                break;
            }
            case RESTART_SELECTION:
                wcout << L"Перезапуск выбора...\n";
                directory = get_Disk_Directory();
                break;
            default:
                wcout << DEFAULT_ERROR;
        }
    }
}

bool is_Directory(wstring& path) {
    DWORD attributes;

    attributes = GetFileAttributesW(path.c_str());
    return (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY));
}

bool is_File(wstring& path) {
    DWORD attributes;

    attributes = GetFileAttributesW(path.c_str());
    return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}

wstring select_Destination(wstring current_Directory) {
    vector<wstring> entries;
    vector<int> indices;

    entries = List_Files_And_Directories(current_Directory);

    for (int i = 0; i < entries.size(); i++) {
        indices.push_back(i + 1);
    }

    while (true) {
        wcout << L"\nВыберите действие:\n";
        wcout << NAVIGATE_DEEPER << L" - Перейти в директорию\n";
        wcout << SELECT_FILE << L" - Переместить файл сюда\n";
        wcout << RESTART_SELECTION << L" - Перезапустить выбор\n";

        int choice = get_Valid_Input({NAVIGATE_DEEPER, SELECT_FILE, RESTART_SELECTION});

        switch (choice) {
            case NAVIGATE_DEEPER:
                wcout << L"Введите номер директории для перехода:\n";
                {
                    int dirIndex = get_Valid_Input(indices) - 1;
                    wstring new_Dir = current_Directory + L"\\" + entries[dirIndex];
                    if (is_Directory(new_Dir)) {
                        return select_Destination(new_Dir);
                    }
                    wcout << L"Ошибка! Это не директория. Попробуйте снова.\n";
                }
                break;
            case SELECT_FILE:
                wcout << L"Введите номер директории, чтобы переместить файл сюда:\n";
                {
                    int dirIndex = get_Valid_Input(indices) - 1;
                    wstring destination_Dir = current_Directory + L"\\" + entries[dirIndex];
                    if (!is_Directory(destination_Dir)) {
                        wcout << L"Ошибка! Это не директория. Попробуйте снова.\n";
                        continue;
                    }
                    return destination_Dir + L"\\";
                }
            case RESTART_SELECTION:
                wcout << L"Перезапуск выбора. . .\n";
                return get_Disk_Directory();
            default:
                wcout << DEFAULT_ERROR;
        }
    }
}

void set_File_Attribute(wstring& file_Path, DWORD attribute, bool enable) {
    DWORD oldAttributes, attributes;

    attributes = GetFileAttributesW(file_Path.c_str());
    oldAttributes = attributes;

    if (enable) {
        attributes |= attribute;
    } else {
        attributes &= ~attribute;
    }

    if (!SetFileAttributesW(file_Path.c_str(), attributes)) {
        wcout << L"Ошибка! Невозможно установить атрибуты файла\n";
    } else {
        wcout << L"Атрибуты успешно изменены\n";
        if (oldAttributes != attributes) {
            wcout << L"Обновленный список атрибутов:\n";
            display_File_Info(file_Path);
        }
    }
}

void display_File_Info(wstring& file_Path) {
    DWORD attributes;
    HANDLE file_Handle;
    FILETIME creation_Time;
    SYSTEMTIME sys_Time;
    BY_HANDLE_FILE_INFORMATION file_Info;

    attributes = GetFileAttributesW(file_Path.c_str());

    wcout << L"Текущие атрибуты файла:\n";
    if (attributes & FILE_ATTRIBUTE_READONLY){
        wcout << L"- Только для чтения\n";
    }
    if (attributes & FILE_ATTRIBUTE_HIDDEN){
        wcout << L"- Скрытый\n";
    }
    if (attributes & FILE_ATTRIBUTE_SYSTEM){
        wcout << L"- Системный\n";
    }
    if (attributes & FILE_ATTRIBUTE_ARCHIVE){
        wcout << L"- Архивный\n";
    }
    if (attributes & FILE_ATTRIBUTE_DIRECTORY){
        wcout << L"- Директория\n";
    }
    if (attributes & FILE_ATTRIBUTE_TEMPORARY){
        wcout << L"- Временный файл\n";
    }
    if (attributes & FILE_ATTRIBUTE_NORMAL){
        wcout << L"- Обычный файл\n";
    }

    file_Handle = CreateFileW(file_Path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (GetFileInformationByHandle(file_Handle, &file_Info)) {
        wcout << L"Размер файла: " << file_Info.nFileSizeLow << L" байт\n";
    } else {
        wcout << L"Ошибка получения информации о файле!\n";
    }

    if (GetFileTime(file_Handle, &creation_Time, NULL, NULL) && FileTimeToSystemTime(&creation_Time, &sys_Time)) {
        wcout << L"Дата создания: " << sys_Time.wDay << L"/" << sys_Time.wMonth << L"/" << sys_Time.wYear << "\n";
    } else {
        wcout << L"Ошибка получения времени файла!\n";
    }

    CloseHandle(file_Handle);
}

void change_File_Attributes() {
    int choice;
    bool enable;
    wstring file_Path;

    file_Path = select_File(get_Disk_Directory());
    display_File_Info(file_Path);

    do {
        wcout << L"\nВыберите атрибут для изменения:\n";
        wcout << ATTR_READONLY << L" - Только для чтения\n";
        wcout << ATTR_HIDDEN << L" - Скрытый\n";
        wcout << ATTR_SYSTEM << L" - Системный\n";
        wcout << ATTR_ARCHIVE << L" - Архивный\n";
        wcout << APPLY_CHANGES << L" - Применить изменения и выйти\n";

        choice = get_Valid_Input({ATTR_READONLY, ATTR_HIDDEN, ATTR_SYSTEM, ATTR_ARCHIVE, APPLY_CHANGES});

        wcout << TRUE << L" - Включить\n";
        wcout << FALSE << L" - Отключить\n";
        enable = get_Valid_Input({TRUE, FALSE});

        switch (choice) {
            case ATTR_READONLY:
                set_File_Attribute(file_Path, FILE_ATTRIBUTE_READONLY, enable);
                break;
            case ATTR_HIDDEN:
                set_File_Attribute(file_Path, FILE_ATTRIBUTE_HIDDEN, enable);
                break;
            case ATTR_SYSTEM:
                set_File_Attribute(file_Path, FILE_ATTRIBUTE_SYSTEM, enable);
                break;
            case ATTR_ARCHIVE:
                set_File_Attribute(file_Path, FILE_ATTRIBUTE_ARCHIVE, enable);
                break;
            case APPLY_CHANGES:
                wcout << L"Выход. . .\n";
                break;
            default:
                wcout << DEFAULT_ERROR;
        }
    } while (choice != APPLY_CHANGES);
}