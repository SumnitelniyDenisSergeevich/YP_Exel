#pragma once

#include "common.h"
#include "formula.h"
#include "sheet.h"

#include <unordered_set>

class Sheet;

class FormulaInterface;

class Impl {
public:
    virtual CellInterface::Value GetValue(const SheetInterface& sheet) const = 0;
    virtual std::string GetText() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
    virtual bool InvalidateCashe() = 0;
};

class EmptyImpl final : public Impl {
public:
    CellInterface::Value GetValue(const SheetInterface& sheet) const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    bool InvalidateCashe() override;
};

class TextImpl final : public Impl {
public:
    TextImpl(std::string text);
    CellInterface::Value GetValue(const SheetInterface& sheet) const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    bool InvalidateCashe() override;
private:
    std::string text_;
};

class FormulaImpl final : public Impl {
public:
    FormulaImpl(std::string text);
    CellInterface::Value GetValue(const SheetInterface& sheet) const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    bool InvalidateCashe() override;
private:
    std::unique_ptr<FormulaInterface> formula_;
};


class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);

    void Clear();
    void InvalidateCashe();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;
private:
    void СheckСyclicity(const std::vector<Position>& ref_positions) const;
    void СheckCellForСyclicity(const Cell* base, std::unordered_set<const Cell*>& verifyed) const;

    Sheet& sheet_;
    std::unique_ptr<Impl> impl_;
    std::unordered_set<Cell*> referenced_cells_; // ячейки, от которых зависит эта ячейка
    std::unordered_set<Cell*> dependent_cells_; // ячейки, зависящие от этой ячейки
};