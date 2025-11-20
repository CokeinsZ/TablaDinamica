#include "DynamicTable.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>

static int DEFAULT_INIT_VALUE = 0;

DynamicTable::DynamicTable(OperationCounter* counter, std::size_t initial_rows, std::size_t initial_cols) {
    _data = nullptr; _rows = 0; _cols = 0;
    _row_capacity = std::max<std::size_t>(1, initial_rows);
    _col_capacity = std::max<std::size_t>(1, initial_cols);
    _counter = counter;

    _data = new int*[_row_capacity];
    for (std::size_t i = 0; i < _row_capacity; ++i) {
        _data[i] = new int[_col_capacity];
        for (std::size_t j = 0; j < _col_capacity; ++j) {
            _data[i][j] = DEFAULT_INIT_VALUE;
            if (_counter) ++_counter->element_writes;
        }
    }
}

DynamicTable::~DynamicTable() {
    if (_data) {
        if (_counter) _counter->element_destroys += _row_capacity * _col_capacity;
        for (std::size_t i = 0; i < _row_capacity; ++i) {
            delete[] _data[i];
        }
        delete[] _data;
        _data = nullptr;
    }
}

void DynamicTable::reallocate(std::size_t new_row_cap, std::size_t new_col_cap) {
    if (new_row_cap < 1) new_row_cap = 1;
    if (new_col_cap < 1) new_col_cap = 1;

    int** new_data = new int*[new_row_cap];
    for (std::size_t i = 0; i < new_row_cap; ++i) {
        new_data[i] = new int[new_col_cap];
        for (std::size_t j = 0; j < new_col_cap; ++j) {
            new_data[i][j] = DEFAULT_INIT_VALUE;
            if (_counter) ++_counter->element_writes;
        }
    }

    std::size_t min_rows = std::min(_rows, new_row_cap);
    std::size_t min_cols = std::min(_cols, new_col_cap);

    for (std::size_t r = 0; r < min_rows; ++r) {
        for (std::size_t c = 0; c < min_cols; ++c) {
            new_data[r][c] = _data[r][c];
            if (_counter) ++_counter->element_assignments;
        }
    }

    if (_counter) {
        ++_counter->row_resizes;
        ++_counter->col_resizes;
        ++_counter->row_capacity_changes;
        ++_counter->col_capacity_changes;
        _counter->element_destroys += _row_capacity * _col_capacity;
    }

    for (std::size_t i = 0; i < _row_capacity; ++i) {
        delete[] _data[i];
    }
    delete[] _data;

    _data = new_data;
    _row_capacity = new_row_cap;
    _col_capacity = new_col_cap;
}

void DynamicTable::ensure_row_capacity_for(std::size_t min_rows) {
    if (min_rows <= _row_capacity) return;
    // double until enough
    std::size_t new_row_cap = _row_capacity;
    while (new_row_cap < min_rows) new_row_cap *= 2;
    reallocate(new_row_cap, _col_capacity);
}

void DynamicTable::ensure_col_capacity_for(std::size_t min_cols) {
    if (min_cols <= _col_capacity) return;
    std::size_t new_col_cap = _col_capacity;
    while (new_col_cap < min_cols) new_col_cap *= 2;
    reallocate(_row_capacity, new_col_cap);
}

void DynamicTable::push_row() {
    ensure_row_capacity_for(_rows + 1);
    // initialize new row cells to default value
    for (std::size_t c = 0; c < _col_capacity; ++c) {
        _data[_rows][c] = DEFAULT_INIT_VALUE;
        if (_counter) ++_counter->element_writes;
    }
    ++_rows;
    if (_counter) ++_counter->row_inserts;
    // shrink policy: if rows very small relative to capacity -> shrink
    if (_rows <= _row_capacity / 4 && _row_capacity > 1) {
        std::size_t new_row_cap = std::max<std::size_t>(1, _row_capacity / 2);
        reallocate(new_row_cap, _col_capacity);
    }
}

void DynamicTable::insert_row_at(std::size_t idx) {
    if (idx > _rows) throw std::out_of_range("insert_row_at: index out of range");
    ensure_row_capacity_for(_rows + 1);
    // move rows down from idx..rows-1 to make space
    for (std::size_t r = _rows; r > idx; --r) {
        // move row r-1 to r
        for (std::size_t c = 0; c < _col_capacity; ++c) {
            _data[r][c] = _data[r-1][c];
            if (_counter) ++_counter->element_assignments;
        }
    }
    // initialize new row
    for (std::size_t c = 0; c < _col_capacity; ++c) {
        _data[idx][c] = DEFAULT_INIT_VALUE;
        if (_counter) ++_counter->element_writes;
    }
    ++_rows;
    if (_counter) ++_counter->row_inserts;
    if (_rows <= _row_capacity / 4 && _row_capacity > 1) {
        std::size_t new_row_cap = std::max<std::size_t>(1, _row_capacity / 2);
        reallocate(new_row_cap, _col_capacity);
    }
}

void DynamicTable::pop_row() {
    if (_rows == 0) throw std::out_of_range("pop_row from empty");
    // logically destroy last row (for int just count)
    for (std::size_t c = 0; c < _col_capacity; ++c) {
        if (_counter) ++_counter->element_destroys;
    }
    --_rows;
    if (_counter) ++_counter->row_removes;
    if (_rows <= _row_capacity / 4 && _row_capacity > 1) {
        std::size_t new_row_cap = std::max<std::size_t>(1, _row_capacity / 2);
        reallocate(new_row_cap, _col_capacity);
    }
}

void DynamicTable::remove_row_at(std::size_t idx) {
    if (idx >= _rows) throw std::out_of_range("remove_row_at: index out of range");
    // move rows up from idx+1..rows-1 into idx..rows-2
    for (std::size_t r = idx; r + 1 < _rows; ++r) {
        for (std::size_t c = 0; c < _col_capacity; ++c) {
            _data[r][c] = _data[r+1][c];
            if (_counter) ++_counter->element_assignments;
        }
    }
    // logically destroy last row
    for (std::size_t c = 0; c < _col_capacity; ++c) {
        if (_counter) ++_counter->element_destroys;
    }
    --_rows;
    if (_counter) ++_counter->row_removes;
    if (_rows <= _row_capacity / 4 && _row_capacity > 1) {
        std::size_t new_row_cap = std::max<std::size_t>(1, _row_capacity / 2);
        reallocate(new_row_cap, _col_capacity);
    }
}

void DynamicTable::push_col() {
    ensure_col_capacity_for(_cols + 1);
    // initialize new column cells
    for (std::size_t r = 0; r < _row_capacity; ++r) {
        _data[r][_cols] = DEFAULT_INIT_VALUE;
        if (_counter) ++_counter->element_writes;
    }
    ++_cols;
    if (_counter) ++_counter->col_inserts;
    if (_cols <= _col_capacity / 4 && _col_capacity > 1) {
        std::size_t new_col_cap = std::max<std::size_t>(1, _col_capacity / 2);
        reallocate(_row_capacity, new_col_cap);
    }
}

void DynamicTable::insert_col_at(std::size_t idx) {
    if (idx > _cols) throw std::out_of_range("insert_col_at: index out of range");
    ensure_col_capacity_for(_cols + 1);
    // shift columns to the right
    for (std::size_t r = 0; r < _row_capacity; ++r) {
        for (std::size_t c = _cols; c > idx; --c) {
            _data[r][c] = _data[r][c-1];
            if (_counter) ++_counter->element_assignments;
        }
        // initialize inserted cell
        _data[r][idx] = DEFAULT_INIT_VALUE;
        if (_counter) ++_counter->element_writes;
    }
    ++_cols;
    if (_counter) ++_counter->col_inserts;
    if (_cols <= _col_capacity / 4 && _col_capacity > 1) {
        std::size_t new_col_cap = std::max<std::size_t>(1, _col_capacity / 2);
        reallocate(_row_capacity, new_col_cap);
    }
}

void DynamicTable::pop_col() {
    if (_cols == 0) throw std::out_of_range("pop_col from empty");
    // destruction semantics
    for (std::size_t r = 0; r < _row_capacity; ++r) {
        if (_counter) ++_counter->element_destroys;
    }
    --_cols;
    if (_counter) ++_counter->col_removes;
    if (_cols <= _col_capacity / 4 && _col_capacity > 1) {
        std::size_t new_col_cap = std::max<std::size_t>(1, _col_capacity / 2);
        reallocate(_row_capacity, new_col_cap);
    }
}

void DynamicTable::remove_col_at(std::size_t idx) {
    if (idx >= _cols) throw std::out_of_range("remove_col_at: index out of range");
    for (std::size_t r = 0; r < _row_capacity; ++r) {
        for (std::size_t c = idx; c + 1 < _cols; ++c) {
            _data[r][c] = _data[r][c+1];
            if (_counter) ++_counter->element_assignments;
        }
        if (_counter) ++_counter->element_destroys; // last col cell destroyed
    }
    --_cols;
    if (_counter) ++_counter->col_removes;
    if (_cols <= _col_capacity / 4 && _col_capacity > 1) {
        std::size_t new_col_cap = std::max<std::size_t>(1, _col_capacity / 2);
        reallocate(_row_capacity, new_col_cap);
    }
}

int DynamicTable::get(std::size_t r, std::size_t c) const {
    if (r >= _rows || c >= _cols) throw std::out_of_range("get: index out of bounds");
    return _data[r][c];
}

void DynamicTable::set(std::size_t r, std::size_t c, int value) {
    if (r >= _rows || c >= _cols) throw std::out_of_range("set: index out of bounds");
    _data[r][c] = value;
    if (_counter) ++_counter->element_writes;
}

void DynamicTable::clear() {
    // logical clear: set rows and cols to zero; keep capacity
    // count destruction of existing logical elements
    if (_counter) {
        _counter->element_destroys += _rows * _cols;
        _counter->row_removes += _rows;
        _counter->col_removes += _cols;
    }
    _rows = 0;
    _cols = 0;
}

std::string DynamicTable::print_shape() const {
    return "rows=" + std::to_string(_rows) + " cols=" + std::to_string(_cols)
        + " row_cap=" + std::to_string(_row_capacity)
        + " col_cap=" + std::to_string(_col_capacity);
}
