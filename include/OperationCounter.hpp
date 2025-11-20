#ifndef OPERATION_COUNTER_HPP
#define OPERATION_COUNTER_HPP

#include <cstddef>
#include <string>

class OperationCounter {
  public:
    std::size_t row_inserts = 0;
    std::size_t row_removes = 0;
    std::size_t col_inserts = 0;
    std::size_t col_removes = 0;

    std::size_t element_writes = 0;
    std::size_t element_assignments = 0;
    std::size_t element_destroys = 0;
    
    std::size_t row_resizes = 0;
    std::size_t col_resizes = 0;
    std::size_t row_capacity_changes = 0;
    std::size_t col_capacity_changes = 0;

    void reset();
    std::size_t total_cost_estimate() const;
    void report(const std::string &title) const;
};

#endif
