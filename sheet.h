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

    Size size_;
    std::map<int, std::map<int, Cell>> row_col_cell_;
};

/*
добавление строки - изменение ключа

int main()
{
    std::map<std::string, std::string> capital { {"111", "555"}, {"222", "666"}, {"333", "444"} };
    const std::string from = "111", to = "222";

    if (auto nh = capital.extract(from); nh) {
        nh.key() = to;
        capital.insert(std::move(nh));
    }

    for (const auto& [key, value] : capital)
        std::cout << key << " " << value << std::endl;
}

 находим ключ, затем пока не конец мэп делаем экстракт и добавляем единицу 
 аналогично с столбцами, только двойной цикл, пройдя по всем строкам


*/