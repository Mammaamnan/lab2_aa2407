#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <algorithm>
#include <sstream> // <- обязательно для istringstream
#include "Manager.h"

// helper input functions
static void ignoreLine() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static std::string inputLine(const std::string& prompt) {
    std::string s;
    std::cout << prompt;
    std::getline(std::cin, s);
    return s;
}

static int inputInt(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        int v;
        if (std::cin >> v) {
            ignoreLine();
            return v;
        } else {
            std::cout << "Ввод некорректен. Попробуйте ещё раз.\n";
            std::cin.clear();
            ignoreLine();
        }
    }
}

static double inputDouble(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        double v;
        if (std::cin >> v) { ignoreLine(); return v; }
        else {
            std::cout << "Ввод некорректен. Попробуйте ещё раз.\n";
            std::cin.clear();
            ignoreLine();
        }
    }
}

void showPipe(const Pipe& p) {
    std::cout << "ID=" << p.getId()
              << " | Name=\"" << p.getName() << "\""
              << " | Diameter=" << p.getDiameter()
              << " | InRepair=" << (p.isInRepair() ? "YES":"NO")
              << "\n";
}

void showStation(const CompressorStation& s) {
    std::cout << "ID=" << s.getId()
              << " | Name=\"" << s.getName() << "\""
              << " | Total=" << s.getTotalWorkshops()
              << " | Working=" << s.getWorkingWorkshops()
              << " | Idle%=" << s.percentIdle()
              << " | Class=\"" << s.getClassification() << "\""
              << "\n";
}

void addSampleData(Manager& m) {
    m.addPipe("MainLine-1", 500.0, false);
    m.addPipe("Feeder-A", 250.0, true);
    m.addPipe("Bypass-02", 300.0, false);
    m.addStation("CS-North", 10, 8, "A");
    m.addStation("CS-South", 6, 2, "B");
    m.addStation("CS-East", 12, 12, "A+");
}

int main() {
    Manager manager;
    std::cout << "=== Менеджер труб и компрессорных станций ===\n";
    std::string logf = inputLine("Введите имя файла для логов (или Enter для actions.log): ");
    if (!logf.empty()) manager.setLogFilename(logf);

    bool running = true;
    while (running) {
        std::cout << "\nГлавное меню:\n";
        std::cout << "1) Добавить трубу\n";
        std::cout << "2) Редактировать трубу\n";
        std::cout << "3) Удалить трубу\n";
        std::cout << "4) Поиск труб\n";
        std::cout << "5) Пакетное редактирование труб\n";
        std::cout << "6) Список всех труб\n";
        std::cout << "7) Добавить КС\n";
        std::cout << "8) Редактировать КС\n";
        std::cout << "9) Удалить КС\n";
        std::cout << "10) Поиск КС\n";
        std::cout << "11) Список всех КС\n";
        std::cout << "12) Сохранить в файл\n";
        std::cout << "13) Загрузить из файла\n";
        std::cout << "14) Добавить демонстрационные данные\n";
        std::cout << "0) Выход\n";
        int choice = inputInt("Выберите пункт: ");
        switch (choice) {
            case 1: {
                std::string name = inputLine("Имя трубы: ");
                double diam = inputDouble("Диаметр (число): ");
                std::string rep = inputLine("В ремонте? (y/n): ");
                bool inrep = (rep.size()>0 && (rep[0]=='y' || rep[0]=='Y'));
                uint64_t id = manager.addPipe(name, diam, inrep);
                std::cout << "Добавлена труба с ID=" << id << "\n";
                break;
            }
            case 2: {
                uint64_t id = (uint64_t) inputInt("ID трубы для редактирования: ");
                Pipe* p = manager.findPipeById(id);
                if (!p) { std::cout << "Труба с таким ID не найдена\n"; break; }
                std::cout << "Текущие данные:\n"; showPipe(*p);
                std::string newName = inputLine("Новое имя (Enter = без изменений): ");
                std::string dstr = inputLine("Новый диаметр (Enter = без изменений): ");
                if (!newName.empty()) p->setName(newName);
                if (!dstr.empty()) {
                    try { double d = std::stod(dstr); p->setDiameter(d); } catch(...) { std::cout << "Диаметр не изменён: неверный ввод\n"; }
                }
                std::string rep = inputLine("В ремонте? (y/n/Enter = без изменений): ");
                if (!rep.empty()) {
                    bool inrep = (rep[0]=='y' || rep[0]=='Y');
                    p->setInRepair(inrep);
                }
                // Вместо обращения к приватному logAction используем публичный writeLog
                manager.writeLog("Edited pipe id=" + std::to_string(p->getId()));
                std::cout << "Изменено.\n";
                break;
            }
            case 3: {
                uint64_t id = (uint64_t) inputInt("ID трубы для удаления: ");
                if (manager.removePipeById(id)) std::cout << "Удалено.\n"; else std::cout << "Не найдено.\n";
                break;
            }
            case 4: {
                std::cout << "Поиск труб: 1) по имени 2) по признаку 'в ремонте'\n";
                int m = inputInt("Выберите фильтр: ");
                if (m == 1) {
                    std::string q = inputLine("Введите подстроку имени: ");
                    auto res = manager.findPipesByName(q);
                    std::cout << "Найдено " << res.size() << " труб:\n";
                    for (auto p : res) showPipe(*p);
                } else if (m == 2) {
                    std::string rep = inputLine("Искать трубы в ремонте? (y ищет в ремонте, n ищет не в ремонте): ");
                    bool flag = (rep.size()>0 && (rep[0]=='y'||rep[0]=='Y'));
                    auto res = manager.findPipesByRepairFlag(flag);
                    std::cout << "Найдено " << res.size() << " труб:\n";
                    for (auto p : res) showPipe(*p);
                } else std::cout << "Неверно.\n";
                break;
            }
            case 5: {
                std::cout << "Пакетное редактирование труб.\n";
                std::cout << "Сначала выполните поиск, чтобы получить список (см. меню Поиск труб).\n";
                std::cout << "Выберите: 1) ввести вручную список ID через пробел  2) загрузить все трубы вхождение по имени\n";
                int sub = inputInt("Выбор: ");
                std::vector<uint64_t> ids;
                if (sub == 1) {
                    std::string line = inputLine("Введите ID через пробел: ");
                    std::istringstream iss(line);
                    uint64_t v;
                    while (iss >> v) ids.push_back(v);
                } else if (sub == 2) {
                    std::string q = inputLine("Подстрока имени для выбора: ");
                    auto res = manager.findPipesByName(q);
                    for (auto p : res) ids.push_back(p->getId());
                } else {
                    std::cout << "Неверный выбор.\n";
                    break;
                }
                if (ids.empty()) { std::cout << "Нечего редактировать.\n"; break; }
                std::cout << "Будут отредактированы " << ids.size() << " труб.\n";
                std::string newName = inputLine("Новое имя для выбранных (Enter = не менять): ");
                std::string dstr = inputLine("Новый диаметр для выбранных (Enter = не менять): ");
                double newDiameter = -1.0;
                if (!dstr.empty()) {
                    try { newDiameter = std::stod(dstr); } catch(...) { newDiameter = -1.0; }
                }
                std::cout << "Изменить флаг 'в ремонте'? (0 = не менять, 1 = установить true, 2 = установить false)\n";
                int rf = inputInt("Выбор: ");
                int changeFlag = -1;
                if (rf == 1) changeFlag = 1;
                else if (rf == 2) changeFlag = 0;
                else changeFlag = -1;
                manager.batchEditPipes(ids, newName, newDiameter, changeFlag);
                std::cout << "Пакетное редактирование выполнено.\n";
                break;
            }
            case 6: {
                auto &ps = manager.getPipes();
                std::cout << "Всего труб: " << ps.size() << "\n";
                for (const auto& p : ps) showPipe(p);
                break;
            }
            case 7: {
                std::string name = inputLine("Имя КС: ");
                int total = inputInt("Общее число цехов: ");
                int working = inputInt("Число работающих цехов: ");
                std::string cls = inputLine("Классификация: ");
                uint64_t id = manager.addStation(name, total, working, cls);
                std::cout << "Добавлена КС ID=" << id << "\n";
                break;
            }
            case 8: {
                uint64_t id = (uint64_t) inputInt("ID КС для редактирования: ");
                CompressorStation* s = manager.findStationById(id);
                if (!s) { std::cout << "Не найдено.\n"; break; }
                showStation(*s);
                std::string n = inputLine("Новое имя (Enter = без изменений): ");
                std::string tot = inputLine("Новый total (Enter = без изменений): ");
                std::string work = inputLine("Новый working (Enter = без изменений): ");
                std::string cls = inputLine("Новая классификация (Enter = без изменений): ");
                if (!n.empty()) s->setName(n);
                if (!tot.empty()) { try { s->setTotalWorkshops(std::stoi(tot)); } catch(...) { } }
                if (!work.empty()) { try { s->setWorkingWorkshops(std::stoi(work)); } catch(...) { } }
                if (!cls.empty()) s->setClassification(cls);
                manager.writeLog("Edited station id=" + std::to_string(s->getId()));
                std::cout << "Изменено.\n";
                break;
            }
            case 9: {
                uint64_t id = (uint64_t) inputInt("ID КС для удаления: ");
                if (manager.removeStationById(id)) std::cout << "Удалено.\n"; else std::cout << "Не найдено.\n";
                break;
            }
            case 10: {
                std::cout << "Поиск КС: 1) по имени  2) по проценту незадействованных цехов (>=)\n";
                int m = inputInt("Выбор: ");
                if (m == 1) {
                    std::string q = inputLine("Подстрока имени: ");
                    auto res = manager.findStationsByName(q);
                    std::cout << "Найдено " << res.size() << ":\n";
                    for (auto s : res) showStation(*s);
                } else if (m == 2) {
                    double thr = inputDouble("Минимальный процент незадействованных цехов: ");
                    auto res = manager.findStationsByIdlePercent(thr);
                    std::cout << "Найдено " << res.size() << ":\n";
                    for (auto s : res) showStation(*s);
                } else std::cout << "Неверно.\n";
                break;
            }
            case 11: {
                auto &ss = manager.getStations();
                std::cout << "Всего КС: " << ss.size() << "\n";
                for (const auto& s : ss) showStation(s);
                break;
            }
            case 12: {
                std::string fname = inputLine("Введите имя файла для сохранения: ");
                if (fname.empty()) { std::cout << "Имя не задано.\n"; break; }
                if (manager.saveToFile(fname)) std::cout << "Сохранено в " << fname << "\n";
                else std::cout << "Ошибка сохранения.\n";
                break;
            }
            case 13: {
                std::string fname = inputLine("Введите имя файла для загрузки: ");
                if (fname.empty()) { std::cout << "Имя не задано.\n"; break; }
                if (manager.loadFromFile(fname)) std::cout << "Загружено из " << fname << "\n";
                else std::cout << "Ошибка загрузки.\n";
                break;
            }
            case 14: {
                addSampleData(manager);
                std::cout << "Демо-данные добавлены.\n";
                break;
            }
            case 0: {
                running = false; break;
            }
            default:
                std::cout << "Неверный выбор\n";
        }
    }
    std::cout << "Выход.\n";
    return 0;
}