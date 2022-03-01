#include "formula.h"
#include "FormulaAST.h"

#include <sstream>
#include <optional>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!"s;
}

namespace {
    class Formula : public FormulaInterface {
    public:
        explicit Formula(std::string expression);

        Value Evaluate(const SheetInterface& sheet) override;

        std::string GetExpression() const override;

        std::vector<Position> GetReferencedCells() const override;

        bool InvalidateCashe() override;
    private:
        double DoubleExecuter(const CellInterface::Value& val);
        double ConvertStringToNumber(const std::string number);

        FormulaAST ast_;
        std::optional<double> cashe_;
    };
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}

Formula::Formula(std::string expression) : ast_(ParseFormulaAST(expression)) {
}

Formula::Value Formula::Evaluate(const SheetInterface& sheet) {
    if (cashe_) {
        return *cashe_;
    }

    auto ExecuteDouble = [&sheet, this](const Position* pos) {
        if (!pos->IsValid()) {
            throw FormulaError(FormulaError::Category::Ref);
        }
        if (!sheet.GetCell(*pos)) { 
            const_cast<SheetInterface&>(sheet).SetCell(*pos, ""s);
        }
        CellInterface::Value val = sheet.GetCell(*pos)->GetValue();

        return DoubleExecuter(val);
    };

    try {
        return ast_.Execute(ExecuteDouble);          // Исправить
    }
    catch (FormulaError& exception) {
        return exception;
    }
}

std::string Formula::GetExpression() const {
    std::ostringstream out;
    ast_.PrintFormula(out);
    return out.str();
}

std::vector<Position> Formula::GetReferencedCells() const {
    std::forward_list<Position> cells = ast_.GetCells();
    cells.unique();
    return std::vector<Position>{cells.begin(), cells.end()};
}

bool Formula::InvalidateCashe() {
    if (cashe_) {
        cashe_ = std::nullopt;
        return true;
    }
    return false;
}

double Formula::DoubleExecuter(const CellInterface::Value& val) {
    if (std::holds_alternative<double>(val)) {
        *cashe_ = std::get<double>(val);
        return *cashe_;
    }
    else if (std::holds_alternative<std::string>(val)) {
        return ConvertStringToNumber(std::get<std::string>(val));
    }
    else {
        throw std::get<FormulaError>(val);
    }
}

double Formula::ConvertStringToNumber(const std::string number) {
    if (number.size() == 0) {
        *cashe_ = 0.0;
        return 0.0;
    }
    double result = 0;
    std::istringstream row_in{ std::string{number} };
    if (!(row_in >> result) || !row_in.eof()) {
        throw FormulaError(FormulaError::Category::Value);
    }
    return result;
}