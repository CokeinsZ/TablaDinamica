#ifndef DYNAMIC_TABLE_HPP
#define DYNAMIC_TABLE_HPP

#include "OperationCounter.hpp"
#include <cstddef>
#include <stdexcept>
#include <string>

class DynamicTable {
  public:
    DynamicTable(OperationCounter* counter = nullptr, std::size_t initial_rows = 1, std::size_t initial_cols = 1);
    ~DynamicTable();

    // row operations
    void push_row();
    void insert_row_at(std::size_t idx);
    void pop_row();
    void remove_row_at(std::size_t idx);

    // column operations
    void push_col();
    void insert_col_at(std::size_t idx);
    void pop_col();
    void remove_col_at(std::size_t idx);

    // element access / modify
    int get(std::size_t r, std::size_t c) const;
    void set(std::size_t r, std::size_t c, int value);

    std::size_t rows() const noexcept { return _rows; }
    std::size_t cols() const noexcept { return _cols; }
    std::size_t row_capacity() const noexcept { return _row_capacity; }
    std::size_t col_capacity() const noexcept { return _col_capacity; }

    void clear();
    std::string print_shape() const;

  private:
    void ensure_row_capacity_for(std::size_t min_rows);
    void ensure_col_capacity_for(std::size_t min_cols);
    void reallocate(std::size_t new_row_cap, std::size_t new_col_cap);

    int** _data;
    std::size_t _rows;
    std::size_t _cols;
    std::size_t _row_capacity;
    std::size_t _col_capacity;

    OperationCounter* _counter;
};

#endif
