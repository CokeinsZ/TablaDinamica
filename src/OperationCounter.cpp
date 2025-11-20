#include "OperationCounter.hpp"
#include <iostream>

void OperationCounter::reset() {
  row_inserts = row_removes = col_inserts = col_removes = 0;
  element_writes = element_assignments = element_destroys = 0;
  row_resizes = col_resizes = row_capacity_changes = col_capacity_changes = 0;
}

std::size_t OperationCounter::total_cost_estimate() const {
  return row_inserts + row_removes + col_inserts + col_removes
    + element_writes + element_assignments + element_destroys
    + row_resizes + col_resizes;
}

void OperationCounter::report(const std::string &title) const {
  std::cout << "Reporte: " << title << "\n";
  std::cout << "Inserciones de filas:            " << row_inserts << "\n";
  std::cout << "Eliminaciones de filas:          " << row_removes << "\n";
  std::cout << "Inserciones de columnas:         " << col_inserts << "\n";
  std::cout << "Eliminaciones de columnas:       " << col_removes << "\n";
  std::cout << "Escrituras de elementos:         " << element_writes << "\n";
  std::cout << "Asignaciones de elementos:       " << element_assignments << " (copias durante realocación)\n";
  std::cout << "Destrucciones de elementos:      " << element_destroys << "\n";
  std::cout << "Redimensionamientos de filas:    " << row_resizes << "\n";
  std::cout << "Redimensionamientos de columnas: " << col_resizes << "\n";
  std::cout << "Cambios de capacidad de filas:   " << row_capacity_changes << "\n";
  std::cout << "Cambios de capacidad de columnas:" << col_capacity_changes << "\n";
  std::cout << "Total:                           " << total_cost_estimate() << "\n";
}
