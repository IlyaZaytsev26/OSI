#include <iostream>
#include <fcntl.h>
#include <vector>
#include <limits>
#include <algorithm>
#include <sysinfoapi.h>
#include <unordered_map>
#include <bitset>
#include <memoryapi.h>
#include <errhandlingapi.h>
#include <winerror.h>

using namespace std;

namespace Menu {
    constexpr int HELP = 0;
    constexpr int GET_INFO_SYSTEM = 1;
    constexpr int GET_INFO_VIRTUAL_MEMORY = 2;
    constexpr int GET_INFO_MEMORY_AREA = 3;
    constexpr int SEPARATE_RESERVATION = 4;
    constexpr int SIMULTANE_RESERVATION = 5;
    constexpr int SET_DATA_TO_MEMORY_AREA = 6;
    constexpr int SET_ACCESS_PROTECTION = 7;
    constexpr int EXIT = 8;
}

namespace Protect_Type {
    constexpr int NOACCESS = 1;
    constexpr int READONLY = 2;
    constexpr int READWRITE = 3;
    constexpr int EXECUTE = 4;
    constexpr int EXECUTE_READ = 5;
    constexpr int EXECUTE_READWRITE = 6;
    constexpr int WRITECOPY = 7;
}

namespace Mode {
    constexpr int AUTO = 1;
    constexpr int SET = 2;
}

namespace Conversion {
    constexpr int TO_GB = 1073741824;
    constexpr int TO_KB = 1024;
}

constexpr int MAX_ADD = 6;

const wstring DEFAULT_ERROR = L"Ошибка! Приложение требует перезагрузки\n";

const unordered_map < DWORD, wstring > architecture = {
        {PROCESSOR_ARCHITECTURE_AMD64, L"64-битная архитектура x64 (AMD или Intel)\n"},
        {PROCESSOR_ARCHITECTURE_ARM, L"32-битная ARM-архитектура\n"},
        {PROCESSOR_ARCHITECTURE_ARM64, L"64-битная ARM-архитектура\n"},
        {PROCESSOR_ARCHITECTURE_IA64, L"Intel Itanium (IA-64)\n"},
        {PROCESSOR_ARCHITECTURE_INTEL, L"32-битная архитектура x86\n"},
        {PROCESSOR_ARCHITECTURE_UNKNOWN, L"Неизвестная архитектура\n"},
};

const unordered_map < DWORD, wstring > protect = {
        {PAGE_NOACCESS, L"Доступ запрещён\n"},
        {PAGE_READONLY, L"Только чтение\n"},
        {PAGE_READWRITE, L"Чтение и запись\n"},
        {PAGE_WRITECOPY, L"Копирование при записи\n"},
        {PAGE_EXECUTE, L"Только выполнение\n"},
        {PAGE_EXECUTE_READ, L"Выполнение и чтение\n"},
        {PAGE_EXECUTE_READWRITE, L"Полный доступ: выполнение, чтение, запись\n"},
        {PAGE_EXECUTE_WRITECOPY, L"Выполнение + копирование при записи\n"},
        {PAGE_GUARD, L"Сторожевая страница\n"},
        {PAGE_NOCACHE, L"Без кэширования\n"},
        {PAGE_WRITECOMBINE, L"Комбинированная запись\n"}
};

const unordered_map < int, DWORD > new_Protect_ = {
        {Protect_Type::NOACCESS, PAGE_NOACCESS},
        {Protect_Type::READONLY, PAGE_READONLY},
        {Protect_Type::READWRITE, PAGE_READWRITE},
        {Protect_Type::EXECUTE, PAGE_EXECUTE},
        {Protect_Type::EXECUTE_READ, PAGE_EXECUTE_READ},
        {Protect_Type::EXECUTE_READWRITE, PAGE_EXECUTE_READWRITE},
        {Protect_Type::WRITECOPY, PAGE_WRITECOPY},
};

void get_Free_Memory_Examples();

void get_Writeable_Memory_Examples();

void help();

int get_Valid_Input(const vector<int>& validOptions);

int menu_Buttons();

void get_Info_System();

void get_Info_Virtual_Memory();

void get_Info_Memory_Area();

void error_Alloc(DWORD last_Error);

void separate_Reservation();

void simultane_Reservation();

void set_Data_To_Memory_Area();

int access_Menu_Button();

void set_Access_Protection();

void menu();

int main() {
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);

    menu();
    return 0;
}

void get_Free_Memory_Examples() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    MEMORY_BASIC_INFORMATION mbi;
    LPVOID addr = sysInfo.lpMinimumApplicationAddress;

    wcout << L"Доступные адреса для записи (не заняты):\n";

    int found = 0;
    const int max_results = MAX_ADD;

    while (addr < sysInfo.lpMaximumApplicationAddress && found < max_results) {
        if (VirtualQuery(addr, &mbi, sizeof(mbi))) {
            if (mbi.State == MEM_FREE) {
                wcout << mbi.BaseAddress << "\n";
                found++;
            }

            addr = static_cast<BYTE*>(mbi.BaseAddress) + mbi.RegionSize;
        } else {
            break;
        }
    }
}

void get_Writeable_Memory_Examples() {
    int found;
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    MEMORY_BASIC_INFORMATION mbi;
    LPVOID addr = sysInfo.lpMinimumApplicationAddress;

    wcout << L"Доступные участки памяти для записи:\n";

    while (addr < sysInfo.lpMaximumApplicationAddress && found < MAX_ADD) {
        if (VirtualQuery(addr, &mbi, sizeof(mbi))) {
            if (mbi.State == MEM_COMMIT && (mbi.Protect == PAGE_READWRITE || mbi.Protect == PAGE_EXECUTE_READWRITE || mbi.Protect == PAGE_WRITECOPY || mbi.Protect == (PAGE_READWRITE | PAGE_GUARD))){
                wcout << mbi.BaseAddress << L"\n";
                found++;
            }
            addr = static_cast<BYTE*>(mbi.BaseAddress) + mbi.RegionSize;
        } else {
            break;
        }
    }
}

void help() {
    wcout << L"\n=== Справка по программе ===\n\n";
    wcout << L"Данное приложение предназначено для работы с виртуальной памятью Windows\n";
    wcout << L"Оно предоставляет следующие возможности:\n\n";

    wcout << Menu::HELP << L" - Получение данной справки\n";
    wcout << Menu::GET_INFO_SYSTEM << L" - Получение информации о вычислительной системе (Win32 API - GetSystemInfo)\n";
    wcout << Menu::GET_INFO_VIRTUAL_MEMORY << L" - Определение статуса виртуальной памяти (Win32 API - GlobalMemoryStatusEx)\n";
    wcout << Menu::GET_INFO_MEMORY_AREA << L" - Определение состояния конкретного участка памяти по заданному адресу (Win32 API - VirtualQuery)\n";
    wcout << Menu::SEPARATE_RESERVATION << L" - Раздельное резервирование региона памяти и выделение физической памяти\n";
    wcout << L"    - Автоматический режим\n";
    wcout << L"    - Ручной ввод адреса\n";
    wcout << Menu::SIMULTANE_RESERVATION << L" - Одновременное резервирование региона памяти и выделение физической памяти\n";
    wcout << L"    - Автоматический режим\n";
    wcout << L"    - Ручной ввод адреса\n";
    wcout << Menu::SET_DATA_TO_MEMORY_AREA << L" - Запись данных в ячейки памяти по введённым адресам\n";
    wcout << Menu::SET_ACCESS_PROTECTION << L" - Установка защиты доступа для заданного региона памяти (Win32 API - VirtualProtect)\n";
    wcout << Menu::EXIT << L" - Выход из программы\n";

    wcout << L"\n=== Управление программой ===\n\n";
    wcout << L"1. Выберите нужную опцию из меню, введя соответствующую цифру\n";
    wcout << L"2. Ввод данных осуществляется в шестнадцатеричном формате (для адресов) и десятичном формате (для чисел)\n";
    wcout << L"3. Если ввод неверный, программа запросит повторный ввод\n";
    wcout << L"4. Для выхода из программы используйте опцию " << Menu::EXIT << L" - Выход\"\n";
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

int menu_Buttons(){
    int user_Choose;

    wcout << Menu::HELP << L" - Получение помощи\n";
    wcout << Menu::GET_INFO_SYSTEM << L" - Получение информации о вычислительной системе\n";
    wcout << Menu::GET_INFO_VIRTUAL_MEMORY << L" - Определение статуса виртуальной памяти\n";
    wcout << Menu::GET_INFO_MEMORY_AREA << L" - Определение состояния конкретного участка памяти по заданному с клавиатуры адресу\n";
    wcout << Menu::SEPARATE_RESERVATION << L" - Автоматическое и ручное резервирование региона с передачей ему физической памяти\n";
    wcout << Menu::SIMULTANE_RESERVATION << L" - Одновременное автоматическое и ручное резервирование региона с передачей ему физической памяти\n";
    wcout << Menu::SET_DATA_TO_MEMORY_AREA << L" - Запись данных в ячейки памяти по заданным с клавиатуры адресам\n";
    wcout << Menu::SET_ACCESS_PROTECTION << L" - Установка защиты доступа для заданного (с клавиатуры) региона памяти и ее проверку\n";
    wcout << Menu::EXIT << L" - Выход из программы\n";

    user_Choose = get_Valid_Input({Menu::HELP, Menu::GET_INFO_SYSTEM, Menu::GET_INFO_VIRTUAL_MEMORY, Menu::GET_INFO_MEMORY_AREA, Menu::SEPARATE_RESERVATION,
                                   Menu::SIMULTANE_RESERVATION, Menu::SET_DATA_TO_MEMORY_AREA, Menu::SET_ACCESS_PROTECTION, Menu::EXIT});
    return user_Choose;
}

void get_Info_System(){
    SYSTEM_INFO sys_Info;

    GetSystemInfo(&sys_Info);

    bitset<64> bitMask(sys_Info.dwActiveProcessorMask);

    wcout << L"\n=== Информация о системе ===\n\n";
    wcout << L"Архитектура системы: ";

    for (const auto& pair : architecture) {
        if (sys_Info.dwOemId == pair.first) {
            wcout << pair.second;
        }
    }

    wcout << L"Логических процессоров: " << sys_Info.dwNumberOfProcessors << "\n";
    wcout << L"Размер страницы: " << sys_Info.dwPageSize << L" Байт \n";
    wcout << L"Минимальный адрес приложения: " << sys_Info.lpMinimumApplicationAddress << " ( " << uintptr_t(sys_Info.lpMinimumApplicationAddress) / Conversion::TO_KB << L"КБ )\n";
    wcout << L"Максимальный адрес приложения: " << sys_Info.lpMaximumApplicationAddress << " ( " << uintptr_t(sys_Info.lpMaximumApplicationAddress) / Conversion::TO_GB << L" ГБ )\n";
    wcout << L"Количество активных процессоров: " << bitMask.count() << "\n";
}

void get_Info_Virtual_Memory(){
    MEMORYSTATUSEX mem_Info;

    mem_Info.dwLength = sizeof(mem_Info);
    GlobalMemoryStatusEx(&mem_Info);

    wcout << L"\n=== Статус памяти ===\n\n";
    wcout << L"Процент используемой физической памяти: " << mem_Info.dwMemoryLoad << L" %\n";
    wcout << L"Объем фактической памяти: " << mem_Info.ullTotalPhys / Conversion::TO_GB << L" ГБ\n";
    wcout << L"Объем физической памяти, доступной в данный момент: " << mem_Info.ullAvailPhys / Conversion::TO_GB << L" ГБ\n\n";
    wcout << L"Размер виртуального адресного пространства: " << mem_Info.ullTotalVirtual / Conversion::TO_GB << L" ГБ (Теоретический максимум)\n";
    wcout << L"Свободно виртуального адресного пространства: " << mem_Info.ullAvailVirtual / Conversion::TO_GB << L" ГБ\n\n";
    wcout << L"Всего виртуальной памяти (ОЗУ + файл подкачки): " << (mem_Info.ullTotalPhys + mem_Info.ullTotalPageFile) / Conversion::TO_GB << L" ГБ\n";
    wcout << L"Доступно виртуальной памяти (свободно ОЗУ + файл подкачки): " << (mem_Info.ullAvailPhys + mem_Info.ullAvailPageFile) / Conversion::TO_GB << L" ГБ\n";
}

void get_Info_Memory_Area(){
    LPVOID address;
    MEMORY_BASIC_INFORMATION mem_Info;

    get_Free_Memory_Examples();
    wcout << L"\nВведите адрес памяти (в шестнадцатеричном формате)\n";
    wcin >> hex >> address;

    if (VirtualQuery(reinterpret_cast<LPVOID>(address), &mem_Info, sizeof(mem_Info)) == 0) {
        switch(GetLastError()){
            case ERROR_INVALID_PARAMETER:
                wcout << L"Ошибка! Неверный параметр\n";
                break;
            case ERROR_ACCESS_DENIED:
                wcout << L"Ошибка! Доступ запрещен\n";
                break;
            default:
                wcout << DEFAULT_ERROR;
        }
        return;
    }

    wcout << L"\n=== Информация о памяти ===\n\n";
    wcout << L"Базовый адрес: " << mem_Info.BaseAddress << "\n";
    wcout << L"Размер региона: " << mem_Info.RegionSize / Conversion::TO_KB << L" КБ\n";
    wcout << L"Тип защиты: ";

    for (const auto& pair : protect) {
        if (mem_Info.Protect & pair.first) {
            wcout << "\n\t" << pair.second;
        }
    }

    wcout << L"Состояние: ";
    switch(mem_Info.State) {
        case MEM_COMMIT:
            wcout << L"Выделено\n";
            break;
        case MEM_RESERVE:
            wcout << L"Зарезервировано\n";
            break;
        case MEM_FREE:
            wcout << L"Свободно\n";
            break;
        default:
            wcout << DEFAULT_ERROR;
    }
}

void error_Alloc(DWORD last_Error){
    switch(last_Error){
        case ERROR_NOT_ENOUGH_MEMORY:
            wcout << L"Недостаточно физической памяти или доступного виртуального адресного пространства\n";
            break;
        case ERROR_INVALID_PARAMETER:
            wcout << L"Переданы неверные параметры\n";
            break;
        case ERROR_ACCESS_DENIED:
            wcout << L"Операция запрещена (например, попытка выделения памяти в защищённой области)\n";
            break;
        case ERROR_GEN_FAILURE:
            wcout << L"Общий сбой системы, связанный с управлением памятью\n";
            break;
        case ERROR_INVALID_ADDRESS:
            wcout << L"Указанный адрес недоступен для выделения\n";
        default:
            wcout << DEFAULT_ERROR;
    }
}

void separate_Reservation(){
    int user_Choose;
    LPVOID mem, address, committed_Region;

    wcout << L"\n=== Раздельное резервирование и выделение физической памяти ===\n\n";

    wcout << Mode::AUTO << L" - Передача памяти в автоматическом режиме\n";
    wcout << Mode::SET << L" - Передача памяти в режиме ввода\n";

    user_Choose = get_Valid_Input({Mode::AUTO, Mode::SET});
    switch (user_Choose) {
        case Mode::AUTO:
            mem = VirtualAlloc(NULL, 8192, MEM_RESERVE, PAGE_READWRITE);

            if(mem){
                wcout << L"\nРезервирование выполнено успешно: " << reinterpret_cast<LPVOID>(mem) << "\n";
            }else{
                error_Alloc(GetLastError());
            }
            break;
        case Mode::SET:{
            get_Free_Memory_Examples();
            wcout << L"\nВведите адрес памяти (в шестнадцатеричном формате)\n";

            wcin >> hex >> address;
            mem = VirtualAlloc(reinterpret_cast<LPVOID>(address), 4096, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

            if(mem){
                wcout << L"\nРезервирование выполнено успешно: " << reinterpret_cast<LPVOID>(mem)<< "\n";
            }else{
                error_Alloc(GetLastError());
                return;
            }
            break;
        }
    }

    committed_Region = VirtualAlloc(reinterpret_cast<LPVOID>(address), 4096, MEM_COMMIT, PAGE_READWRITE);

    if(committed_Region){
        wcout << L"Физическая память выделена успешно: " << committed_Region << "\n";
    }else{
        error_Alloc(GetLastError());
        return;
    }

    wcout << L"\nОсвобождение памяти . . .\n";

    if (committed_Region) {
        if (VirtualFree(committed_Region, 4096, MEM_DECOMMIT)) {
            wcout << L"\nФизическая память освобождена успешно\n";
        } else {
            error_Alloc(GetLastError());
            return;
        }
    }

    if (mem) {
        if (VirtualFree(mem, 0, MEM_RELEASE)) {
            wcout << L"Резерв памяти освобождён успешно\n";
        } else {
            error_Alloc(GetLastError());
        }
    }
}

void simultane_Reservation(){
    int user_Choose;
    LPVOID mem, address;

    wcout << L"\n=== Одновременное резервирование и выделение памяти ===\n\n";

    wcout << Mode::AUTO << L" - Автоматическое выделение\n";
    wcout << Mode::SET << L" - Выделение с вводом адреса\n";

    user_Choose = get_Valid_Input({Mode::AUTO, Mode::SET});

    switch (user_Choose) {
        case Mode::AUTO:
            mem = VirtualAlloc(NULL, 8192, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

            if (mem) {
                wcout << L"\nПамять успешно выделена: " << reinterpret_cast<LPVOID>(mem) << "\n";
            } else {
                error_Alloc(GetLastError());
                return;
            }
            break;

        case Mode::SET:{
            get_Free_Memory_Examples();
            wcout << L"\nВведите адрес памяти (в шестнадцатеричном формате)\n";
            wcin >> hex >> address;

            mem = VirtualAlloc(reinterpret_cast<LPVOID>(address), 8192, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

            if (mem) {
                wcout << L"\nПамять успешно выделена: " << reinterpret_cast<LPVOID>(mem) << "\n";
            } else {
                error_Alloc(GetLastError());
                return;
            }
            break;
        }
    }

    wcout << L"\nОсвобождение памяти . . .\n";

    if (mem) {
        if (VirtualFree(mem, 0, MEM_RELEASE)) {
            wcout << L"Память полностью освобождена\n";
        } else {
            error_Alloc(GetLastError());
        }
    }
}

void set_Data_To_Memory_Area() {
    DWORD old_Protect;
    LPVOID address;
    int value;

    wcout << L"\n=== Запись данных в ячейки памяти по заданным с клавиатуры адресам ===\n\n";

    get_Writeable_Memory_Examples();
    wcout << L"\nВведите адрес памяти (в шестнадцатеричном формате)\n";

    wcin >> hex >> address;

    wcout << L"\nВведите целочисленное значение для записи:\n";
    wcin >> value;

    if (VirtualProtect(address, sizeof(int), PAGE_READWRITE, &old_Protect)) {
        *reinterpret_cast<int*>(address) = value;

        wcout << L"Значение " << *reinterpret_cast<int*>(address) << L" успешно записано по адресу " << address << L"\n";

        VirtualProtect(address, sizeof(int), old_Protect, &old_Protect);
    } else {
        switch(GetLastError()) {
            case ERROR_ACCESS_DENIED:
                wcout << L"Доступ запрещён. Нельзя изменять защищённую область памяти\n";
                break;
            case ERROR_INVALID_ADDRESS:
                wcout << L"Неверный адрес. Указанная область памяти недоступна.\n";
                break;
            case ERROR_NOT_ENOUGH_MEMORY:
                wcout << L"Недостаточно памяти для выполнения операции.\n";
                break;
            default:
                wcout << DEFAULT_ERROR;
        }
    }
}

int access_Menu_Button(){
    int protection_Choice;

    wcout << L"\nВыберите тип защиты:\n";
    wcout << Protect_Type::NOACCESS << L" - Запрет любого доступа (чтение/запись/исполнение)\n";
    wcout << Protect_Type::READONLY << L" - Только чтение данных\n";
    wcout << Protect_Type::READWRITE << L" - Чтение и запись данных\n";
    wcout << Protect_Type::EXECUTE << L" - Только исполнение кода\n";
    wcout << Protect_Type::EXECUTE_READ << L" - Исполнение и чтение\n";
    wcout << Protect_Type::EXECUTE_READWRITE << L" - Полный доступ (исполнение/чтение/запись)\n";
    wcout << Protect_Type::WRITECOPY << L" - Копирование при записи (для отображения файлов)\n";

    protection_Choice = get_Valid_Input({Protect_Type::NOACCESS, Protect_Type::READONLY, Protect_Type::READWRITE, Protect_Type::EXECUTE,
                                         Protect_Type::EXECUTE_READ, Protect_Type::EXECUTE_READWRITE, Protect_Type::WRITECOPY});

    return protection_Choice;
}

void set_Access_Protection() {
    MEMORY_BASIC_INFORMATION mem_Info;
    DWORD old_Protect, new_Protect;
    LPVOID address, allocated_Memory;
    SYSTEM_INFO sysInfo;
    SIZE_T size;
    int user_Choose;

    wcout << L"\n=== Установка защиты доступа для собственного региона памяти и её проверка ===\n\n";

    get_Free_Memory_Examples();

    wcout << L"\nВведите адрес памяти (в шестнадцатеричном формате):\n";
    wcin >> hex >> address;

    allocated_Memory = VirtualAlloc(address, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!allocated_Memory) {
        error_Alloc(GetLastError());
        return;
    }

    wcout << L"Память успешно выделена: " << allocated_Memory << L"\n";

    GetSystemInfo(&sysInfo);
    size = sysInfo.dwPageSize;

    user_Choose = access_Menu_Button();

    for (const auto& pair : new_Protect_) {
        if (user_Choose == pair.first) {
            new_Protect = pair.second;
        }
    }

    if (VirtualQuery(allocated_Memory, &mem_Info, sizeof(mem_Info)) == 0) {
        wcout << L"\nОшибка VirtualQuery!\n";
        return;
    }

    if (!VirtualProtect(allocated_Memory, size, new_Protect, &old_Protect)) {
        error_Alloc(GetLastError());
        return;
    }

    wcout << L"\nЗащита успешно изменена!\n";
}

void menu() {
    int user_Choose;

    do {
        wcout << L"\n=== Меню ===\n\n";
        user_Choose = menu_Buttons();

        switch(user_Choose) {
            case Menu::HELP:
                help();
                break;
            case Menu::GET_INFO_SYSTEM:
                get_Info_System();
                break;
            case Menu::GET_INFO_VIRTUAL_MEMORY:
                get_Info_Virtual_Memory();
                break;
            case Menu::GET_INFO_MEMORY_AREA:
                get_Info_Memory_Area();
                break;
            case Menu::SEPARATE_RESERVATION:
                separate_Reservation();
                break;
            case Menu::SIMULTANE_RESERVATION:
                simultane_Reservation();
                break;
            case Menu::SET_DATA_TO_MEMORY_AREA:
                set_Data_To_Memory_Area();
                break;
            case Menu::SET_ACCESS_PROTECTION:
                set_Access_Protection();
                break;
            case Menu::EXIT:
                wcout << L"\nВыход . . .\n";
                return;
            default:
                wcout << DEFAULT_ERROR;
        }
    } while (user_Choose != Menu::EXIT);
}