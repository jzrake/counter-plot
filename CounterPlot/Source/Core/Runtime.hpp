#pragma once
#include "JuceHeader.h"
#include "../Plotting/PlotModels.hpp"




//=============================================================================
class VarCallAdapter
{
public:
    using ObjectType = var;
    using list_t = std::vector<ObjectType>;
    using dict_t = std::unordered_map<std::string, ObjectType>;
    using func_t = std::function<ObjectType(list_t, dict_t)>;

    template<typename Mapping>
    static ObjectType call(const Mapping& scope, const crt::expression& expr)
    {
        auto self = var (new DynamicObject); //std::make_unique<DynamicObject>();
        auto head = var();
        auto args = Array<var>();

        for (const auto& part : expr)
        {
            if (part == expr.front())
            {
                head = part.resolve<ObjectType, VarCallAdapter> (scope);
            }
            else if (expr.key().empty())
            {
                args.add (part.resolve<ObjectType, VarCallAdapter> (scope));
            }
            else
            {
                self.getDynamicObject()->setProperty (String (expr.key()), part.resolve<ObjectType, VarCallAdapter> (scope));
            }
        }
        auto f = head.getNativeFunction();
        // auto a = var::NativeFunctionArgs (self.release(), args.begin(), args.size());
        auto a = var::NativeFunctionArgs (self, args.begin(), args.size());
        return f(a);
    }

    template<typename Mapping>
    static const ObjectType& at (const Mapping& scope, const std::string& key)
    {
        try {
            return scope.at (key);
        }
        catch (const std::exception&)
        {
            throw std::runtime_error ("unresolved symbol in scope: " + key);
        }
    }

    static ObjectType convert (const crt::expression::none&) { return var(); }
    static ObjectType convert (const int& value) { return value; }
    static ObjectType convert (const double& value) { return value; }
    static ObjectType convert (const std::string& value) { return String (value); }
};




//=============================================================================
class Runtime
{
public:

    //=========================================================================
    using Kernel = crt::kernel<var, VarCallAdapter>;
    static void load_builtins (Kernel& kernel);

    //=========================================================================
    template<typename> class DataTypeInfo {};

    //=========================================================================
    class GenericData : public ReferenceCountedObject
    {
    public:
        virtual String getTypeName() = 0;
        virtual String getSummary() = 0;
    };

    //=========================================================================
    template<typename T>
    struct Data : public GenericData
    {
    public:
        Data() {}
        Data (const T& value) : value (value) {}

        String getTypeName() override { return info.getTypeName(); }
        String getSummary() override { return info.getSummary (value); }

        T value;
        DataTypeInfo<T> info;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Data)
    };

    template<typename T>
    static var make_data (const T& value)
    {
        return new Data<T> (value);
    }

    template<typename T>
    static T& check_data (const var& value)
    {
        if (auto result = dynamic_cast<Data<T>*> (value.getObject()))
        {
            return result->value;
        }
        throw std::runtime_error ("check_data failed with wrong data type");
    }
};




//=============================================================================
template<>
class Runtime::DataTypeInfo<nd::ndarray<double, 1>>
{
public:
    String getTypeName() const { return "nd::array<double, 1>"; }
    String getSummary (const nd::ndarray<double, 1>& A) const { return "Array[" + std::to_string (A.shape()[0]) + "]"; }
};

template<>
class Runtime::DataTypeInfo<std::shared_ptr<PlotArtist>>
{
public:
    String getTypeName() const { return "PlotArtist"; }
    String getSummary (const std::shared_ptr<PlotArtist>& A) const { return "Artist"; }
};
