#pragma once
#include "JuceHeader.h"
#include "../Plotting/PlotModels.hpp"




//=============================================================================
class VarCallAdapter
{
public:


    //=========================================================================
    using ObjectType = var;
    using list_t = std::vector<ObjectType>;
    using dict_t = std::unordered_map<std::string, ObjectType>;
    using func_t = std::function<ObjectType(list_t, dict_t)>;


    //=========================================================================
    struct BailoutChecker : public ReferenceCountedObject
    {
        static Identifier argkey;
        BailoutChecker (std::function<bool()> callback) : callback (callback) {}
        std::function<bool()> callback;
    };


    //=========================================================================
    VarCallAdapter() {}
    VarCallAdapter (std::function<bool()> callback) : bailout (new BailoutChecker (callback)) {}


    //=========================================================================
    template<typename Mapping>
    ObjectType call (const Mapping& scope, const crt::expression& expr) const
    {
        auto head = var();
        auto self = var (new DynamicObject);
        auto args = Array<var>();
        auto first = true;

        if (! bailout.isVoid())
        {
            self.getDynamicObject()->setProperty (BailoutChecker::argkey, bailout);
        }

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

        if (f == nullptr)
            throw std::runtime_error("expression head is not a function");

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

    var bailout;
};




//=============================================================================
class Runtime
{
public:

    enum Flags {
        builtin      = 2,
        asynchronous = 4,
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
        std::string name() override { return DataTypeInfo<T>::name(); }
        std::string summary() override { return DataTypeInfo<T>::summary (value); }
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
    static T& check_data (const var& value, const char* caller=nullptr, int index=-1)
    {
        if (auto result = dynamic_cast<Data<T>*> (value.getObject()))
        {
            return result->value;
        }
        throw std::runtime_error ((caller ? std::string (caller) + ": " : "")
                                  + "wrong data type "
                                  + DataTypeInfo<T>::name()
                                  + ", got "
                                  + type_name (value)
                                  + (index == -1 ? "" : " at index " + std::to_string (index)));
    }

    static String represent (const var& value)
    {
        if (auto result = dynamic_cast<GenericData*> (value.getObject()))
        {
            return result->summary();
        }
        else if (value.isArray())
        {
            return "list[]";
        }
        else if (value.getDynamicObject())
        {
            return "dict{}";
        }
        return value.toString();
    }
};




//=============================================================================
template<>
class Runtime::DataTypeInfo<nd::array<double, 1>>
{
public:
    static std::string name() { return "nd::array<double, 1>"; }
    static std::string summary (const nd::array<double, 1>& A)
    {
        auto ni = std::to_string (A.shape(0));
        return "double[" + ni + "]";
    }
};

template<>
class Runtime::DataTypeInfo<nd::array<double, 2>>
{
public:
    static std::string name() { return "nd::array<double, 2>"; }
    static std::string summary (const nd::array<double, 2>& A)
    {
        auto ni = std::to_string (A.shape(0));
        auto nj = std::to_string (A.shape(1));
        return "double[" + ni + ", " + nj + "]";
    }
};

template<>
class Runtime::DataTypeInfo<nd::array<double, 3>>
{
public:
    static std::string name() { return "nd::array<double, 3>"; }
    static std::string summary (const nd::array<double, 3>& A)
    {
        auto ni = std::to_string (A.shape(0));
        auto nj = std::to_string (A.shape(1));
        auto nk = std::to_string (A.shape(2));
        return "double[" + ni + ", " + nj + ", " + nk + "]";
    }
};

//=============================================================================
template<>
class Runtime::DataTypeInfo<Array<Colour>>
{
public:
    static std::string name() { return "Array<Colour>"; }
    static std::string summary (const Array<Colour>& A) { return "color[" + std::to_string (A.size()) + "]"; }
};

//=============================================================================
template<>
class Runtime::DataTypeInfo<std::shared_ptr<PlotArtist>>
{
public:
    static std::string name() { return "std::shared_ptr<PlotArtist>"; }
    static std::string summary (const std::shared_ptr<PlotArtist>& A) { return "PlotArtist"; }
};

//=============================================================================
template<>
class Runtime::DataTypeInfo<ScalarMapping>
{
public:
    static std::string name() { return "ScalarMapping"; }
    static std::string summary (const ScalarMapping& A)
    {
        return "mapping(" + std::to_string (A.vmin) + " -> " + std::to_string (A.vmax) + ")";
    }
};

//=============================================================================
template<>
class Runtime::DataTypeInfo<DeviceBufferFloat1>
{
public:
    static std::string name() { return "DeviceBufferFloat1"; }
    static std::string summary (const DeviceBufferFloat1& A) { return "device::float1[" + std::to_string (A.size) + "]"; }
};

//=============================================================================
template<>
class Runtime::DataTypeInfo<DeviceBufferFloat2>
{
public:
    static std::string name() { return "DeviceBufferFloat2"; }
    static std::string summary (const DeviceBufferFloat2& A) { return "device::float2[" + std::to_string (A.size) + "]"; }
};

//=============================================================================
template<>
class Runtime::DataTypeInfo<DeviceBufferFloat4>
{
public:
    static std::string name() { return "DeviceBufferFloat4"; }
    static std::string summary (const DeviceBufferFloat4& A) { return "device::float4[" + std::to_string (A.size) + "]"; }
};
