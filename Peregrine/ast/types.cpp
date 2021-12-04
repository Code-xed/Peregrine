#include "types.hpp"
#include "lexer/tokens.hpp"

#include <memory>

namespace types {

IntType::IntType(IntSizes intSize, Modifier modifier) {
    m_intSize = intSize;
    m_modifier = modifier;
}

TypeCategory IntType::category() const { return TypeCategory::Integer; }

IntType::IntSizes IntType::size() const { return m_intSize; }

IntType::Modifier IntType::modifier() const { return m_modifier; }

bool IntType::isConvertibleTo(const Type& type) const {
    switch (type.category()) {
        case TypeCategory::Integer:
        case TypeCategory::Decimal:
            break;

        default:
            return false;
    }

    if (type.category() == TypeCategory::Integer) {
        auto typeInt = dynamic_cast<const IntType&>(type);

        // an Int32 can't be converted to a Int8
        if (m_intSize > typeInt.size())
            return false;
    } else {
        auto typeDecimal = dynamic_cast<const DecimalType&>(type);

        // if the integer has a value of 64 bits, it can only fit in doubles
        if (m_intSize == IntType::Int64 && typeDecimal.isFloat())
            return false;
    }

    return true;
}

std::string IntType::stringify() const { return "integer"; }

// TODO: unsigned ints
TypePtr IntType::prefixOperatorResult(Token op) const {
    switch (op.tkType) {
        case tk_not:
            return TypeProducer::boolean();

        case tk_minus:
        case tk_bit_not:
            return TypeProducer::integer(); // no

        default:
            return nullptr;
    }
}

TypePtr IntType::infixOperatorResult(Token op, const TypePtr type) const {
    switch (type->category()) {
        case TypeCategory::Integer:
        case TypeCategory::Decimal:
            break;

        default:
            return nullptr;
    }

    if (TokenUtils::isArithmeticToken(op))
        return type;

    if (TokenUtils::isComparisonToken(op))
        return TypeProducer::boolean();

    // TODO: handle bitwise operations
    return nullptr;
}

bool IntType::operator==(const Type& type) const {
    if (type.category() == TypeCategory::Integer) {
        auto intType = dynamic_cast<const IntType&>(type);
        if (intType.size() == size() && intType.modifier() == modifier())
            return true;
    }

    return false;
}

DecimalType::DecimalType(DecimalSize decimalSize) {
    m_decimalSize = decimalSize;
}

TypeCategory DecimalType::category() const { return TypeCategory::Decimal; }

DecimalType::DecimalSize DecimalType::size() const { return m_decimalSize; }

bool DecimalType::isConvertibleTo(const Type& type) const {
    if (type.category() != TypeCategory::Decimal)
        return false;

    auto typeDecimal = dynamic_cast<const DecimalType&>(type);

    if (!isFloat() && typeDecimal.isFloat())
        return false;

    return true;
}

std::string DecimalType::stringify() const {
    return (isFloat()) ? "float" : "double";
}

bool DecimalType::isFloat() const {
    return (m_decimalSize == DecimalSize::Float);
}

TypePtr DecimalType::prefixOperatorResult(Token op) const {
    switch (op.tkType) {
        case tk_not:
            return TypeProducer::boolean();

        case tk_minus:
            return TypeProducer::decimal(); // no

        default:
            return nullptr;
    }
}

TypePtr DecimalType::infixOperatorResult(Token op, const TypePtr type) const {
    switch (type->category()) {
        case TypeCategory::Integer:
        case TypeCategory::Decimal:
            break;

        default:
            return nullptr;
    }

    if (TokenUtils::isArithmeticToken(op))
        return TypeProducer::decimal();

    if (TokenUtils::isComparisonToken(op))
        return TypeProducer::boolean();

    return nullptr;
}

bool DecimalType::operator==(const Type& type) const {
    if (type.category() == TypeCategory::Decimal) {
        auto decimalType = dynamic_cast<const DecimalType&>(type);
        if (decimalType.size() == size())
            return true;
    }

    return false;
}

TypeCategory StringType::category() const { return TypeCategory::String; }

bool StringType::isConvertibleTo(const Type& type) const {
    return (type.category() == TypeCategory::String);
}

std::string StringType::stringify() const { return "string"; }

TypePtr StringType::prefixOperatorResult(Token op) const {
    if (op.tkType == tk_not)
        return TypeProducer::boolean();

    return nullptr;
}

TypePtr StringType::infixOperatorResult(Token op, const TypePtr type) const {
    if (type->category() != TypeCategory::String)
        return nullptr;

    switch (op.tkType) {
        case tk_plus:
            return type;

        case tk_equal:
        case tk_not_equal:
            return TypeProducer::boolean();

        default:
            return nullptr;
    }
}

TypeCategory BoolType::category() const { return TypeCategory::Bool; }

bool BoolType::isConvertibleTo(const Type& type) const {
    return (type.category() == TypeCategory::Bool);
}

std::string BoolType::stringify() const { return "bool"; }

TypePtr BoolType::prefixOperatorResult(Token op) const { return nullptr; }

TypePtr BoolType::infixOperatorResult(Token op, const TypePtr type) const {
    return nullptr;
}

TypeCategory VoidType::category() const { return TypeCategory::Void; }

bool VoidType::isConvertibleTo(const Type& type) const { return false; }

std::string VoidType::stringify() const { return "void"; }

ListType::ListType(TypePtr baseType) { m_baseType = baseType; }

TypeCategory ListType::category() const { return TypeCategory::List; }

bool ListType::isConvertibleTo(const Type& type) const { return false; }

std::string ListType::stringify() const { return ""; }

UserDefinedType::UserDefinedType(TypePtr baseType) { m_baseType = baseType; }

TypeCategory UserDefinedType::category() const {
    return TypeCategory::UserDefined;
}

bool UserDefinedType::isConvertibleTo(const Type& type) const {
    return m_baseType->isConvertibleTo(type);
}

std::string UserDefinedType::stringify() const { return ""; }

FunctionType::FunctionType(std::vector<TypePtr> parameterTypes,
                           TypePtr returnType) {
    m_parameterTypes = parameterTypes;
    m_returnType = returnType;
}

TypeCategory FunctionType::category() const { return TypeCategory::Function; }

std::vector<TypePtr> FunctionType::parameterTypes() const {
    return m_parameterTypes;
}

TypePtr FunctionType::returnType() const { return m_returnType; }

bool FunctionType::isConvertibleTo(const Type& type) const { return false; }

std::string FunctionType::stringify() const { return "function"; }

std::array<TypePtr, 8> TypeProducer::m_integer = {
    std::make_shared<IntType>(IntType::IntSizes::Int8),
    std::make_shared<IntType>(IntType::IntSizes::Int16),
    std::make_shared<IntType>(IntType::IntSizes::Int32),
    std::make_shared<IntType>(IntType::IntSizes::Int64),
    std::make_shared<IntType>(IntType::IntSizes::Int8,
                              IntType::Modifier::Unsigned),
    std::make_shared<IntType>(IntType::IntSizes::Int16,
                              IntType::Modifier::Unsigned),
    std::make_shared<IntType>(IntType::IntSizes::Int32,
                              IntType::Modifier::Unsigned),
    std::make_shared<IntType>(IntType::IntSizes::Int64,
                              IntType::Modifier::Unsigned)};

std::array<TypePtr, 2> TypeProducer::m_decimal = {
    std::make_shared<DecimalType>(DecimalType::DecimalSize::Float),
    std::make_shared<DecimalType>(DecimalType::DecimalSize::Double)};

TypePtr TypeProducer::m_bool = std::make_shared<BoolType>();
TypePtr TypeProducer::m_string = std::make_shared<StringType>();
TypePtr TypeProducer::m_void = std::make_shared<VoidType>();

TypePtr TypeProducer::integer(IntType::IntSizes intSize,
                              IntType::Modifier modifier) {
    if (modifier == IntType::Modifier::Signed) {
        return m_integer[intSize];
    }

    return m_integer[intSize + 4];
}

TypePtr TypeProducer::decimal(DecimalType::DecimalSize decimalSize) {
    return m_decimal[decimalSize];
}

TypePtr TypeProducer::boolean() { return m_bool; }

TypePtr TypeProducer::string() { return m_string; }

TypePtr TypeProducer::voidT() { return m_void; }

std::map<std::string, TypePtr> identifierToTypeMap = {
    {"i8", TypeProducer::integer(IntType::IntSizes::Int8)},
    {"i16", TypeProducer::integer(IntType::IntSizes::Int16)},
    {"i32", TypeProducer::integer()},
    {"int", TypeProducer::integer()},
    {"i64", TypeProducer::integer(IntType::IntSizes::Int64)},
    {"float", TypeProducer::decimal()},
    {"double", TypeProducer::decimal(DecimalType::DecimalSize::Double)},
    {"str", TypeProducer::string()},
    {"bool", TypeProducer::boolean()},
    {"void", TypeProducer::voidT()}};

} // namespace types