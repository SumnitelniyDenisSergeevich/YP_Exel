#include "cell.h"

using namespace std::literals;

CellInterface::Value EmptyImpl::GetValue(const SheetInterface& sheet) const {
	return ""s;
}

std::string EmptyImpl::GetText() const {
	return ""s;
}

std::vector<Position> EmptyImpl::GetReferencedCells() const {
	return {};
}

bool EmptyImpl::InvalidateCashe() {
	return false;
}

TextImpl::TextImpl(std::string text) : text_(text) {
}

CellInterface::Value TextImpl::GetValue(const SheetInterface& sheet) const {
	return *text_.begin() == ESCAPE_SIGN ? text_.substr(1) : text_;
}

std::string TextImpl::GetText() const {
	return text_;
}

std::vector<Position> TextImpl::GetReferencedCells() const {
	return {};
}


bool TextImpl::InvalidateCashe() {
	return false;
}

FormulaImpl::FormulaImpl(std::string text) : formula_(ParseFormula(text)) {
}

CellInterface::Value FormulaImpl::GetValue(const SheetInterface& sheet) const {
	FormulaInterface::Value value = formula_->Evaluate(sheet);
	if (std::holds_alternative<double>(value)) {
		return std::get<double>(value);
	}
	else {
		return std::get<FormulaError>(value);
	}
}

std::string FormulaImpl::GetText() const {
	return FORMULA_SIGN + formula_->GetExpression();
}

std::vector<Position> FormulaImpl::GetReferencedCells() const {
	return formula_->GetReferencedCells();
}

bool FormulaImpl::InvalidateCashe() {
	return formula_->InvalidateCashe();
}

Cell::Cell(Sheet& sheet) : sheet_(sheet), impl_(std::make_unique<EmptyImpl>(EmptyImpl{})) {
}

Cell::~Cell() {
}

void Cell::Set(std::string text) {
	std::unique_ptr<Impl> buffer;
	if (text.empty()) {
		buffer = std::make_unique<EmptyImpl>(EmptyImpl{});
	}
	else if (*text.begin() == FORMULA_SIGN && text.size() > 1) {
		buffer = std::make_unique<FormulaImpl>(FormulaImpl{ text.substr(1) });
	}
	else {
		buffer = std::make_unique<TextImpl>(TextImpl{ text });
	}
	СheckСyclicity(buffer->GetReferencedCells());
	InvalidateCashe();
	impl_ = std::move(buffer);
	for (Cell* cell : referenced_cells_) {
		cell->dependent_cells_.erase(this);
	}
	referenced_cells_.clear();
	for (const Position& pos : impl_->GetReferencedCells()) {
		Cell* cell = dynamic_cast<Cell*>(sheet_.GetCell(pos));
		referenced_cells_.insert(cell);
		cell->dependent_cells_.insert(this);
	}
}

void Cell::Clear() {
	Set(""s);
}

Cell::Value Cell::GetValue() const {
	return impl_->GetValue(sheet_);
}
std::string Cell::GetText() const {
	return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
	return impl_->GetReferencedCells();
}

void Cell::InvalidateCashe() { // надо инвалидировать по дереву
	if (impl_->InvalidateCashe()) {
		for (Cell* cell : dependent_cells_) {
			cell->InvalidateCashe();
		}
	}
}

bool Cell::IsReferenced() const {
	return dependent_cells_.size();
}

void Cell::СheckСyclicity(const std::vector<Position>& ref_cells) const {
	std::unordered_set<const Cell*> verifyed;
	std::vector<Position> created;

	for (const Position& pos : ref_cells) {
		if (!sheet_.GetCell(pos)) {
			sheet_.SetCell(pos, ""s);
			created.push_back(pos);
			continue;
		}
		try {
			dynamic_cast<Cell*>(sheet_.GetCell(pos))->СheckCellForСyclicity(this, verifyed);
		}
		catch (CircularDependencyException& e) {
			for (const Position& pos : created) {
				sheet_.ClearCell(pos);
			}
			throw CircularDependencyException{ e.what() };
		}
	
	}
}

void Cell::СheckCellForСyclicity(const Cell* base, std::unordered_set<const Cell*>& verifyed) const {
	if (this == base) {
		throw CircularDependencyException(""s);
	}
	if (verifyed.count(base)) {
		return;
	}
	for (const Cell* cell : referenced_cells_) {
		cell->СheckCellForСyclicity(base, verifyed);
	}
	verifyed.insert(this);
}