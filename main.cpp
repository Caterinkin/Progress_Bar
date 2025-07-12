#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <iomanip>
#include <atomic>
#include <sstream>
#include <windows.h> // ��� ������ � �������� Windows

// ���������� ������� ��� ������������� ������ � �������
std::mutex g_cout_mutex;

// ������� ��� ��������� ������� �������
void set_cursor_position(int x, int y) {
    COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// ����� ��� ���������� ��������-����� ������
class ProgressBar {
private:
    const int thread_num;      // ����� ������ �� �������
    const std::thread::id tid; // ������������� ������
    const int length;          // ����� ��������-����
    int progress = 0;          // ������� ��������
    std::chrono::time_point<std::chrono::steady_clock> start_time;
    int line_position;         // ������� ������ ����� ������

public:
    ProgressBar(int num, std::thread::id id, int len, int pos)
        : thread_num(num), tid(id), length(len), line_position(pos) {
        start_time = std::chrono::steady_clock::now();
        print_initial();
    }

    // ����� ��������� ���������� � ������
    void print_initial() {
        std::lock_guard<std::mutex> lock(g_cout_mutex);
        set_cursor_position(0, line_position);
        std::cout << "����� " << std::setw(2) << thread_num
            << " (ID: " << tid << "): [";
        for (int i = 0; i < length; i++) std::cout << " ";
        std::cout << "]   0%";
        std::cout.flush();
    }

    // ���������� ��������-����
    void update() {
        if (progress >= length) return;

        progress++;

        std::lock_guard<std::mutex> lock(g_cout_mutex);
        set_cursor_position(18 + progress, line_position);
        std::cout << "#";
        set_cursor_position(19 + length, line_position);
        std::cout << std::setw(3) << (100 * progress / length) << "%";
        std::cout.flush();
    }

    // ����� �������� ����������
    void print_final() {
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        std::lock_guard<std::mutex> lock(g_cout_mutex);
        set_cursor_position(25 + length, line_position);
        std::cout << " �����: " << duration.count() << " ��";
        std::cout.flush();
    }

    bool is_complete() const {
        return progress >= length;
    }
};

// �������, ����������� � ������ ������
void calculation_task(int thread_num, int progress_length, int line_pos) {
    ProgressBar pb(thread_num, std::this_thread::get_id(), progress_length, line_pos);

    // �������� �������
    while (!pb.is_complete()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100 + rand() % 200));
        pb.update();
    }
    pb.print_final();
}

int main() {
    std::setlocale(LC_ALL, "Russian");

    const int num_threads = 5;      // ���������� �������
    const int progress_length = 30; // ����� ��������-����

    // ������� ������� � ������������� ������ � ������
    system("cls");
    std::cout << "������������� ������ � ��������-������:\n\n";

    // ������� ������ �������
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    // ������� � ��������� ������
    for (int i = 1; i <= num_threads; ++i) {
        threads.emplace_back(calculation_task, i, progress_length, 2 + i);
    }

    // ������� ���������� ���� �������
    for (auto& t : threads) {
        t.join();
    }

    // ���������� ������ � �����
    set_cursor_position(0, 3 + num_threads);
    std::cout << "\n��� ������ ��������� ������.\n";
    return 0;
}