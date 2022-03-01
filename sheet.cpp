#include "sheet.h"

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position"s);
    }
    if (!GetCell(pos)) {
        CreateCellIfExceptionDeleteCell(pos, text);
    }
    else {
        row_col_cell_.at(pos.row).at(pos.col).Set(text);
    }
    UpdateSize();
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return const_cast<Sheet*>(this)->GetCell(pos);
}
CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position"s);
    }
    if (row_col_cell_.count(pos.row)) {
        if (row_col_cell_.at(pos.row).count(pos.col)) {
            return &row_col_cell_.at(pos.row).at(pos.col);
        }
    }
    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position"s);
    }
    if (GetCell(pos)) {
        if (row_col_cell_.at(pos.row).at(pos.col).IsReferenced()) {
            row_col_cell_.at(pos.row).at(pos.col).Clear();
        }
        else {
            EraseCell(pos);
        }
    }
    UpdateSize();
}

Size Sheet::GetPrintableSize() const {
    return size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < size_.rows; ++i) {
        bool flag = false;
        for (int j = 0; j < size_.cols; ++j) {
            if (flag) {
                output << '\t';
            }
            flag = true;
            if (auto* cell = GetCell({ i, j }); cell) {
                std::visit(ValuePrinter{ output }, cell->GetValue());
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < size_.rows; ++i) {
        bool flag = false;
        for (int j = 0; j < size_.cols; ++j) {
            if (flag) {
                output << '\t';
            }
            flag = true;
            if (auto* cell = GetCell({ i, j }); cell) {
                output << cell->GetText();
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

void Sheet::EraseCell(Position pos) {  
    if (auto* cell = GetCell(pos); cell) {
        row_col_cell_.at(pos.row).erase(pos.col);
        if (row_col_cell_.at(pos.row).size() == 0) {
            row_col_cell_.erase(pos.row);
        }
    }
}

void Sheet::UpdateSize() {
    Size result;
    if (row_col_cell_.size() > 0) {
        result.rows = row_col_cell_.rbegin()->first + 1;
        for (const auto& [row, col_cell] : row_col_cell_) {
            int temp = col_cell.rbegin()->first + 1;
            if (temp > result.cols) {
                result.cols = temp;
            }
        }
    }
    size_ = result;
}

void Sheet::CreateCellIfExceptionDeleteCell(Position pos, std::string text) {
    row_col_cell_[pos.row].emplace(pos.col, *this);
    try {
        row_col_cell_.at(pos.row).at(pos.col).Set(text);
    }
    catch (CircularDependencyException& e) {
        EraseCell(pos);
        throw CircularDependencyException{ e.what() };
    }
}