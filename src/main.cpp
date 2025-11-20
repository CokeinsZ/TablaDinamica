#include <iostream>
#include <vector>
#include <random>
#include "DynamicTable.hpp"
#include "OperationCounter.hpp"

/*
  main builds 10 different sequences (non-trivial patterns) with incremental sizes,
  runs them on a fresh DynamicTable<int> instrumented with OperationCounter, and prints results.
*/

enum class SeqType {
    ROW_ONLY,
    COL_ONLY,
    PUSH_THEN_POP_ROWS,
    PUSH_THEN_POP_COLS,
    ALT_ROW_COL,
    BATCHED_EXPAND,
    RANDOM_OPS,
    INSERT_REMOVE_MID,
    GROW_SHRINK_CYCLES,
    PATTERNED_MIX
};

struct Sequence {
    SeqType type;
    std::size_t param; // size or repeats
};

void run_sequence(const Sequence& seq, OperationCounter &counter) {
    counter.reset();
    DynamicTable table(&counter, 2, 2); // start with small capacity

    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> dist_value(1, 1000);

    switch (seq.type) {
        case SeqType::ROW_ONLY:
            for (std::size_t i = 0; i < seq.param; ++i) {
                table.push_row();
                // fill row with a value
                for (std::size_t c = 0; c < table.cols(); ++c) {
                    table.set(table.rows()-1, c, static_cast<int>(i));
                }
            }
            break;

        case SeqType::COL_ONLY:
            for (std::size_t i = 0; i < seq.param; ++i) {
                table.push_col();
                for (std::size_t r = 0; r < table.rows(); ++r) {
                    table.set(r, table.cols()-1, static_cast<int>(i));
                }
            }
            break;

        case SeqType::PUSH_THEN_POP_ROWS:
            for (std::size_t i = 0; i < seq.param; ++i) table.push_row();
            for (std::size_t i = 0; i < seq.param; ++i) table.pop_row();
            break;

        case SeqType::PUSH_THEN_POP_COLS:
            for (std::size_t i = 0; i < seq.param; ++i) table.push_col();
            for (std::size_t i = 0; i < seq.param; ++i) table.pop_col();
            break;

        case SeqType::ALT_ROW_COL:
            for (std::size_t i = 0; i < seq.param; ++i) {
                table.push_row();
                table.push_col();
                if (i % 3 == 0 && table.rows()>0) table.pop_row();
                if (i % 5 == 0 && table.cols()>0) table.pop_col();
            }
            break;

        case SeqType::BATCHED_EXPAND:
            for (std::size_t batch = 0; batch < seq.param; ++batch) {
                for (int j = 0; j < 50; ++j) table.push_row();
                for (int j = 0; j < 25; ++j) table.pop_row();
                for (int j = 0; j < 50; ++j) table.push_col();
                for (int j = 0; j < 25; ++j) table.pop_col();
            }
            break;

        case SeqType::RANDOM_OPS:
            for (std::size_t i = 0; i < seq.param; ++i) {
                int op = rng() % 6;
                try {
                    switch (op) {
                        case 0: table.push_row(); break;
                        case 1: if (table.rows()>0) table.pop_row(); break;
                        case 2: table.push_col(); break;
                        case 3: if (table.cols()>0) table.pop_col(); break;
                        case 4:
                            if (table.rows()>0 && table.cols()>0) {
                                std::size_t r = rng() % table.rows();
                                std::size_t c = rng() % table.cols();
                                table.set(r,c, dist_value(rng));
                            }
                            break;
                        case 5:
                            if (table.rows()>0 && table.cols()>0) {
                                std::size_t r = rng() % table.rows();
                                std::size_t c = rng() % table.cols();
                                (void) table.get(r,c);
                            }
                            break;
                    }
                } catch (...) { /* ignore out_of_range in random */ }
            }
            break;

        case SeqType::INSERT_REMOVE_MID:
            for (std::size_t i = 0; i < seq.param; ++i) {
                table.push_row();
                if (table.rows() >= 2) table.insert_row_at(table.rows()/2);
                table.push_col();
                if (table.cols() >= 2) table.insert_col_at(table.cols()/2);
                if (i % 4 == 0 && table.rows()>0) table.remove_row_at(0);
                if (i % 6 == 0 && table.cols()>0) table.remove_col_at(0);
            }
            break;

        case SeqType::GROW_SHRINK_CYCLES:
            for (std::size_t cycle = 0; cycle < seq.param; ++cycle) {
                for (int j = 0; j < 512; ++j) table.push_row();
                for (int j = 0; j < 512; ++j) table.pop_row();
            }
            break;

        case SeqType::PATTERNED_MIX:
            for (std::size_t i = 0; i < seq.param; ++i) {
                table.push_row();
                for (std::size_t c = 0; c < table.cols(); ++c)
                    table.set(table.rows()-1, c, static_cast<int>(i));
                if (i % 2 == 0) table.push_col();
                if (i % 7 == 0 && table.cols()>0) table.pop_col();
                if (i % 11 == 0 && table.rows()>0) table.pop_row();
            }
            break;
    }

    // report
    std::string title;
    switch (seq.type) {
        case SeqType::ROW_ONLY: title = "ROW_ONLY N=" + std::to_string(seq.param); break;
        case SeqType::COL_ONLY: title = "COL_ONLY N=" + std::to_string(seq.param); break;
        case SeqType::PUSH_THEN_POP_ROWS: title = "PUSH_THEN_POP_ROWS N=" + std::to_string(seq.param); break;
        case SeqType::PUSH_THEN_POP_COLS: title = "PUSH_THEN_POP_COLS N=" + std::to_string(seq.param); break;
        case SeqType::ALT_ROW_COL: title = "ALT_ROW_COL N=" + std::to_string(seq.param); break;
        case SeqType::BATCHED_EXPAND: title = "BATCHED_EXPAND batches=" + std::to_string(seq.param); break;
        case SeqType::RANDOM_OPS: title = "RANDOM_OPS ops=" + std::to_string(seq.param); break;
        case SeqType::INSERT_REMOVE_MID: title = "INSERT_REMOVE_MID N=" + std::to_string(seq.param); break;
        case SeqType::GROW_SHRINK_CYCLES: title = "GROW_SHRINK_CYCLES cycles=" + std::to_string(seq.param); break;
        case SeqType::PATTERNED_MIX: title = "PATTERNED_MIX N=" + std::to_string(seq.param); break;
    }

    counter.report(title);
}

int main() {
    std::vector<Sequence> sequences = {
        {SeqType::ROW_ONLY, 1000},
        {SeqType::COL_ONLY, 1000},
        {SeqType::PUSH_THEN_POP_ROWS, 2000},
        {SeqType::PUSH_THEN_POP_COLS, 2000},
        {SeqType::ALT_ROW_COL, 1500},
        {SeqType::BATCHED_EXPAND, 10},
        {SeqType::RANDOM_OPS, 10000},
        {SeqType::INSERT_REMOVE_MID, 500},
        {SeqType::GROW_SHRINK_CYCLES, 5},
        {SeqType::PATTERNED_MIX, 1200}
    };

    OperationCounter counter;
    for (const auto &s : sequences) {
        run_sequence(s, counter);
    }

    return 0;
}
