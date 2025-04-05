#include <iostream>
#include <vector>
#include <algorithm>
#include <fcntl.h>
#include <limits>
#include <bitset>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>

using namespace std;

const string SHARED_FILE_PATH = "/mnt/c/Users/aaa/Desktop/shared_memory_file.txt";
const wstring DEFAULT_ERROR = L"Ошибка! Приложение требует перезагрузки\n";
const size_t MAX_WCHARS = 1024/sizeof(wchar_t);

namespace Menu {
    constexpr int CREATE_PROJECT_FILE = 1;
    constexpr int PROJECT_FRAGMENT_FILE_TO_MEM = 2;
    constexpr int SET_DATA_TO_PROJ_FILE = 3;
    constexpr int EXIT = 4;
}

int get_Valid_Input(const vector<int>& validOptions);

int menu_Buttons();

int create_Project_File();

void* project_Fragment_File_To_Mem(int map_File);

void set_Data_To_Proj_File(void* mapping_File);

void menu();

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

int menu_Buttons(){
    int user_Choose;

    wcout << L"\n=== Серверное приложение ===\n\n";

    wcout << Menu::CREATE_PROJECT_FILE << L" - Создать проецируемый файл\n";
    wcout << Menu::PROJECT_FRAGMENT_FILE_TO_MEM << L" - Спроецировать фрагмент файла в память\n";
    wcout << Menu::SET_DATA_TO_PROJ_FILE << L" - Записать данные в файл с клавиатуры\n";
    wcout << Menu::EXIT << L" - Выход из программы + отмена проецирования\n";

    user_Choose = get_Valid_Input({Menu::CREATE_PROJECT_FILE, Menu::PROJECT_FRAGMENT_FILE_TO_MEM, Menu::SET_DATA_TO_PROJ_FILE, Menu::EXIT});
    return user_Choose;
}

int create_Project_File() {
    int file;

    file = open(SHARED_FILE_PATH.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if(file == -1) {
        wcout << L"Ошибка создания файла!";
        switch(errno) {
            case EACCES:
                wcout << L"Ошибка: Нет прав доступа к файлу или директории\n";
                break;
            case ENFILE:
                wcout << L"Ошибка: Достигнут системный лимит открытых файлов\n";
                break;
            case ENOENT:
                wcout << L"Ошибка: Директория не существует\n";
                break;
            case ENOSPC:
                wcout << L"Ошибка: Нет свободного места на устройстве\n";
                break;
            default:
                wcout << DEFAULT_ERROR;
        }
        return -1;
    }

    if (ftruncate(file, 1024) == -1) {
        wcout << L"Ошибка при установке размера файла!\n";
        close(file);
        return -1;
    }

    wcout << L"Файл успешно создан\n";
    return file;
}

void* project_Fragment_File_To_Mem(int map_File) {
    void* mapping_File;

    mapping_File = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, map_File, 0);
    if (mapping_File == MAP_FAILED) {
        wcout << L"Ошибка отображения файла!\n";
        return nullptr;
    }
    return mapping_File;
}

void set_Data_To_Proj_File(void* mapping_File) {
    wstring input;

    wcout << L"Введите текст:\n";
    wcin.ignore(numeric_limits<streamsize>::max(), L'\n');
    getline(wcin, input);

    wcsncpy(static_cast<wchar_t*>(mapping_File), input.c_str(), MAX_WCHARS);
    static_cast<wchar_t*>(mapping_File)[MAX_WCHARS-1] = L'\0';
    wcout << L"Данные успешно записаны в файл!\n";
}

void menu() {
    int user_Choose;
    int file;
    void* mapping_File;

    file = -1;
    mapping_File = nullptr;

    do {
        user_Choose = menu_Buttons();

        switch(user_Choose) {
            case Menu::CREATE_PROJECT_FILE:
                if (file != -1) {
                    close(file);
                    file = -1;
                }
                file = create_Project_File();
                break;
            case Menu::PROJECT_FRAGMENT_FILE_TO_MEM:
                if (file == -1) {
                    wcout << L"Сначала создайте файл!\n";
                    break;
                }
                if (mapping_File) {
                    munmap(mapping_File, 1024);
                }
                mapping_File = project_Fragment_File_To_Mem(file);
                break;
            case Menu::SET_DATA_TO_PROJ_FILE:
                if (!mapping_File) {
                    wcout << L"Сначала спроецируйте файл в память!\n";
                    break;
                }
                set_Data_To_Proj_File(mapping_File);
                break;
            case Menu::EXIT:
                wcout << L"\nЗавершение работы...\n";
                if (mapping_File && mapping_File != MAP_FAILED) {
                    munmap(mapping_File, 1024);
                }
                if (file != -1) {
                    close(file);
                    if (unlink(SHARED_FILE_PATH.c_str()) == -1) {
                        wcout << L"Ошибка удаления файла: ";
                        switch(errno) {
                            case EACCES:
                                wcout << L"Ошибка: Нет прав на удаление файла\n";
                                break;
                            case EBUSY:
                                wcout << L"Ошибка: Файл используется другим процессом\n";
                                break;
                            case EIO:
                                wcout << L"Ошибка: Ошибка ввода-вывода при удалении\n";
                                break;
                            case ENOENT:
                                wcout << L"Ошибка: Файл не существует\n";
                                break;
                            case EPERM:
                                wcout << L"Ошибка: Операция запрещена\n";
                                break;
                            default:
                                wcout << DEFAULT_ERROR;
                        }
                    } else {
                        wcout << L"Файл успешно удален\n";
                    }
                }
                return;
            default:
                wcout << DEFAULT_ERROR;
        }
    } while (user_Choose != Menu::EXIT);
}

int main() {
    setlocale(LC_ALL, "");
    menu();
    return 0;
}