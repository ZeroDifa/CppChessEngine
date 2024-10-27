#pragma once;
#include "SimpleJSON.h"
#include <string>
#include <mutex>
#include <random>
#include <array>
#include "robin_hood.h"
#include "constants.h"

using namespace std;

std::mutex tt_mtx;


struct TranspositionTableEntry {
    double value;
    int depth;
    int flag;
};

class TranspositionTable {
public:
    TranspositionTable(size_t maxSize) : maxSize(maxSize) {}
    void clear() {
        std::lock_guard<std::mutex> lock(tt_mtx);
        table.clear();
    }
    bool lookup(uint64_t key, TranspositionTableEntry& entry) {
        std::lock_guard<std::mutex> lock(tt_mtx);
        auto it = table.find(key);
        if (it != table.end()) {
            entry = it->second;
            return true;
        }
        return false;
    }

    void store(uint64_t key, TranspositionTableEntry entry) {
        std::lock_guard<std::mutex> lock(tt_mtx);
        table[key] = entry;
    }
    robin_hood::unordered_map<uint64_t, TranspositionTableEntry> table;

private:
    size_t maxSize;
};

class ZobristHash {
public:
    ZobristHash() {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis;

        // Инициализация случайных чисел для каждого типа фигуры на каждой клетке
        for (int i = 0; i < 64; ++i) {
            for (int j = 0; j < NUM_STATES; ++j) {
                piece_keys[i][j] = dis(gen);
            }
        }
    }

    uint64_t hash(const std::array<int, 64>& board) const {
        uint64_t h = 0;
        for (int i = 0; i < 64; ++i) {
            int piece = board[i];
            if (piece != EMPTY) {
                h ^= piece_keys[i][piece];
            }
        }
        return h;
    }

private:
    static constexpr int NUM_STATES = 13;  // 6 типов фигур * 2 цвета + пустая клетка
    static constexpr int EMPTY = 12;       // Индекс пустой клетки в массиве piece_keys
    uint64_t piece_keys[64][NUM_STATES];
};

