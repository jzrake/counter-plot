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
    ObjectType call (const Mapping& scope, const crt::expression& expr) const
    {
        auto head = var();
        auto self = var (new DynamicObject);
        auto args = Array<var>();
        auto first = true;

        for (const auto& part : expr)
        {
            auto val = part.resolve<ObjectType> (scope, *this);

            if (first)
                head = val;
            else if (part.key().empty())
                args.add (val);
            else
                self.getDynamicObject()->setProperty (Identifier (part.key()), val);

            first = false;
        }
        auto f = head.getNativeFunction();
        auto a = var::NativeFunctionArgs (self, args.begin(), args.size());
        return f(a);
    }

    template<typename Mapping>
    const ObjectType& at (const Mapping& scope, const std::string& key) const
    {
        static var empty;

        try {
            return scope.at (key);
        }
        catch (const std::exception&)
        {
            return empty;
        }
    }

    ObjectType convert (const crt::expression::none&) const { return var(); }
    ObjectType convert (const int& value) const { return value; }
    ObjectType convert (const double& value) const { return value; }
    ObjectType convert (const std::string& value) const { return String (value); }
};




//=============================================================================
class Runtime
{
public:

    enum Flags {
        builtin = 2,
    };

    //=========================================================================
    using Kernel = crt::kernel<var, VarCallAdapter>;
    static void load_builtins (Kernel& kernel);

    //=========================================================================
    template<typename> class DataTypeInfo {};

    //=========================================================================
    class GenericData : public ReferenceCountedObject
    {
    public:
        virtual std::string name() = 0;
        virtual std::string summary() = 0;
    };

    //=========================================================================
    template<typename T>
    struct Data : public GenericData
    {
    public:
        Data() {}
        Data (const T& value) : value (value) {}
        std::string name() override { return  DataTypeInfo<T>::name(); }
        std::string summary() override { return  DataTypeInfo<T>::summary (value); }
        T value;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Data)
    };

    template<typename T>
    static var make_data (const T& value)
    {
        return new Data<T> (value);
    }

    static std::string type_name (const var& value)
    {
        if (auto data = dynamic_cast<GenericData*> (value.getObject()))
        {
            return data->name();
        }
        return value.toString().toStdString();
    }

    template<typename T>
    static T& check_data (const var& value)
    {
        if (auto result = dynamic_cast<Data<T>*> (value.getObject()))
        {
            return result->value;
        }
        throw std::runtime_error ("wrong data type: expected "
                                  + DataTypeInfo<T>::name()
                                  + ", got "
                                  + type_name (value));
    }

    static String represent (const var& value)
    {
        if (auto result = dynamic_cast<GenericData*> (value.getObject()))
        {
            return result->summary();
        }
        else if (value.isArray())
        {
            return "list[ ]";
        }
        else if (value.getDynamicObject())
        {
            return "dict{ }";
        }
        return value.toString();
    }
};




//=============================================================================
template<>
class Runtime::DataTypeInfo<nd::ndarray<double, 1>>
{
public:
    static std::string name() { return "nd::array<double, 1>"; }
    static std::string summary (const nd::ndarray<double, 1>& A) { return "double[" + std::to_string (A.shape(0)) + "]"; }
};

template<>
class Runtime::DataTypeInfo<Array<Colour>>
{
public:
    static std::string name() { return "Array<Colour>"; }
    static std::string summary (const Array<Colour>& A) { return "color[" + std::to_string (A.size()) + "]"; }
};

template<>
class Runtime::DataTypeInfo<std::shared_ptr<PlotArtist>>
{
public:
    static std::string name() { return "std::shared_ptr<PlotArtist>"; }
    static std::string summary (const std::shared_ptr<PlotArtist>& A) { return "PlotArtist"; }
};
