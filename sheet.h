#pragma once

#include "common.h"
#include "cell.h"

#include <map>
#include <iostream>

class Cell;

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    struct ValuePrinter {
        std::ostream& out;
        void operator()(const double val) const {
            out << val;
        }
        void operator()(const std::string& text) const {
            out << text;
        }
        void operator()(const FormulaError& f_err) const {
            out << f_err;
        }
    };

    void EraseCell(Position pos);
    void UpdateSize();
    void CreateCellIfExceptionDeleteCell(Position pos, std::string text);
    void ValidPosition(Position pos);

    Size size_;
    std::map<int, std::map<int, Cell>> row_col_cell_;
};